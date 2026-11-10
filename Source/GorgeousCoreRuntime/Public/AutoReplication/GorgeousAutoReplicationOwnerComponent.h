#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "GorgeousAutoReplicationOwnerComponent.generated.h"

/** Opt-in AutoReplication for a replicated actor or pawn. Add to the actor's
 * Blueprint or constructor so it exists on both endpoints before delivery. */
UCLASS(ClassGroup=(Gorgeous),meta=(BlueprintSpawnableComponent))
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationOwnerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UGorgeousAutoReplicationOwnerComponent();
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Gorgeous|AutoReplication")
    TMap<FName,FGorgeousObjectVariableEntry> AdditionalGorgeousData;
    UPROPERTY(EditAnywhere,Category="Gorgeous|AutoReplication")
    FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;
    UFUNCTION(BlueprintCallable,Category="Gorgeous|AutoReplication")
    UGorgeousObjectVariable* RegisterObjectVariable(FName Key,TSubclassOf<UGorgeousObjectVariable> Class,const FGorgeousAutoReplicationStreamConfig& Configuration);
    UFUNCTION(BlueprintPure,Category="Gorgeous|AutoReplication")
    UGorgeousObjectVariable* FindObjectVariable(FName Key);
    FGorgeousAutoReplicationMixin& GetAutoReplicationMixin();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(Transient) TArray<FGorgeousReplicatedVariableEntry> RuntimeEntries;
    FGorgeousAutoReplicationMixin Mixin;
    bool bBound=false;
};
