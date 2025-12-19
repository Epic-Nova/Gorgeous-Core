// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "GorgeousObjectVariableStructures.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "Interfaces/GorgeousObjectVariableInteraction_I.h"
#include "ObjectVariables/Helpers/GorgeousObjectVariableHelperMacros.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesSetter_I.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "Net/UnrealNetwork.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousObjectVariable.generated.h"
//<-------------------------------------------------------------------------->

class UGorgeousAutoReplicationRPCRequestAsyncAction;
class UGorgeousRPC_OV;
class AActor;
class AController;
class AGorgeousPlayerController;
class UGorgeousRootObjectVariable;
class FArchive;
class UGorgeousObjectVariable;
#if WITH_AUTOMATION_TESTS
struct FGorgeousObjectVariablePerfTestAccess;
#endif

UENUM(BlueprintType)
enum class EGorgeousObjectVariableAccessPolicy : uint8
{
	Everyone,
	OwningControllerOnly,
	Custom
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousObjectVariableRootConfiguration
{
	GENERATED_BODY()

public:
	FGorgeousObjectVariableRootConfiguration();

	/** Optional override that selects a named root from the developer settings. */
	//UGorgeousRootObjectVariable::GetRegisteredRootNames
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Root", meta = (GetOptions = "/Script/GorgeousCoreRuntime.GorgeousRootObjectVariable.GetRegisteredRootNames"))
	FName PreferredRootName;

	/** Returns the explicitly configured root or falls back to the developer default. */
	FName ResolvePreferredRootName() const;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousRootNetworkAccessConfig
{
	GENERATED_BODY()

public:
	FGorgeousRootNetworkAccessConfig();

	/** Enables the root network stack for this variable when networking is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Root Network Stack")
	bool bExposeThroughRootNetworkStack;

	/** Determines which controllers are allowed to receive replicated data. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Root Network Stack", meta = (EditCondition = "bExposeThroughRootNetworkStack", EditConditionHides))
	EGorgeousObjectVariableAccessPolicy AccessPolicy;

	/** Optional logical channel name used by the networking layer to group streams. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Root Network Stack", meta = (EditCondition = "bExposeThroughRootNetworkStack", EditConditionHides))
	FName ReplicationChannel;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousSharedNetworkStackConfig
{
	GENERATED_BODY()

public:
	FGorgeousSharedNetworkStackConfig();

	/** Determines access policy when routing through the shared network stack. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shared Network Stack")
	EGorgeousObjectVariableAccessPolicy AccessPolicy;

	/** Optional logical channel name used when leveraging the shared stack. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shared Network Stack")
	FName ReplicationChannel;

	void Reset();
};

UENUM(BlueprintType)
enum class EGorgeousRepNotifyPolicy : uint8
{
	OnChanged,
	Always
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousReplicatedPropertyConfig
{
	GENERATED_BODY()

public:
	FGorgeousReplicatedPropertyConfig();

	/** Optional per-property replication condition mirroring legacy lifetime conditions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	TEnumAsByte<ELifetimeCondition> ReplicationCondition;

	/** When set, the provided function name will be executed as a RepNotify when the property updates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	FName RepNotifyFunction;

	/** Determines whether the RepNotify should fire only when a change is detected or on every network update. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	EGorgeousRepNotifyPolicy RepNotifyPolicy;

	/** Allows the RepNotify to trigger once when the initial replicated state arrives on a client. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bFireInitialNotify;
};

inline FGorgeousReplicatedPropertyConfig::FGorgeousReplicatedPropertyConfig()
	: ReplicationCondition(COND_None)
	, RepNotifyFunction(NAME_None)
	, RepNotifyPolicy(EGorgeousRepNotifyPolicy::OnChanged)
	, bFireInitialNotify(true)
{
}

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationRPCResultDescriptor
{
	GENERATED_BODY()

public:
	FGorgeousAutoReplicationRPCResultDescriptor()
		: ResultClass(nullptr)
		, bReplicateToAllConnections(true)
	{
	}

	/** Request guid forwarded by the originating RPC payload. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking")
	FGuid RequestGuid;

	/** Identifier that resolves the spawned result variable across the network. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking")
	FGuid ResultIdentifier;

	/** Handler name that produced this descriptor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking")
	FName HandlerName;

	/** Concrete result variable class (helpful for validation/debugging). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking")
	TSubclassOf<UGorgeousObjectVariable> ResultClass;

	/** True when the descriptor should replicate to every connection, false for invoker-only delivery. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking")
	bool bReplicateToAllConnections;

	/** Snapshot payload that reconstructs the full RPC result hierarchy on remote peers. */
	UPROPERTY()
	TArray<uint8> SnapshotPayload;
};

/**
 * Base class for defining variables as objects within the Gorgeous Things ecosystem.
 *
 * This class provides a flexible and extensible way to represent variables as UObjects,
 * allowing for dynamic data storage and manipulation. Child classes can leverage this
 * foundation to create specialized variable types with network replication capabilities.
 *
 * Key features include:
 * - Support for single, array, map, and set variable types.
 * - Integration with interfaces for getter and setter operations.
 * - Dynamic property management using templates.
 * - Registry for tracking object variables.
 * - Persistence options for level transitions.
 * - Unique identifier generation for each object variable.
 *
 * @note This class serves as the cornerstone for managing variables in the Gorgeous Things system,
 * enabling a more object-oriented approach to variable handling.
 *
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, ClassGroup = "Gorgeous Core|Gorgeous Object Variables", DisplayName = "Gorgeous Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables",
	meta = (ToolTip = "Used for providing a more interactive way to define variables in object form.", ShortTooltip = "The base class for all object variables."))
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariable : public UGorgeousBaseWorldContextUObject,
public IGorgeousObjectVariableInteraction_I,
public IGorgeousSingleObjectVariablesGetter_I, public IGorgeousSingleObjectVariablesSetter_I,
public IGorgeousArrayObjectVariablesGetter_I, public IGorgeousArrayObjectVariablesSetter_I,
public IGorgeousMapObjectVariablesGetter_I, public IGorgeousMapObjectVariablesSetter_I,
public IGorgeousSetObjectVariablesGetter_I, public IGorgeousSetObjectVariablesSetter_I
{
	GENERATED_BODY()

	//<================--- Friend Classes ---================>
	friend struct FGorgeousAutoReplicationHandle;
	friend class FGorgeousAutoReplicationCoordinator;
	friend class FGorgeousAutoReplicationMixin;
	friend class UGorgeousRootNetworkStackSubsystem;
#if WITH_AUTOMATION_TESTS
	friend struct FGorgeousObjectVariablePerfTestAccess;
#endif
	//<------------------------------------------------------>

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGorgeousAutoReplicationRPCPayloadEvent, const FGorgeousQueuedRPC&, QueuedRPC, UGorgeousObjectVariable*, TargetVariable);


protected:
	
	/** The Class Constructor for the Base Object Variable is used to set Default Values. */
	UGorgeousObjectVariable();

	virtual ~UGorgeousObjectVariable() override;

	// Sets up this object variable to be supported for networking.
	virtual bool IsSupportedForNetworking() const override { return bSupportsNetworking; }

public:

	/**
	 * Constructs a new object variable and registers it within the given registry depending on the parent given.
	 *
	 * @param Class The class that the object variable should derive from.
	 * @param Identifier The unique identifier of the object variable.
	 * @param Parent The parent of this object variable. The chain can be followed up to the root object variable.
	 * @param bShouldPersist Weather this object variable should be persistent across level switches.
	 * @return A new variable in object format.
	 *
	 * //@TODO: UGorgeousEvent is appearing here as it is also a object variable, we need to filter it out as the construction is handled differently
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (DeterminesOutputType = "Class"))
	UGorgeousObjectVariable* NewObjectVariable(TSubclassOf<UGorgeousObjectVariable> Class, FGuid& Identifier, UGorgeousObjectVariable* Parent = nullptr, bool bShouldPersist = false);

	/**
	 * Instantiates a new object variable of the specified class as transactional and registers it as a child of the given Parent for persistence across editor sessions.
	 *
	 * If no parent is specified, the root object variable will be used as the default parent.
	 * The created object is marked as transactional and assigned a unique identifier.
	 *
	 * @param Class The class type to instantiate. Must be a subclass of UGorgeousObjectVariable.
	 * @param Parent The optional parent object variable. If null, the root object variable is used instead.
	 * @param Outer The outer that should be used to store this transactional object variable.
	 * @return A pointer to the newly instantiated UGorgeousObjectVariable, or nullptr if instantiation failed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (DeterminesOutputType = "Class"))
	UGorgeousObjectVariable* InstantiateTransactionalObjectVariable(TSubclassOf<UGorgeousObjectVariable> Class, UGorgeousObjectVariable* Parent = nullptr, UObject* Outer = nullptr);

	/**
	 * Invokes the instanced functionality for when the ObjectVariable is contained inside a UPROPERTY with the Instanced meta specifier.
	 * 
	 * @param NewUniqueIdentifier The new unique identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
	virtual void InvokeInstancedFunctionality(FGuid NewUniqueIdentifier);
	
	/** Applies an authoritative identifier (from replication or tooling) and registers the variable if needed. */
	void ApplyReplicatedIdentifier(const FGuid& InIdentifier);

	virtual void BeginDestroy() override;

    /**
     * Registers the object variable with the registry.
     *
     * @param NewObjectVariable The object variable to register.
     */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
    virtual void RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable);

