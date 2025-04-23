#include "UK2Node_MyCustomNode.h"

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_VariableGet.h"
#include "K2Node_CallFunction.h"
#include "Engine/Blueprint.h" // Required for GetBlueprintClassFromNode() context
#include "Kismet2/BlueprintEditorUtils.h" // May be needed for more complex scenarios or utilities
#include "UObject/UnrealType.h" // Required for FProperty, FObjectProperty
#include "Logging/LogMacros.h" // For UE_LOG
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "KismetCompiler.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableDefinitions.h"

#define LOCTEXT_NAMESPACE "K2Node"

void UK2Node_MyCustomNode::AllocateDefaultPins()
{
    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

    CreatePin(EGPD_Input, Schema->PC_Object, UGorgeousObjectVariable::StaticClass(), TEXT("Target"));
    CreatePin(EGPD_Output, Schema->PC_Wildcard, NAME_None, nullptr, TEXT("ReturnValue"));
}

void UK2Node_MyCustomNode::ReconstructNode()
{
    Super::ReconstructNode();

    if(GetTargetPin())
    {
        BuildReturnValuePin();

        GetGraph()->NotifyGraphChanged();
    }
}

void UK2Node_MyCustomNode::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);
}

void UK2Node_MyCustomNode::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
    Super::NotifyPinConnectionListChanged(Pin);
    
    if (Pin == GetTargetPin())
    {
        UEdGraphPin* OutputPin = BuildReturnValuePin();

        // Reset default values
        Pin->DefaultValue = TEXT("");
        Pin->DefaultObject = nullptr;
        Pin->DefaultTextValue = FText::GetEmpty();

        if (OutputPin)
        {
            OutputPin->DefaultValue = TEXT("");
            OutputPin->DefaultObject = nullptr;
            OutputPin->DefaultTextValue = FText::GetEmpty();
        }

        // Notify the graph of changes
        GetGraph()->NotifyGraphChanged();
    }
}

FText UK2Node_MyCustomNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return LOCTEXT("DynamicTypeNode_Title", "Get Object Variable Value");
}

FText UK2Node_MyCustomNode::GetTooltipText() const
{
    return Super::GetTooltipText();
}

FText UK2Node_MyCustomNode::GetMenuCategory() const
{
    return LOCTEXT("CustomCategory", "Custom");
}

void UK2Node_MyCustomNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();
    if (!ActionRegistrar.IsOpenForRegistration(ActionKey))
        return;

    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
	
    // Bind a custom filter for context
    Spawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(&UK2Node_MyCustomNode::CustomizeNodeForContext);
	
    ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
}


void UK2Node_MyCustomNode::CustomizeNodeForContext(UEdGraphNode* NewNode, bool bIsTemplateNode)
{
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(NewNode->GetGraph());
    if (!Blueprint || !Blueprint->GeneratedClass->IsChildOf(UGorgeousObjectVariable::StaticClass()))
    {
        // Optional: mark node as hidden or delete it
        NewNode->DestroyNode(); // Prevents it from showing up in the context menu
    }
}

UEdGraphPin* UK2Node_MyCustomNode::BuildReturnValuePin()
{
    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
    
    const UClass* TargetClass = GetConnectedUObjectInstance(GetTargetPin());
    UEdGraphPin* OutputPin = GetReturnValuePin();

    // If the target object is valid, proceed with type inference
    if (TargetClass)
    {
        // If the target is a GorgeousObjectVariable, set the type to Wildcard
        if (TargetClass == UGorgeousObjectVariable::StaticClass())
        {
            // Keep the type as wildcard for GorgeousObjectVariable
            if (OutputPin)
            {
                OutputPin->PinType = FEdGraphPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
            }
        }
        else
        {
            // If the target is not a GorgeousObjectVariable, try to get the PinType settings for the "Value" property
            if (TargetClass->FindPropertyByName(TEXT("Value")) && OutputPin)
            {
                OutputPin->PinType = Cast<UGorgeousObjectVariable>(TargetClass->GetDefaultObject())->GetObjectVariablePinConfiguration().GetMappedPinType();
            }
        }
    }
    else
    {
        // If no target is connected, reset the type to wildcard
        if (OutputPin)
        {
            OutputPin->PinType = FEdGraphPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
        }
    }

    return OutputPin;
}

