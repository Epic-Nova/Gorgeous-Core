// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#include "K2Nodes/GorgeousK2Node_NewObjectVariable.h"

#if 0

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "KismetCompilerMisc.h"
#include "ObjectVariables/GorgeousObjectVariable.h"

#define LOCTEXT_NAMESPACE "GorgeousNewObjectVariable"

const FName UK2Node_GorgeousNewObjectVariable::ParentPinName(TEXT("Parent"));
const FName UK2Node_GorgeousNewObjectVariable::PersistPinName(TEXT("bShouldPersist"));
const FName UK2Node_GorgeousNewObjectVariable::DisplayNamePinName(TEXT("DisplayNameOverride"));
const FName UK2Node_GorgeousNewObjectVariable::IdentifierPinName(TEXT("Identifier"));

UK2Node_GorgeousNewObjectVariable::UK2Node_GorgeousNewObjectVariable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeTooltip = LOCTEXT("NodeTooltip", "Creates a Gorgeous Object Variable and registers it while honoring ExposeOnSpawn pins.");
}

void UK2Node_GorgeousNewObjectVariable::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UGorgeousObjectVariable::StaticClass(), ParentPinName);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, nullptr, PersistPinName);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, nullptr, DisplayNamePinName)->bAdvancedView = true;
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FGuid>::Get(), IdentifierPinName);
}

FText UK2Node_GorgeousNewObjectVariable::GetTooltipText() const
{
	return NodeTooltip;
}

FText UK2Node_GorgeousNewObjectVariable::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		return GetBaseNodeTitle();
	}

	if (UClass* ClassToSpawn = GetClassToSpawn())
	{
		if (CachedNodeTitle.IsOutOfDate(this))
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("ClassName"), ClassToSpawn->GetDisplayNameText());
			CachedNodeTitle.SetCachedText(FText::Format(GetNodeTitleFormat(), Args), this);
		}
		return CachedNodeTitle;
	}

	return GetDefaultNodeTitle();
}

void UK2Node_GorgeousNewObjectVariable::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	UEdGraphPin* ExecPin = GetExecPin();
	UEdGraphPin* ThenPin = GetThenPin();
	UEdGraphPin* ClassPin = GetClassPin();
	UEdGraphPin* ParentPin = GetParentPin();
	UEdGraphPin* PersistPin = GetPersistPin();
	UEdGraphPin* DisplayNamePin = GetDisplayNamePin();
	UEdGraphPin* IdentifierPin = GetIdentifierPin();
	UEdGraphPin* ResultPin = GetResultPin();

	UClass* ClassToSpawn = GetClassToSpawn();
	UClass* SpawnClassLiteral = ClassPin && ClassPin->LinkedTo.Num() == 0 ? Cast<UClass>(ClassPin->DefaultObject) : nullptr;
	if (!ClassPin || (!SpawnClassLiteral && ClassPin->LinkedTo.Num() == 0))
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("NewObjectVariableMissingClass", "@@ must have a @@ specified.").ToString(), this, ClassPin);
		BreakAllNodeLinks();
		return;
	}

	UFunction* FactoryFunction = UGorgeousObjectVariable::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGorgeousObjectVariable, NewObjectVariable));
	if (!FactoryFunction)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("NewObjectVariableMissingFunction", "Could not find factory function for @@.").ToString(), this);
		BreakAllNodeLinks();
		return;
	}

	UK2Node_CallFunction* CallNewVariableNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallNewVariableNode->SetFromFunction(FactoryFunction);
	CallNewVariableNode->AllocateDefaultPins();

	UEdGraphPin* CallExecPin = CallNewVariableNode->GetExecPin();
	UEdGraphPin* CallThenPin = CallNewVariableNode->GetThenPin();
	UEdGraphPin* CallClassPin = CallNewVariableNode->FindPinChecked(TEXT("Class"));
	UEdGraphPin* CallIdentifierPin = CallNewVariableNode->FindPinChecked(TEXT("Identifier"));
	UEdGraphPin* CallParentPin = CallNewVariableNode->FindPinChecked(TEXT("Parent"));
	UEdGraphPin* CallPersistPin = CallNewVariableNode->FindPinChecked(TEXT("bShouldPersist"));
	UEdGraphPin* CallDisplayNamePin = CallNewVariableNode->FindPinChecked(TEXT("DisplayNameOverride"));
	UEdGraphPin* CallResultPin = CallNewVariableNode->GetReturnValuePin();

	CompilerContext.MovePinLinksToIntermediate(*ExecPin, *CallExecPin);

	if (ClassPin->LinkedTo.Num() > 0)
	{
		CompilerContext.MovePinLinksToIntermediate(*ClassPin, *CallClassPin);
	}
	else
	{
		CallClassPin->DefaultObject = ClassPin->DefaultObject;
	}

	auto MovePinWithDefault = [&CompilerContext](UEdGraphPin* SourcePin, UEdGraphPin* TargetPin)
	{
		if (!SourcePin || !TargetPin)
		{
			return;
		}

		if (SourcePin->LinkedTo.Num() > 0)
		{
			CompilerContext.MovePinLinksToIntermediate(*SourcePin, *TargetPin);
		}
		else
		{
			TargetPin->DefaultObject = SourcePin->DefaultObject;
			TargetPin->DefaultValue = SourcePin->DefaultValue;
			TargetPin->DefaultTextValue = SourcePin->DefaultTextValue;
		}
	};

	MovePinWithDefault(ParentPin, CallParentPin);
	MovePinWithDefault(PersistPin, CallPersistPin);
	MovePinWithDefault(DisplayNamePin, CallDisplayNamePin);
	MovePinWithDefault(IdentifierPin, CallIdentifierPin);

	CallResultPin->PinType = ResultPin->PinType;
	CompilerContext.MovePinLinksToIntermediate(*ResultPin, *CallResultPin);

	UEdGraphPin* LastThen = FKismetCompilerUtilities::GenerateAssignmentNodes(CompilerContext, SourceGraph, CallNewVariableNode, this, CallResultPin, ClassToSpawn ? ClassToSpawn : SpawnClassLiteral);
	if (LastThen)
	{
		CompilerContext.MovePinLinksToIntermediate(*ThenPin, *LastThen);
	}
	else
	{
		CompilerContext.MovePinLinksToIntermediate(*ThenPin, *CallThenPin);
	}

	BreakAllNodeLinks();
}