	/**
	 * Sets the new parent oft this object variable.
	 * 
	 * @param NewParent The new parent of the object variable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
	void SetParent(UGorgeousObjectVariable* NewParent);


	/** Returns the parent in the hierarchy, or null for root. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
	UGorgeousObjectVariable* GetParent() const { return Parent; }

	/** Enters the replication activation scope, allowing Blueprints to register properties & RPC bindings. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void ActivateReplication(const FGorgeousAutoReplicationContext& Context);

	/** Declares a property for replication while inside the activation scope. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void RegisterReplicatedProperty(FName PropertyName, EGorgeousReplicationMode Mode = EGorgeousReplicationMode::EProperty, bool bSendInitialState = true, const FGorgeousReplicatedPropertyConfig& AdvancedConfig = FGorgeousReplicatedPropertyConfig());

	/** Binds a named RPC handler that can react to async AutoReplication networking events. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void BindRPCHandler(FName RPCName, EGorgeousAutoReplicationRPCType Reliability);

	/** Queues an AutoReplication RPC using the stored binding (or the provided override data). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool RequestAutoReplicationRPC(EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, FName OverrideKey = NAME_None, UObject* OverrideContext = nullptr, EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EObjectVariable);

	/** Blueprint async helper that mirrors the mixin RPC request node using this variable's binding. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	UGorgeousAutoReplicationRPCRequestAsyncAction* RequestAutoReplicationRPCAsync(EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, FName OverrideKey = NAME_None, UObject* OverrideContext = nullptr, EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EObjectVariable);

	/** Serializes the registered properties into a custom payload when the native path is bypassed. */
	bool BuildAutoReplicationPropertyPayload(const FGorgeousAutoReplicationConditionContext& ConditionContext, FGorgeousAutoReplicationPropertyPayload& OutPayload) const;

