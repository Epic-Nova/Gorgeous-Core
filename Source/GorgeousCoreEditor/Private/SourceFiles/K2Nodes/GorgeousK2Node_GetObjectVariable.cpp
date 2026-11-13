#include "K2Nodes/GorgeousK2Node_GetObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_VariableGet.h"
#include "KismetCompiler.h"
#include "UObject/UnrealType.h"

void UGorgeousK2Node_GetObjectVariable::AllocateDefaultPins()
{
    CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Object,UGorgeousObjectVariable::StaticClass(),TEXT("Target"));
    CreatePin(EGPD_Output,UEdGraphSchema_K2::PC_Wildcard,TEXT("ReturnValue"));
    RefreshValueType();
}
UClass* UGorgeousK2Node_GetObjectVariable::ResolveTargetClass() const
{
    const UEdGraphPin* Target=FindPin(TEXT("Target"));
    if (!Target) return nullptr;
    if (!Target->LinkedTo.IsEmpty()) return Cast<UClass>(Target->LinkedTo[0]->PinType.PinSubCategoryObject.Get());
    return Target->DefaultObject ? Target->DefaultObject->GetClass() : nullptr;
}
void UGorgeousK2Node_GetObjectVariable::RefreshValueType()
{
    UEdGraphPin* Output=FindPin(TEXT("ReturnValue"));
    if (!Output) return;
    FEdGraphPinType Type;
    Type.PinCategory=UEdGraphSchema_K2::PC_Wildcard;
    if (UClass* Class=ResolveTargetClass())
        if (FProperty* Value=FindFProperty<FProperty>(Class,TEXT("Value"))) GetDefault<UEdGraphSchema_K2>()->ConvertPropertyToPinType(Value,Type);
    Output->PinType=Type;
}
void UGorgeousK2Node_GetObjectVariable::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
    Super::NotifyPinConnectionListChanged(Pin);
    if (Pin==FindPin(TEXT("Target"))) RefreshValueType();
}
void UGorgeousK2Node_GetObjectVariable::PinDefaultValueChanged(UEdGraphPin* Pin)
{
    Super::PinDefaultValueChanged(Pin);
    if (Pin==FindPin(TEXT("Target"))) RefreshValueType();
}
void UGorgeousK2Node_GetObjectVariable::PostReconstructNode() { Super::PostReconstructNode(); RefreshValueType(); }
FText UGorgeousK2Node_GetObjectVariable::GetNodeTitle(ENodeTitleType::Type) const { return NSLOCTEXT("GorgeousOV","TypedGet","Get Object Variable Value"); }
FText UGorgeousK2Node_GetObjectVariable::GetMenuCategory() const { return NSLOCTEXT("GorgeousOV","Category","Gorgeous Core|Object Variables"); }
void UGorgeousK2Node_GetObjectVariable::GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const
{
    if (Registrar.IsOpenForRegistration(GetClass())) Registrar.AddBlueprintAction(GetClass(),UBlueprintNodeSpawner::Create(GetClass()));
}
void UGorgeousK2Node_GetObjectVariable::ExpandNode(FKismetCompilerContext& Context, UEdGraph* Graph)
{
    UClass* Class=ResolveTargetClass();
    FProperty* Value=Class ? FindFProperty<FProperty>(Class,TEXT("Value")) : nullptr;
    if (!Value || !Class->IsChildOf(UGorgeousObjectVariable::StaticClass()))
    {
        Context.MessageLog.Error(TEXT("@@ requires a concrete Object Variable target with a reflected Value property."),this);
        BreakAllNodeLinks(); return;
    }
    UK2Node_VariableGet* Get=Context.SpawnIntermediateNode<UK2Node_VariableGet>(this,Graph);
    Get->VariableReference.SetExternalMember(TEXT("Value"),Class);
    Get->AllocateDefaultPins();
    UEdGraphPin* Self=Get->FindPin(UEdGraphSchema_K2::PN_Self);
    UEdGraphPin* Target=FindPinChecked(TEXT("Target"));
    if (!Self) { Context.MessageLog.Error(TEXT("@@ could not resolve target pin."),this); BreakAllNodeLinks(); return; }
    if (!Target->LinkedTo.IsEmpty()) Context.MovePinLinksToIntermediate(*Target,*Self);
    else Self->DefaultObject=Target->DefaultObject;
    Context.MovePinLinksToIntermediate(*FindPinChecked(TEXT("ReturnValue")),*Get->FindPinChecked(TEXT("Value")));
    BreakAllNodeLinks();
}
