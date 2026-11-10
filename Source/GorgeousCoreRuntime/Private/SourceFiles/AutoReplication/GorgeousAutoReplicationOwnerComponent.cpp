#include "AutoReplication/GorgeousAutoReplicationOwnerComponent.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GameFramework/Actor.h"

UGorgeousAutoReplicationOwnerComponent::UGorgeousAutoReplicationOwnerComponent()
{
    PrimaryComponentTick.bCanEverTick=false;
    SetIsReplicatedByDefault(true);
}
FGorgeousAutoReplicationMixin& UGorgeousAutoReplicationOwnerComponent::GetAutoReplicationMixin()
{
    if (!bBound && GetOwner() && !GetOwner()->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        Mixin.Bind(GetOwner(),&AdditionalGorgeousData,&RuntimeEntries,&DefaultObjectVariableTrunk);
        bBound=true;
        Mixin.InitializeAdditionalData(true);
    }
    return Mixin;
}
void UGorgeousAutoReplicationOwnerComponent::BeginPlay()
{
    Super::BeginPlay();
    GetAutoReplicationMixin();
}
void UGorgeousAutoReplicationOwnerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    for (auto& Pair:AdditionalGorgeousData)
        if (Pair.Value.DefaultValue) Pair.Value.DefaultValue->SetNetworkingEnabled(false);
    if (bBound) Mixin.InitializeAdditionalData(false);
    Super::EndPlay(Reason);
}
UGorgeousObjectVariable* UGorgeousAutoReplicationOwnerComponent::RegisterObjectVariable(FName Key,TSubclassOf<UGorgeousObjectVariable> Class,const FGorgeousAutoReplicationStreamConfig& Configuration)
{
    if (Key.IsNone() || !GetOwner() || !GetOwner()->HasAuthority() || !Class || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) return nullptr;
    auto& BoundMixin=GetAutoReplicationMixin();
    auto& Entry=AdditionalGorgeousData.FindOrAdd(Key);
    if (Entry.DefaultValue && Entry.DefaultValue->GetClass()!=Class.Get()) return nullptr;
    if (!Entry.DefaultValue) Entry.DefaultValue=NewObject<UGorgeousObjectVariable>(GetOwner(),Class);
    Entry.bReplicate=true; Entry.bOverrideStreamConfig=true; Entry.StreamConfigOverride=Configuration;
    BoundMixin.InitializeAdditionalData(true);
    return Entry.DefaultValue;
}
UGorgeousObjectVariable* UGorgeousAutoReplicationOwnerComponent::FindObjectVariable(FName Key)
{
    UGorgeousObjectVariable* Result=nullptr;
    GetAutoReplicationMixin().TryGetValue(Key,Result);
    return Result;
}