	/** Applies a previously serialized property payload on the receiving endpoint. */
	bool ApplyAutoReplicationPropertyPayload(const FGorgeousAutoReplicationPropertyPayload& Payload, UPackageMap* PackageMap = nullptr);

	/** Override to serialize custom payloads when EGorgeousReplicationMode::CustomPayload is selected. Return true to include the payload in the batch. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|Auto Replication", DisplayName = "Build Custom Auto Replication Payload")
	bool BuildCustomAutoReplicationPayload(FName PropertyName, UPARAM(ref) TArray<uint8>& OutPayload, bool bIsInitialState);
	virtual bool BuildCustomAutoReplicationPayload_Implementation(FName PropertyName, TArray<uint8>& OutPayload, bool bIsInitialState);

	/** Override to consume custom payloads on clients. Return true when the payload was applied successfully. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|Auto Replication", DisplayName = "Apply Custom Auto Replication Payload")
	bool ApplyCustomAutoReplicationPayload(FName PropertyName, const TArray<uint8>& Payload, bool bIsInitialState);
	virtual bool ApplyCustomAutoReplicationPayload_Implementation(FName PropertyName, const TArray<uint8>& Payload, bool bIsInitialState);

	/** Returns true when this object variable is attached to an AutoReplication entry. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool HasAutoReplicationBinding() const { return AutoReplicationEntryKey != NAME_None && AutoReplicationOwner.IsValid(); }

	/** Provides the owning AutoReplication object for debugging/advanced routing. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	UObject* GetAutoReplicationOwner() const { return AutoReplicationOwner.Get(); }

	/** Returns the bound AutoReplication entry key, if any. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	FName GetAutoReplicationEntryKey() const { return AutoReplicationEntryKey; }

	/** Returns the replication index assigned by the owning mixin, or INDEX_NONE. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	int32 GetAutoReplicationReplicationIndex() const { return AutoReplicationReplicationIndex; }

	/** Executes a previously queued AutoReplication RPC that targets this object variable instance. */
	bool ExecuteAutoReplicationRPC(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable = nullptr);