UClass* UK2Node_MyCustomNode::GetConnectedUObjectInstance(const UEdGraphPin* TargetPin)
{
    if (!TargetPin || TargetPin->Direction != EGPD_Input)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetConnectedUObjectInstance: Invalid or non-input pin provided."));
        return nullptr;
    }

    UClass* FoundClass = nullptr;

    // --- 1. Check for Connections ---
    if (TargetPin->LinkedTo.Num() > 0)
    {
        // Use the first connection, as per the simplified approach in the report.
        const UEdGraphPin* LinkedPin = TargetPin->LinkedTo[0];
        if (!LinkedPin)
        {
            UE_LOG(LogTemp, Warning, TEXT("GetConnectedUObjectInstance: Target pin '%s' LinkedTo[0] is null."), *TargetPin->GetFName().ToString());
            return nullptr;
        }

        UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();
        if (!LinkedNode)
        {
            UE_LOG(LogTemp, Warning, TEXT("GetConnectedUObjectInstance: Could not get owning node for linked pin '%s'."), *LinkedPin->GetFName().ToString());
            return nullptr;
        }

        // --- 2. Identify Connected Node Type ---
        if (UK2Node_VariableGet* VarGetNode = Cast<UK2Node_VariableGet>(LinkedNode))
        {
            UE_LOG(LogTemp, Log, TEXT("GetConnectedUObjectInstance: Linked node is VariableGet: %s"), *VarGetNode->GetNodeTitle(ENodeTitleType::ListView).ToString());
            
            FoundClass = FindObject<UClass>(ANY_PACKAGE, *VarGetNode->Pins[0]->PinType.PinSubCategoryObject->GetName());
            
            if (FoundClass)
            {
                UE_LOG(LogTemp, Log, TEXT("GetConnectedUObjectInstance: Found object '%s' from VariableGet node '%s'."),
                    *FoundClass->GetName(), *VarGetNode->GetNodeTitle(ENodeTitleType::ListView).ToString());
            }
        }
        // Other Node Types
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("GetConnectedUObjectInstance: Connected node '%s' is of an unhandled type (%s) for pin '%s'."),
                *LinkedNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
                *LinkedNode->GetClass()->GetName(),
                *TargetPin->GetFName().ToString());
            // Potential future extensions: Handle UK2Node_Self, UK2Node_MakeArray, UK2Node_AssignmentStatement, etc.
        }
    }
    // --- 3. Handle Unconnected Pin Case ---
    else // TargetPin is not connected
    {
        UE_LOG(LogTemp, Log, TEXT("GetConnectedUObjectInstance: Target pin '%s' is not connected. Checking its DefaultObject."), *TargetPin->GetFName().ToString());
        // Check the DefaultObject directly on the target pin itself
        if (TargetPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Object || TargetPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Class)
        {
            if (const UObject* FoundObject = Cast<UObject>(TargetPin->DefaultObject))
            {
                UE_LOG(LogTemp, Log, TEXT("GetConnectedUObjectInstance: Found object '%s' from Target pin's DefaultObject (unconnected)."), *FoundObject->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("GetConnectedUObjectInstance: Target pin '%s' DefaultObject is null or not a UObject/UClass."), *TargetPin->GetFName().ToString());
            }
        }
         else
         {
             UE_LOG(LogTemp, Warning, TEXT("GetConnectedUObjectInstance: Unconnected target pin '%s' is not an Object or Class pin."), *TargetPin->GetFName().ToString());
         }
    }

    return FoundClass;
}

bool UK2Node_MyCustomNode::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
    return true;
    /**
    if (const UBlueprint* BP = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph))
    {
        return BP->GeneratedClass->IsChildOf(UGorgeousObjectVariable::StaticClass());
    }
    return false;*/
}