void UK2Node_GorgeousNewObjectVariable::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_GorgeousNewObjectVariable::GetMenuCategory() const
{
	return LOCTEXT("GorgeousObjectVariablesCategory", "Gorgeous Core|Object Variables");
}

UClass* UK2Node_GorgeousNewObjectVariable::GetClassPinBaseClass() const
{
	return UGorgeousObjectVariable::StaticClass();
}

FText UK2Node_GorgeousNewObjectVariable::GetBaseNodeTitle() const
{
	return LOCTEXT("ConstructObjectVariableBaseTitle", "Create Gorgeous Object Variable");
}

FText UK2Node_GorgeousNewObjectVariable::GetDefaultNodeTitle() const
{
	return LOCTEXT("ConstructObjectVariableDefaultTitle", "Create Object Variable");
}

FText UK2Node_GorgeousNewObjectVariable::GetNodeTitleFormat() const
{
	return LOCTEXT("ConstructObjectVariableNodeTitle", "Create {ClassName}");
}

bool UK2Node_GorgeousNewObjectVariable::IsSpawnVarPin(UEdGraphPin* Pin) const
{
	return Super::IsSpawnVarPin(Pin)
		&& Pin->PinName != ParentPinName
		&& Pin->PinName != PersistPinName
		&& Pin->PinName != DisplayNamePinName
		&& Pin->PinName != IdentifierPinName;
}

UEdGraphPin* UK2Node_GorgeousNewObjectVariable::GetParentPin() const
{
	UEdGraphPin* Pin = FindPin(ParentPinName);
	ensure(!Pin || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_GorgeousNewObjectVariable::GetPersistPin() const
{
	UEdGraphPin* Pin = FindPin(PersistPinName);
	ensure(!Pin || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_GorgeousNewObjectVariable::GetDisplayNamePin() const
{
	UEdGraphPin* Pin = FindPin(DisplayNamePinName);
	ensure(!Pin || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_GorgeousNewObjectVariable::GetIdentifierPin() const
{
	UEdGraphPin* Pin = FindPin(IdentifierPinName);
	ensure(!Pin || Pin->Direction == EGPD_Output);
	return Pin;
}

#undef LOCTEXT_NAMESPACE


#endif