	/** Invokes a AutoReplication RPC handler on any UObject using the payload's named arguments. */
	static bool InvokeNativeAutoReplicationRPCHandlerOnObject(UObject* Target, const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable = nullptr);

	/** Creates a standalone UGorgeousRPC_OV result container when no object variable context is available. */
	static UGorgeousRPC_OV* CreateStandaloneRPCResultContainer(UObject* OwningContext);

	/** Attempts to resolve the replicated RPC result container associated with the supplied RequestGuid. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool TryGetReplicatedRPCResult(const FGuid& RequestGuid, UGorgeousObjectVariable*& OutResult);

	/** Removes the cached RPC result descriptor locally and, when authoritative, clears it from replication. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void ClearReplicatedRPCResult(const FGuid& RequestGuid);

	/** Enables or disables replication for this variable instance at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void SetNetworkingEnabled(bool bShouldReplicate);

	/** Returns true if this instance should leverage the AutoReplication backend. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool SupportsAutoReplicationFeatures() const { return ReplicationMode != EGorgeousObjectVariableReplicationMode::EManual; }

	/** Returns the currently resolved AutoReplication config (after any runtime overrides). */
	const FGorgeousAutoReplicationStreamConfig& GetAutoReplicationConfig() const { return AutoReplicationConfig; }

	/** Returns true if this instance should use the legacy/native replication path. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool SupportsLegacyReplication() const { return ReplicationMode != EGorgeousObjectVariableReplicationMode::EFullAutoReplication; }

	/** Blueprint hook used to opt-in properties/RPC handlers right after the mixin activates replication. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void OnReplicationActivated(const FGorgeousAutoReplicationContext& Context);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreNetReceive() override;
	virtual void PostNetReceive() override;
	virtual bool IsNameStableForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	/** Blueprint extension point fired once a AutoReplication RPC payload is ready to be consumed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void HandleAutoReplicationRPCPayload(const FGorgeousQueuedRPC& QueuedRPC);
	virtual void HandleAutoReplicationRPCPayload_Implementation(const FGorgeousQueuedRPC& QueuedRPC);

	/** Allows Blueprints to override controller-based access checks for the root network stack. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool CanControllerAccessVariable(AGorgeousPlayerController* Controller) const;
	virtual bool CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller) const;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	AGorgeousPlayerController* GetOwningGorgeousPlayerController() const { return ResolveOwningPlayerController(); }

	/**
	 * Optional override that is evaluated when AutoReplicationConfig.bRespectAccessPolicy is true
	 * while the resolved policy is Everyone. Allows blueprints to provide an alternate policy that
	 * participates in the strict access pipeline without changing the serialized config.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|Gorgeous Object Variables|Networking", meta = (DisplayName = "Resolve Respect Access Policy"))
	EGorgeousObjectVariableAccessPolicy ResolveRespectAccessPolicy(UGorgeousObjectVariable* Variable) const;
	virtual EGorgeousObjectVariableAccessPolicy ResolveRespectAccessPolicy_Implementation(UGorgeousObjectVariable* Variable) const;

	/** Simple example handler that can be referenced by AutoReplication RPC payloads for testing. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void ExampleAutoReplicationRPCHandler(double ExampleValue, const FString& ExampleLabel);

	/** Assigns a friendly display name to this object variable (auto-generates one when empty). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
	void SetDisplayName(const FString& InDisplayName);

	UFUNCTION()
	void OnRep_DisplayName(const FString& PreviousDisplayName);

	UFUNCTION()
	void OnRep_BroadcastRPCResultDescriptors();

	UFUNCTION()
	void OnRep_OwnerScopedRPCResultDescriptors();

	/** Returns the configured display name or the UObject's fallback name when unset. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
	const FString& GetDisplayName() const { return DisplayName; }

	/** Helper that exposes the runtime replication flag. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool IsReplicationActive() const { return bSupportsNetworking && bReplicates; }

	/** Helper that exposes whether legacy subobject replication is currently registered. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool IsLegacyReplicationRegistered() const { return bLegacyReplicationRegistered; }

	/** Helper to toggle the root network stack at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void SetRootNetworkStackEnabled(bool bEnable);

	/** Helper to toggle the shared-stack flag while keeping configs in sync. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void SetSharedNetworkStackEnabled(bool bEnable);

	/** Internal helpers that allow callers to opt-out from immediate coordinator notifications. */
	void SetRootNetworkStackEnabledInternal(bool bEnable, bool bNotifyCoordinator);
	void SetSharedNetworkStackEnabledInternal(bool bEnable, bool bNotifyCoordinator);

