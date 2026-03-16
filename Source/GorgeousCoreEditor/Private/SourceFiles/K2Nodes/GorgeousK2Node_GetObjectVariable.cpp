#include "K2Nodes/GorgeousK2Node_GetObjectVariable.h"

#if 0
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "Engine/Blueprint.h" // Required for GetBlueprintClassFromNode() context
#include "Kismet2/BlueprintEditorUtils.h" // May be needed for more complex scenarios or utilities
#include "UObject/UnrealType.h" // Required for FProperty, FObjectProperty
#include "Logging/LogMacros.h" // For UE_LOG
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "CoreUObject.h"
#include "Editor.h"
#include "ObjectVariables/GorgeousObjectVariable.h"

#define LOCTEXT_NAMESPACE "K2Node"

//@TODO: Absolutely does not function, compiling throws errors because this node does not have a equivalent bitecode executor. Implement for v1.1, for now this is just a proof of concept for the dynamic pin type logic and the general idea of how to implement such a node.

void UGorgeousK2Node_GetObjectVariable::AllocateDefaultPins()
{
    Super::AllocateDefaultPins();
    
    BuildTargetPin();
    BuildReturnValuePin();
}

void UGorgeousK2Node_GetObjectVariable::ReconstructNode()
{
    Super::ReconstructNode();
    
    BuildTargetPin();
    BuildReturnValuePin();

    GetGraph()->NotifyGraphChanged();
}

void UGorgeousK2Node_GetObjectVariable::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
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

FText UGorgeousK2Node_GetObjectVariable::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return LOCTEXT("DynamicTypeNode_Title", "Get Object Variable Value");
}

FText UGorgeousK2Node_GetObjectVariable::GetMenuCategory() const
{
    return LOCTEXT("CustomCategory", "Gorgeous Core|Object Variables");
}

void UGorgeousK2Node_GetObjectVariable::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();
    if (!ActionRegistrar.IsOpenForRegistration(ActionKey))
        return;

    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
	
    // Bind a custom filter for context
   // Spawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(&UGorgeousK2Node_GetObjectVariable::CustomizeNodeForContext);
	
    ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
}


/*void UGorgeousK2Node_GetObjectVariable::CustomizeNodeForContext(UEdGraphNode* NewNode, bool bIsTemplateNode)
{
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(NewNode->GetGraph());
    if (!Blueprint || !Blueprint->GeneratedClass->IsChildOf(UGorgeousObjectVariable::StaticClass()))
    {
        // Optional: mark node as hidden or delete it
        NewNode->DestroyNode(); // Prevents it from showing up in the context menu
    }
}*/

UEdGraphPin* UGorgeousK2Node_GetObjectVariable::BuildTargetPin()
{
    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
    const UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(GetGraph());

    const bool bIsSuitableForInput = Blueprint && Blueprint->GeneratedClass 
    && !Blueprint->GeneratedClass->IsChildOf(UGorgeousObjectVariable::StaticClass());
        
    if (bIsSuitableForInput)
    {
        return CreatePin(EGPD_Input, Schema->PC_Object, UGorgeousObjectVariable::StaticClass(), TEXT("Target"));
    }
    
    return nullptr;
}

UEdGraphPin* UGorgeousK2Node_GetObjectVariable::BuildReturnValuePin()
{
    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
    
    if (!GetReturnValuePin())
    {
        return CreatePin(EGPD_Output, Schema->PC_Wildcard, TEXT("ReturnValue"));
    }
    
    //either the target pin or self pin
    
    const UClass* TargetClass = ResolveTargetClass();
    
    GT_I_LOG("GT.Test", TEXT("Building ReturnValue pin. TargetClass determined to be: %s"), TargetClass ? *TargetClass->GetName() : TEXT("None"));

    UEdGraphPin* OutputPin = GetReturnValuePin();
    
    // If the target is a GorgeousObjectVariable, set the type to Wildcard
    if (TargetClass == UGorgeousObjectVariable::StaticClass())
    {
        // Keep the type as wildcard for GorgeousObjectVariable
        if (OutputPin)
        {
            GT_I_LOG("GT.Test", TEXT("Target is a GorgeousObjectVariable, setting ReturnValue pin type to Wildcard."));
            OutputPin->PinType = FEdGraphPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
        }
    }
    else
    {
        // If the target class is a child of GorgeousObjectVariable, set the pin type based on the PinConfiguration of the target class
        if (Cast<UGorgeousObjectVariable>(TargetClass->GetDefaultObject()))
        {
            GT_I_LOG("GT.Test", TEXT("Target is a '%s', setting ReturnValue pin type based on the 'PinConfigration' property of the target class."), *TargetClass->GetName());
            OutputPin->PinType = Cast<UGorgeousObjectVariable>(TargetClass->GetDefaultObject())->GetObjectVariablePinConfiguration().GetMappedPinType();
            GetGraph()->NotifyNodeChanged(this);
        }
    }

    return OutputPin;
}

