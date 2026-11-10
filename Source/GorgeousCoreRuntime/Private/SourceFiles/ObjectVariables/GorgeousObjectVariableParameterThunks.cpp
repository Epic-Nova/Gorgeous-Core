#include "ObjectVariables/GorgeousObjectVariable.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"

DEFINE_FUNCTION(IGorgeousObjectVariableInteraction_I::execGetObjectVariableParameter)
{
    Stack.StepCompiledIn<FProperty>(nullptr);
    FProperty* Output=Stack.MostRecentProperty; void* OutputAddress=Stack.MostRecentPropertyAddress;
    P_GET_PROPERTY(FNameProperty,Name);
    P_FINISH;
    UGorgeousObjectVariable* Variable=Cast<UGorgeousObjectVariable>(Stack.Object);
    FProperty* Source=Variable ? FindFProperty<FProperty>(Variable->GetClass(),Name.IsNone() ? FName(TEXT("Value")) : Name) : nullptr;
    if (Source && Output && OutputAddress && Source->SameType(Output)) Source->CopyCompleteValue(OutputAddress,Source->ContainerPtrToValuePtr<void>(Variable));
}
DEFINE_FUNCTION(IGorgeousObjectVariableInteraction_I::execSetObjectVariableParameter)
{
    Stack.StepCompiledIn<FProperty>(nullptr);
    FProperty* Input=Stack.MostRecentProperty; void* InputAddress=Stack.MostRecentPropertyAddress;
    P_GET_PROPERTY(FNameProperty,Name);
    P_FINISH;
    UGorgeousObjectVariable* Variable=Cast<UGorgeousObjectVariable>(Stack.Object);
    if (Name.IsNone()) Name=TEXT("Value");
    FProperty* Target=Variable ? FindFProperty<FProperty>(Variable->GetClass(),Name) : nullptr;
    if (Target && Input && InputAddress && Target->SameType(Input) && !Target->Identical(Target->ContainerPtrToValuePtr<void>(Variable),InputAddress) && Variable->ValidateVariableAssignment(Name,Target,InputAddress))
    { Target->CopyCompleteValue(Target->ContainerPtrToValuePtr<void>(Variable),InputAddress); Variable->MarkPropertyDirty(Name); }
}

void UGorgeousObjectVariable::GetObjectVariableParameter_Implementation(int32& OutValue,FName Name)
{ GetDynamicProperty<int32,FProperty>(Name.IsNone() ? FName(TEXT("Value")) : Name,OutValue); }
void UGorgeousObjectVariable::SetObjectVariableParameter_Implementation(const int32& NewValue,FName Name)
{ SetDynamicProperty<int32,FProperty>(Name.IsNone() ? FName(TEXT("Value")) : Name,NewValue); }