	/** Returns true whenever the shared stack path is active alongside networking support. */
	bool IsSharedNetworkStackActive() const { return bSupportsNetworking && bUseSharedNetworkStack; }

	/** Resolves the currently active access policy (root, shared, or default). */
	EGorgeousObjectVariableAccessPolicy GetEffectiveAccessPolicy() const;

	/** Blueprint helper that mirrors AutoReplicationConfig.bRespectAccessPolicy. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void SetAutoReplicationRespectAccessPolicy(bool bEnable);

	/** Blueprint helper that updates the active network access policy at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void SetNetworkAccessPolicy(EGorgeousObjectVariableAccessPolicy NewPolicy, bool bApplyToSharedStack = false);

	/** Ensures this variable (and its shared/root stack path) keeps a valid owner for networking purposes. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void EnsureSharedNetworkStackOwner(UObject* FallbackOwner);

	/** Resolves the replication channel used by the active networking stack. */
	FName GetEffectiveNetworkChannel() const;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
	FString GetDisplayNameOrFallback() const;

	/** Multicast dispatcher triggered for every executed AutoReplication RPC payload. */
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Object Variable|Networking")
	FGorgeousAutoReplicationRPCPayloadEvent OnAutoReplicationRPCPayload;

	/** Determines which networking stack(s) this object variable should use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking"
		, meta = (EditCondition = "bSupportsNetworking", EditConditionHides, DisplayAfter = "bSupportsNetworking"))
	EGorgeousObjectVariableReplicationMode ReplicationMode;
	
	/** Per-instance tuning for the auto-replication backend. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking"
		, meta = (EditCondition = "bSupportsNetworking", ShowOnlyInnerProperties, DisplayAfter = "ReplicationMode"))
	FGorgeousAutoReplicationStreamConfig AutoReplicationConfig;

	/** Optional root network stack configuration (only visible when networking is available and the shared stack is disabled). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking"
		, meta = (EditCondition = "bSupportsNetworking && !bUseSharedNetworkStack", EditConditionHides, ShowOnlyInnerProperties, DisplayAfter = "AutoReplicationConfig"))
	FGorgeousRootNetworkAccessConfig RootNetworkConfig;

	/** Enables the shared network stack path when root access is disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking"
		, meta = (EditCondition = "bSupportsNetworking", EditConditionHides, DisplayAfter = "RootNetworkConfig"))
	bool bUseSharedNetworkStack;

	/** Shared network stack configuration surfaced when the shared stack is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking"
		, meta = (EditCondition = "bUseSharedNetworkStack", EditConditionHides, ShowOnlyInnerProperties, DisplayAfter = "bUseSharedNetworkStack"))
	FGorgeousSharedNetworkStackConfig SharedNetworkStackConfig;


private:
	bool InvokeNativeAutoReplicationRPCHandler(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable = nullptr);

protected:
	/**
	 * Sets a dynamic property of the object variable.
	 *
	 * @tparam InTCppType The C++ type of the property.
	 * @tparam TInPropertyBaseClass The base class of the property.
	 * @param PropertyName The name of the property.
	 * @param Value The value to set.
	 */
//grepper cpp unreal wildcard
	//Gorgeous-Core on [GitHub](https://github.com/Epic-Nova/Gorgeous-Core)
    template<typename InTCppType, typename TInPropertyBaseClass>
    void SetDynamicProperty(const FName PropertyName, const InTCppType& Value)
    {
    	if (FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName); PropertyName.IsValid())
    	{
    		// Handle UObject* properties separately
    		if constexpr (std::is_base_of_v<UObject, InTCppType>)
    		{
    			if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
    			{
    				ObjectProperty->SetObjectPropertyValue_InContainer(this, Value);
    			}
    		}
    		else if (Property && Property->IsA<TProperty<InTCppType, TInPropertyBaseClass>>())
    		{
    			if (TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastField<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
    			{
    				TypedProperty->SetPropertyValue_InContainer(this, Value);
    			}
    		}
    	}
    }
//end grepper
	
