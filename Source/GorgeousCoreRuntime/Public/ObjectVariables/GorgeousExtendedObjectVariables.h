#pragma once
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GameplayTagContainer.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "GorgeousExtendedObjectVariables.generated.h"

/** Reflected Vector value with ordinary serialization, GC and AutoReplication support. */
UCLASS(BlueprintType, DisplayName="Vector Object Variable")
class GORGEOUSCORERUNTIME_API UGorgeousVectorObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
    UE_SETUP_OBJECT_VARIABLE_AUTO_REPLICATION
public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Value",meta=(ExposeOnSpawn="true"))
    FVector Value = FVector::ZeroVector;
    UFUNCTION(BlueprintPure,Category="Gorgeous Core|Object Variables")
    FVector GetValue() const { return Value; }
    UFUNCTION(BlueprintCallable,Category="Gorgeous Core|Object Variables")
    void SetValue(const FVector& NewValue) { SetNativeObjectVariableValue(Value,NewValue); }
};

/** Reflected Transform value with ordinary serialization, GC and AutoReplication support. */
UCLASS(BlueprintType, DisplayName="Transform Object Variable")
class GORGEOUSCORERUNTIME_API UGorgeousTransformObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
    UE_SETUP_OBJECT_VARIABLE_AUTO_REPLICATION
public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Value",meta=(ExposeOnSpawn="true"))
    FTransform Value = FTransform::Identity;
    UFUNCTION(BlueprintPure,Category="Gorgeous Core|Object Variables")
    FTransform GetValue() const { return Value; }
    UFUNCTION(BlueprintCallable,Category="Gorgeous Core|Object Variables")
    void SetValue(const FTransform& NewValue) { SetNativeObjectVariableValue(Value,NewValue); }
};

/** Reflected GameplayTag value with ordinary serialization, GC and AutoReplication support. */
UCLASS(BlueprintType, DisplayName="GameplayTag Object Variable")
class GORGEOUSCORERUNTIME_API UGorgeousGameplayTagObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
    UE_SETUP_OBJECT_VARIABLE_AUTO_REPLICATION
public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Value",meta=(ExposeOnSpawn="true"))
    FGameplayTag Value = FGameplayTag();
    UFUNCTION(BlueprintPure,Category="Gorgeous Core|Object Variables")
    FGameplayTag GetValue() const { return Value; }
    UFUNCTION(BlueprintCallable,Category="Gorgeous Core|Object Variables")
    void SetValue(const FGameplayTag& NewValue) { SetNativeObjectVariableValue(Value,NewValue); }
};

/** Reflected Struct value with ordinary serialization, GC and AutoReplication support. */
UCLASS(BlueprintType, DisplayName="Struct Object Variable")
class GORGEOUSCORERUNTIME_API UGorgeousStructObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
    UE_SETUP_OBJECT_VARIABLE_AUTO_REPLICATION
public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Value",meta=(ExposeOnSpawn="true"))
    FInstancedStruct Value = FInstancedStruct();
    UFUNCTION(BlueprintPure,Category="Gorgeous Core|Object Variables")
    FInstancedStruct GetValue() const { return Value; }
    UFUNCTION(BlueprintCallable,Category="Gorgeous Core|Object Variables")
    void SetValue(const FInstancedStruct& NewValue) { SetNativeObjectVariableValue(Value,NewValue); }
};

/** Enum values retain both their enum definition and its integral value. */
UCLASS(BlueprintType, DisplayName="Enum Object Variable")
class GORGEOUSCORERUNTIME_API UGorgeousEnumObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Value")
    TObjectPtr<UEnum> EnumType;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,SaveGame,Category="Value",meta=(ExposeOnSpawn="true"))
    int64 Value=0;
    UFUNCTION(BlueprintPure,Category="Gorgeous Core|Object Variables")
    int64 GetValue() const { return Value; }
    UFUNCTION(BlueprintCallable,Category="Gorgeous Core|Object Variables")
    bool SetValue(int64 NewValue) { if (EnumType && EnumType->GetIndexByValue(NewValue)==INDEX_NONE) return false; SetNativeObjectVariableValue(Value,NewValue); return true; }
protected:
    virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override
    { Super::OnReplicationActivated_Implementation(Context); RegisterReplicatedProperty(TEXT("Value")); RegisterReplicatedProperty(TEXT("EnumType")); }
};
