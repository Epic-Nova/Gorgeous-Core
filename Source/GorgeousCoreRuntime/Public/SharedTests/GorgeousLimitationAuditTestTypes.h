#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "GorgeousLimitationAuditTestTypes.generated.h"

GORGEOUSCORERUNTIME_API FGameplayTag GetGorgeousNetworkAuditStatTag();

/** Isolated receiver for argument serialization and dispatch authorization regression tests. */
UCLASS()
class UGorgeousLimitationAuditReceiver : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 Calls = 0;
    UPROPERTY() int32 Changes=0;
    UFUNCTION() void OnStatChanged(AActor* Actor,FGameplayTag Tag,float Value) { ++Changes; }
    UFUNCTION() void OnVariableChanged(UGorgeousObjectVariable* Variable,FName PropertyName) { ++Changes; }
    UPROPERTY() FName LastName;
    UPROPERTY() TArray<FString> LastValues;
    UFUNCTION() void Receive(FName Token, const TArray<FString>& Values) { ++Calls; LastName=Token; LastValues=Values; }
};

/** Explicitly bound RPC seam used by real multi-client regression tests. */
UCLASS(Transient)
class GORGEOUSCORERUNTIME_API UGorgeousNetworkAuditRPCVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
protected:
    virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override;
public:
    UFUNCTION() void AuditImmediate(UInteger_SOV* ReturnValue);
    UFUNCTION() void AuditDeferred(UInteger_SOV* ReturnValue,const FGorgeousQueuedRPC& Request);
};

UCLASS(Transient)
class UGorgeousConditionAuditVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 Value=731;
    void ObserveValueChanges(UGorgeousLimitationAuditReceiver* Listener)
    { OnPropertyValueChanged.AddDynamic(Listener,&UGorgeousLimitationAuditReceiver::OnVariableChanged); }
    void SetAuditNativeValue(const int32& NewValue) { SetNativeObjectVariableValue(Value,NewValue); }
    void SetAuditNamedValue(const int32& NewValue) { SetDynamicProperty<int32,FProperty>(TEXT("Value"),NewValue); }
    FGorgeousReplicatedPropertyConfig Configuration;
protected:
    virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override
    { Super::OnReplicationActivated_Implementation(Context); RegisterReplicatedProperty(TEXT("Value"),EGorgeousReplicationMode::EProperty,true,Configuration); }
};

UCLASS(Transient)
class UGorgeousTrunkAuditOwner : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;
    UPROPERTY() TMap<FName,FGorgeousObjectVariableEntry> AdditionalData;
    UPROPERTY() TArray<FGorgeousReplicatedVariableEntry> ReplicatedData;
    FGorgeousAutoReplicationMixin Mixin;
};

/** Parser witness: untrusted result import must reject this before executing native code. */
USTRUCT()
struct FGorgeousRPCUnsafeImportAudit
{
    GENERATED_BODY()
    UPROPERTY() FString Text;
    static int32 ImportCalls;
    bool ImportTextItem(const TCHAR*& Buffer,int32 Flags,UObject* Parent,FOutputDevice* Errors)
    { ++ImportCalls; return false; }
};
template<> struct TStructOpsTypeTraits<FGorgeousRPCUnsafeImportAudit> : TStructOpsTypeTraitsBase2<FGorgeousRPCUnsafeImportAudit>
{ enum { WithImportTextItem=true }; };
UCLASS(Transient)
class UGorgeousRPCUnsafeImportAuditVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
public:
    UPROPERTY() TArray<FGorgeousRPCUnsafeImportAudit> Value;
};