	/**
	 * Gets a dynamic property of the object variable.
	 *
	 * @tparam InTCppType The C++ type of the property.
	 * @tparam TInPropertyBaseClass The base class of the property.
	 * @param PropertyName The name of the property.
	 * @param OutValue The output value of the property.
	 * @return True if the property was successfully retrieved, false otherwise.
	 */
//grepper cpp unreal wildcard
	//Gorgeous-Core on [GitHub](https://github.com/Epic-Nova/Gorgeous-Core)
    template<typename InTCppType, typename TInPropertyBaseClass>
    bool GetDynamicProperty(const FName PropertyName, InTCppType& OutValue) const
    {
    	if (FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName); Property && PropertyName.IsValid())
    	{
    		// Handle UObject* properties separately
    		if constexpr (std::is_base_of_v<UObject, InTCppType>)
    		{
    			if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
    			{
    				OutValue = Cast<InTCppType>(ObjectProperty->GetObjectPropertyValue_InContainer(this));
    				return true;
    			}
    		}
    		else if (Property && Property->IsA<TProperty<InTCppType, TInPropertyBaseClass>>())
    		{
    			if (const TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastField<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
    			{
    				OutValue = TypedProperty->GetPropertyValue_InContainer(this);
    				return true;
    			}
    		}
    	}
    	return false;
    }
//end grepper

#if WITH_EDITOR

	/**
	 * Returns the in the constructor configured settings for the getter and setter nodes.
	 * 
	 * @return The pin configuration of this object variable.
	 */
	FObjectVariablePinConfiguration_S GetObjectVariablePinConfiguration() const { return PinConfiguration; }
	
#endif WITH_EDITOR
	
public:
	/**
	 * The unique identifier of the object variable.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	FGuid UniqueIdentifier;

	/**
	 * The registry of object variables.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	TArray<TObjectPtr<UGorgeousObjectVariable>> VariableRegistry;

	/**
	 * Whether the object variable is persistent across level switches.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	bool bPersistent;

	/** Whether this variable supports networking features at all. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking")
	bool bSupportsNetworking;

	/** Per-instance binding that opts the variable into a specific root registry. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Object Variable|Root Setup", meta = (ShowOnlyInnerProperties))
	FGorgeousObjectVariableRootConfiguration RootConfiguration;

	/** Optional friendly name surfaced in tooling and console commands. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_DisplayName, Category = "Gorgeous Object Variable")
	FString DisplayName;

	/** Returns the world associated with this variable for callers that cannot access the protected GetWorld override. */
	UWorld* GetVariableWorld() const { return GetWorld(); } //@TODO: Some classes are friend with this class just to access GetWorld, replace these fn calls with this call

	/** Returns true when the variable is configured to route through the root network stack. */
	bool IsRootNetworkStackEnabled() const { return ShouldUseRootNetworkStack(); }

