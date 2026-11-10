#pragma once
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "AutoReplication/GorgeousAutoReplicationOwnerComponent.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatComponent_AC.h"
#include "GorgeousNetworkAuditActors.generated.h"

UCLASS(Transient)
class AGorgeousNetworkAuditController : public AGorgeousPlayerController
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 UnregisteredWitness=0;
    UFUNCTION() void UnregisteredAuditFunction() { ++UnregisteredWitness; }
};
UCLASS(Transient)
class AGorgeousNetworkAuditGameMode : public AGorgeousGameMode
{
    GENERATED_BODY()
public:
    AGorgeousNetworkAuditGameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void StartPlay() override;
};

UCLASS(Transient)
class UGorgeousNetworkAuditCallbacks : public UObject
{
    GENERATED_BODY()
public:
    int32 Completed=0;
    int32 Failed=0;
    UFUNCTION() void OnComplete(const FGorgeousAutoReplicationRPCAsyncResult& Result) { ++Completed; }
    UFUNCTION() void OnFailure(const FGorgeousAutoReplicationRPCAsyncResult& Result) { ++Failed; }
};

UCLASS(Transient)
class AGorgeousNetworkAuditActor : public AActor
{
    GENERATED_BODY()
public:
    AGorgeousNetworkAuditActor();
    UPROPERTY() TObjectPtr<UGorgeousAutoReplicationOwnerComponent> AutoReplication;
    UPROPERTY() TObjectPtr<UGorgeousStatComponent_AC> Stats;
    virtual void BeginPlay() override;
    bool bObservedStatParentSignal=false;
    UFUNCTION() void OnAuditStatSignal(FGameplayTag SignalTag,const FInstancedStruct& Payload);
};