void UGorgeousK2Node_GetObjectVariable::PostPlacedNewNode()
{
    Super::PostPlacedNewNode();
    BindListeners();
}

void UGorgeousK2Node_GetObjectVariable::PostLoad()
{
    Super::PostLoad();
    BindListeners();
}

void UGorgeousK2Node_GetObjectVariable::BeginDestroy()
{
    UnbindListeners();
    Super::BeginDestroy();
}

UClass* UGorgeousK2Node_GetObjectVariable::ResolveTargetClass() const
{
    if (UEdGraphPin* TargetPin = GetTargetPin())
    {
        // 1. Check Connections, then take the class from the first connected pin
        if (TargetPin->LinkedTo.Num() > 0)
        {
            if (const UEdGraphPin* LinkedPin = TargetPin->LinkedTo[0]; 
                LinkedPin->PinType.PinSubCategoryObject.IsValid())
            {
                if (UClass* LinkedClass = Cast<UClass>(LinkedPin->PinType.PinSubCategoryObject.Get()))
                {
                    return LinkedClass;
                }
            }
        }

        // 2. If no connections, check if the pin has a default object and return its class
        if (const UObject* DefaultObj = TargetPin->DefaultObject)
        {
            return DefaultObj->GetClass();
        }
    }
    
    // 3. Fallback to the owning blueprint's generated class as a last resort (though this may not be ideal for all use cases)
    if (UBlueprint* BP = FBlueprintEditorUtils::FindBlueprintForNode(this))
    {
        return BP->GeneratedClass;
    }

    return nullptr;
}

void UGorgeousK2Node_GetObjectVariable::HandleBlueprintCompiled(UBlueprint* Blueprint)
{
    if (const UBlueprint* OwnerBP = FBlueprintEditorUtils::FindBlueprintForNode(this); Blueprint == OwnerBP)
    {
        QueueRefresh();
    }
}

void UGorgeousK2Node_GetObjectVariable::HandleObjectModified(UObject* Object)
{
    if (!Object)
        return;

    // if a GorgeousObjectVariable instance changed
    if (Object->IsA(UGorgeousObjectVariable::StaticClass()))
    {
        QueueRefresh();
        return;
    }

    // if owning blueprint asset modified
    if (const UBlueprint* BP = Cast<UBlueprint>(Object))
    {
        if (BP == FBlueprintEditorUtils::FindBlueprintForNode(this))
        {
            QueueRefresh();
        }
    }
}

void UGorgeousK2Node_GetObjectVariable::BindListeners()
{
    if (GEditor && !BlueprintCompiledHandle.IsValid())
    {
        BlueprintCompiledHandle =
            GEditor->OnBlueprintPreCompile().AddUObject(
                this,
                &UGorgeousK2Node_GetObjectVariable::HandleBlueprintCompiled);
    }

    if (!ObjectModifiedHandle.IsValid())
    {
        ObjectModifiedHandle =
            FCoreUObjectDelegates::OnObjectModified.AddUObject(
                this,
                &UGorgeousK2Node_GetObjectVariable::HandleObjectModified);
    }
}

void UGorgeousK2Node_GetObjectVariable::UnbindListeners()
{
    if (GEditor && BlueprintCompiledHandle.IsValid())
    {
        GEditor->OnBlueprintCompiled().Remove(BlueprintCompiledHandle);
        BlueprintCompiledHandle.Reset();
    }

    if (ObjectModifiedHandle.IsValid())
    {
        FCoreUObjectDelegates::OnObjectModified.Remove(ObjectModifiedHandle);
        ObjectModifiedHandle.Reset();
    }
}

void UGorgeousK2Node_GetObjectVariable::QueueRefresh()
{
    if (bRefreshQueued)
        return;

    bRefreshQueued = true;

    if (!GEditor)
        return;

    UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
    if (!EditorWorld)
        return;

    EditorWorld->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateUObject(
            this,
            &UGorgeousK2Node_GetObjectVariable::PerformDeferredRefresh));
}

void UGorgeousK2Node_GetObjectVariable::PerformDeferredRefresh()
{
    bRefreshQueued = false;

    if (!IsValid(this))
        return;

    ReconstructNode();

    if (UEdGraph* Graph = GetGraph())
    {
        Graph->NotifyGraphChanged();
    }
}

bool UGorgeousK2Node_GetObjectVariable::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
    return true; //In an old iteration we didn't want this node to be placable outside UGorgeousObjectVariable, but since this would violate with the freedom of the flexibility of the plugin, we've decided to support this.
}

#endif