	/** Runtime replicated flag toggled by the owning AutoReplication system. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking", meta = (EditCondition = "bSupportsNetworking", EditConditionHides))
	bool bReplicates;

protected:

	/**
	 * The parent of the object variable.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	UGorgeousObjectVariable* Parent;

#if WITH_EDITORONLY_DATA
	
	/**
	 * Configuration values that provides the editor with information how to handle and display various object variables.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable", meta = (AllowPrivateAccess = true))
	FObjectVariablePinConfiguration_S PinConfiguration;

	
	/**
	 * The Ptr to the transactional default value object of this object variable
	 */
	UPROPERTY()
	TWeakObjectPtr<UGorgeousObjectVariable> DefaultValuePtr;
	
#endif WITH_EDITORONLY_DATA

	struct FReplicatedPropertyDeclaration
	{
		FReplicatedPropertyDeclaration();
		~FReplicatedPropertyDeclaration();

		bool HasRepNotify() const { return !RepNotifyFunction.IsNone(); }
		void InitializeShadowState(const FProperty* Property, const void* InitialData);
		void ResetShadowState();

		FName PropertyName;
		FProperty* CachedProperty;
		EGorgeousReplicationMode Mode;
		bool bSendInitialState;
		TEnumAsByte<ELifetimeCondition> ReplicationCondition;
		FName RepNotifyFunction;
		EGorgeousRepNotifyPolicy RepNotifyPolicy;
		bool bFireInitialNotify;
		bool bDeliveredInitialNotify;
		bool bShadowInitialized;
		bool bHasValidatedRepNotifySignature;
		bool bIsRepNotifySignatureValid;
		TArray<uint8> RepNotifyShadow;
	};

	struct FRPCBindingDeclaration
	{
		FName RPCName;
		EGorgeousAutoReplicationRPCType Reliability;
	};

	bool EnsureReplicationActivation(const TCHAR* SourceFunction) const;

