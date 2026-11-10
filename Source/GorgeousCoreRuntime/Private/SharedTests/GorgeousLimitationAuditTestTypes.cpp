#include "NativeGameplayTags.h"
#include "SharedTests/GorgeousLimitationAuditTestTypes.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "Engine/World.h"
#include "TimerManager.h"
void UGorgeousNetworkAuditRPCVariable::OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context)
{
    Super::OnReplicationActivated_Implementation(Context);
    for (auto Type:{EGorgeousAutoReplicationRPCType::EReliableServer,EGorgeousAutoReplicationRPCType::EUnreliableServer,EGorgeousAutoReplicationRPCType::EReliableClient,EGorgeousAutoReplicationRPCType::EUnreliableClient,EGorgeousAutoReplicationRPCType::EReliableMulticast,EGorgeousAutoReplicationRPCType::EUnreliableMulticast})
    { BindRPCHandler(TEXT("AuditImmediate"),Type); BindRPCHandler(TEXT("AuditDeferred"),Type); }
}
void UGorgeousNetworkAuditRPCVariable::AuditImmediate(UInteger_SOV* ReturnValue)
{
    if (ReturnValue) { int32 Value=731; ReturnValue->SetIntegerSingleObjectVariable_Implementation(NAME_None,Value); }
}
void UGorgeousNetworkAuditRPCVariable::AuditDeferred(UInteger_SOV* ReturnValue,const FGorgeousQueuedRPC& Request)
{
    if (ReturnValue) { int32 Value=937; ReturnValue->SetIntegerSingleObjectVariable_Implementation(NAME_None,Value); }
    if (UWorld* World=GetWorld())
    {
        TWeakObjectPtr<UGorgeousNetworkAuditRPCVariable> WeakThis(this);
        World->GetTimerManager().SetTimerForNextTick([WeakThis,Request]() {
            if (auto* Self=WeakThis.Get()) UGorgeousAutoReplicationRPCRequestAsyncAction::MarkAutoReplicationRPCResponderReady(Self,Request,EGorgeousRPCReadyState::Ready);
        });
    }
}

int32 FGorgeousRPCUnsafeImportAudit::ImportCalls=0;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GorgeousNetworkAuditStat,"Gorgeous.Test.Stat.NetworkAudit");
FGameplayTag GetGorgeousNetworkAuditStatTag() { return TAG_GorgeousNetworkAuditStat.GetTag(); }