	bool bReplicationActivationGuard;
	FGorgeousAutoReplicationContext ActiveReplicationContext;
	TArray<FReplicatedPropertyDeclaration> RegisteredReplicatedProperties;
	TArray<FRPCBindingDeclaration> RegisteredRPCBindings;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking", meta = (AllowPrivateAccess = "true"))
	TArray<FGorgeousAutoReplicationRPCResultDescriptor> BroadcastRPCResultDescriptors;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable|Networking", meta = (AllowPrivateAccess = "true"))
	TArray<FGorgeousAutoReplicationRPCResultDescriptor> OwnerScopedRPCResultDescriptors;
	
private:
	bool ResolveAutoReplicationRPCContext(FName OverrideKey, UObject* OverrideContext, FName& OutKey, UObject*& OutContext) const;
	FReplicatedPropertyDeclaration* FindReplicatedDeclarationByName(FName PropertyName);
	const FReplicatedPropertyDeclaration* FindReplicatedDeclarationByName(FName PropertyName) const;
	void EvaluateRegisteredRepNotifies(bool bForceAllNotifies = false);
	void InvokeRepNotify(FReplicatedPropertyDeclaration& Declaration, const uint8* OldValueData);
	void NotifyDisplayNameChanged();
	void SetAutoReplicationBinding(UObject* InOwner, FName InEntryKey, int32 InReplicationIndex, const FGorgeousAutoReplicationStreamConfig* StreamOverride, bool bWantsReplication);
	void RegisterLegacyReplication(UObject* BindingContext);
	void UnregisterLegacyReplication();
	AActor* ResolveReplicationOwnerActor(UObject* BindingContext = nullptr) const;
	void SetIsReplicated(bool InIsReplicated);
	void UpdateAutoReplicationState(bool bEnable);
	FName GetConfiguredRootName() const;
	bool ShouldUseRootNetworkStack() const;
	bool DoesRootEnforceNetworking() const;
	bool DoesConfiguredRootSupportNetworking() const;
	bool EvaluateAccessPolicyForController(AController* Controller) const;
	AGorgeousPlayerController* ResolveOwningPlayerController() const;
	UGorgeousRPC_OV* CreateAutoRPCResultContainer();
	void RegisterReplicatedRPCResult(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable* ResultContainer);
	void HandleReplicatedRPCResultDescriptors(const TArray<FGorgeousAutoReplicationRPCResultDescriptor>& Descriptors);
	void TrimRPCResultArray(TArray<FGorgeousAutoReplicationRPCResultDescriptor>& InOutArray);
	void MarkRPCResultDescriptorsDirty();
	static UGorgeousObjectVariable* GetOrCreateRPCResultParent();
	bool BuildRPCResultSnapshot(UGorgeousObjectVariable* ResultContainer, TArray<uint8>& OutSnapshot) const;
	bool SerializeRPCSnapshotRecursive(UGorgeousObjectVariable* Variable, FArchive& Ar) const;
	UGorgeousObjectVariable* InstantiateRPCResultFromDescriptor(const FGorgeousAutoReplicationRPCResultDescriptor& Descriptor);
	UGorgeousObjectVariable* DeserializeRPCSnapshotRecursive(UGorgeousObjectVariable* InParent, FArchive& Ar);
	
private:
	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UGorgeousObjectVariable*, ObjectVariable, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UObject*, ObjectObject, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UClass*, ObjectClass, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(bool, Boolean, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(uint8, Byte, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(float, Float, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(double, Double, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(int64, Integer64, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(int32, Integer, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FName, Name, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FRotator, Rotator, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FString, String, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FText, Text, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FTransform, Transform, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FVector, Vector, Single)

	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UGorgeousObjectVariable*>, ObjectVariable, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UObject*>, ObjectObject, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UClass*>, ObjectClass, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<TSoftObjectPtr<UObject>>, SoftObjectObject, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<TSoftClassPtr<UObject>>, SoftObjectClass, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<bool>, Boolean, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<uint8>, Byte, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<float>, Float, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<double>, Double, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<int64>, Integer64, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<int32>, Integer, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FName>, Name, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FRotator>, Rotator, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FString>, String, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FText>, Text, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FTransform>, Transform, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FVector>, Vector, Array)

	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UGorgeousObjectVariable*, ObjectVariable)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UObject*, ObjectObject)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UClass*, ObjectClass)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(uint8, Byte)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(float, Float)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(double, Double)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(int64, Integer64)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(int32, Integer)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FName, Name)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FString, String)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FTransform, Transform)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FVector, Vector)

	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UGorgeousObjectVariable*>, ObjectVariable, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UObject*>, ObjectObject, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UClass*>, ObjectClass, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<TSoftObjectPtr<UObject>>, SoftObjectObject, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<TSoftClassPtr<UObject>>, SoftObjectClass, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<uint8>, Byte, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<float>, Float, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<double>, Double, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<int64>, Integer64, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<int32>, Integer, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FName>, Name, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FString>, String, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FTransform>, Transform, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FVector>, Vector, Set)


	UPROPERTY(Transient)
	TWeakObjectPtr<UObject> AutoReplicationOwner;

	UPROPERTY(Transient)
	FName AutoReplicationEntryKey;

	UPROPERTY(Transient)
	int32 AutoReplicationReplicationIndex;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> LegacyReplicationOwner;

	UPROPERTY(Transient)
	uint8 bLegacyReplicationRegistered : 1;

	UPROPERTY(Transient)
	TMap<FGuid, FGorgeousAutoReplicationRPCResultDescriptor> CachedRPCResultDescriptors;

	UPROPERTY(Transient)
	TMap<FGuid, TWeakObjectPtr<UGorgeousObjectVariable>> CachedRPCResultInstances;

	/**
	 * Gets the unique identifier for an object variable.
	 *
	 * @return The unique identifier as an FGuid.
	 */
	virtual FGuid GetUniqueIdentifierForObjectVariable_Implementation() override;
};

#if WITH_AUTOMATION_TESTS
	struct FGorgeousObjectVariablePerfTestAccess
	{
		static AActor* ResolveReplicationOwner(const UGorgeousObjectVariable* Variable)
		{
			return Variable ? Variable->ResolveReplicationOwnerActor() : nullptr;
		}

		static UWorld* ResolveWorld(const UGorgeousObjectVariable* Variable)
		{
			return Variable ? Variable->GetWorld() : nullptr;
		}

		static AGorgeousPlayerController* ResolveOwningPlayerController(const UGorgeousObjectVariable* Variable)
		{
			return Variable ? Variable->ResolveOwningPlayerController() : nullptr;
		}
	};
#endif
