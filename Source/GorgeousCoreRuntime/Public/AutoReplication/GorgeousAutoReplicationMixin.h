// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"

#include "GorgeousAutoReplicationMixin.generated.h"

class UGorgeousObjectVariable;
class UGorgeousAutoReplicationRPCTransporter;
class AController;

/**
 * Handle that maps a AutoReplication key to a replicated array index and optional cached value.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationHandle
{
	GENERATED_BODY()

public:
	static constexpr uint16 InvalidReplicationIndex = MAX_uint16;

	FGorgeousAutoReplicationHandle();

	/** Returns true if the handle references either a replication slot or a cached value. */
	bool IsValid() const { return ReplicationIndex != InvalidReplicationIndex || CachedValue != nullptr; }

	/** Resets the handle to an invalid state. */
	void Reset();

	/** Assigns the handle to a specific key / replication index pair. */
	void Assign(const FName InKey, const uint16 InReplicationIndex, UObject* InOwner);

	/** Updates the cached value for quick Blueprint access. */
	void CacheValue(UGorgeousObjectVariable* InValue, const FGorgeousAutoReplicationStreamConfig* StreamOverride = nullptr, bool bAutoEnableNetworking = false);

	uint16 GetReplicationIndex() const { return ReplicationIndex; }
	FName GetKey() const { return RegisteredKey; }
	UGorgeousObjectVariable* GetCachedValue() const { return CachedValue; }

private:
	UPROPERTY()
	uint16 ReplicationIndex;

	UPROPERTY()
	FName RegisteredKey;

	UPROPERTY(Transient)
	TObjectPtr<UGorgeousObjectVariable> CachedValue;

	TWeakObjectPtr<UObject> OwningObject;
};

/**
 * Entry stored inside AdditionalGorgeousData that keeps the default object and replication metadata.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousObjectVariableEntry
{
	GENERATED_BODY()

public:
	FGorgeousObjectVariableEntry();

	// Default value that will be instanced or used when networking is disabled.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced, Category = "Gorgeous AutoReplication", meta = (GorgeousObjectVariableTrunk = "DefaultObjectVariableTrunk"))
	TObjectPtr<UGorgeousObjectVariable> DefaultValue;

	/** If true, this entry will register itself with the replicated array when networking is enabled. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gorgeous AutoReplication")
	uint8 bReplicate : 1;

	/** If true, StreamConfigOverride will be applied on top of the developer defaults for this entry. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gorgeous AutoReplication")
	uint8 bOverrideStreamConfig : 1;

	/** Optional per-entry stream configuration that selects Iris/Replication Graph behavior. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gorgeous AutoReplication", meta = (EditCondition = "bOverrideStreamConfig"))
	FGorgeousAutoReplicationStreamConfig StreamConfigOverride;

	/** Internal handle that keeps track of replicated slot + cached pointer. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Gorgeous AutoReplication")
	FGorgeousAutoReplicationHandle Handle;
};

/**
 * Replicated payload slot used by the mixin to keep authoritative object variables in sync.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousReplicatedVariableEntry
{
	GENERATED_BODY()

public:
	FGorgeousReplicatedVariableEntry();

	UPROPERTY()
	uint16 ReplicationIndex;

	UPROPERTY()
	FName Key;

	UPROPERTY(Instanced)
	TObjectPtr<UGorgeousObjectVariable> Value;

	/** Class used to instantiate the variable when the instance is not yet available. */
	UPROPERTY()
	TSubclassOf<UGorgeousObjectVariable> ValueClass;

	/** Stable identifier mirrored from the authoritative instance so remote lookups can resolve the same variable. */
	UPROPERTY()
	FGuid VariableIdentifier;

	UPROPERTY()
	uint8 bIsActive : 1;
};

/**
 * Lightweight mixin dropped into each AutoReplication class to consolidate data registration, replication, and async hooks.
 *
 * @TODO NOTE (future plan): Replication currently requires variables to be registered inside the
 * AdditionalGorgeousData map of a QoL class (e.g. GorgeousPlayerController, GorgeousPlayerState).
 * A replication slot is only allocated when InitializeAdditionalData() iterates entries with
 * bReplicate=true.  We plan to extend this so that replication is also available for object
 * variables that are NOT registered in AdditionalGorgeousData — e.g. standalone variables
 * created at runtime or owned by non-QoL actors — by allowing direct slot allocation through
 * the mixin without requiring an AdditionalGorgeousData entry.
 *
 * @TODO NOTE (future plan): We also plan to support runtime registration of individual
 * replicated variables into the slot array without having to recall InitializeAdditionalData().
 * Currently, adding a new entry at runtime forces a full re-initialization pass which resets
 * and rebuilds the entire ReplicatedVariables array.  A dedicated method (e.g.
 * RegisterReplicatedSlot(FName Key, UGorgeousObjectVariable* Value)) would allocate a single
 * slot on demand, making hot-path runtime registration cheaper and avoiding side-effects on
 * already-active slots.
 */
class GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationMixin
{
public:
	friend class UGorgeousAutoReplicationRPCTransporter;
	friend class UGorgeousAutoReplicationRPCRelayComponent;

	FGorgeousAutoReplicationMixin();

	/** Binds the mixin to the owning object and the storage containers declared by macros on that class. */
	void Bind(UObject* InOwner,
		TMap<FName, FGorgeousObjectVariableEntry>* InAdditionalData,
		TArray<FGorgeousReplicatedVariableEntry>* InReplicatedVariables);

	/** Initializes the additional data container and optionally activates networking. */
	void InitializeAdditionalData(bool bActivateNetworkingCapabilities);

	/** Returns true if networking is currently active for this AutoReplication instance. */
	bool IsNetworkingEnabled() const { return bNetworkingEnabled; }

	/** Finds an entry by key. */
	FGorgeousObjectVariableEntry* FindEntry(const FName Key);
	const FGorgeousObjectVariableEntry* FindEntry(const FName Key) const;

	/** Sets a replicated value, returning false if the key cannot be found or networking is disabled. */
	bool TrySetReplicatedValue(const FName Key, UGorgeousObjectVariable* NewValue);

	/** Resolves a value for the provided key, preferring replicated data when available. */
	bool TryGetValue(const FName Key, UGorgeousObjectVariable*& OutValue) const;

	/** Maps a key onto a deterministic replication index, allocating one if needed. */
	uint16 GetOrAssignReplicationIndex(const FName Key);

	/** Called from replicated OnRep to rebuild caches after a payload update. */
	void RefreshCachedValues();

	/** Queues an asynchronous RPC request routed through the owning AutoReplication class. */
	bool RequestRPC(const FName Key, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EAuto, FGuid* OutRequestGuid = nullptr);

	/** Executes a queued RPC immediately against the resolved object variable entry. */
	bool ExecuteAutoReplicationRPC(const FGorgeousQueuedRPC& QueuedRPC);

	/** Pops the next queued RPC so Blueprints can process it manually. */
	bool DequeuePendingRPC(FGorgeousQueuedRPC& OutRPC);

	/** Drains the pending queue and forwards entries to the owning AutoReplication responder if available. */
	void DispatchPendingRPCs();

	/** Returns true if there are pending RPC entries awaiting Blueprint processing. */
	bool HasPendingRPCs() const { return PendingRPCs.Num() > 0; }

	/** Number of replicated entries currently tracked by the mixin. */
	int32 GetReplicatedEntryCount() const;

	/** Sets the relay component to use for client-to-server RPC routing. */
	void SetRPCRelayComponent(UGorgeousAutoReplicationRPCRelayComponent* InRelay) { RPCRelayComponent = InRelay; }

private:
	void EnsureBound() const;
	UObject* ResolveOwnerObject() const;
	bool IsAuthorityContext() const;
	FGorgeousReplicatedVariableEntry* FindReplicatedEntryByKey(const FName Key) const;
	void InitializeTransporter();
	bool EnqueueRPCInternal(const FGorgeousQueuedRPC& QueuedRPC);
	bool HandleTransportedRPC(const FGorgeousQueuedRPC& QueuedRPC);
	void HandleTransportedPropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope);
	bool ResolveRPCDestination(const FName Key, EGorgeousAutoReplicationTargetKind RequestedKind, EGorgeousAutoReplicationTargetKind& OutResolvedKind, UGorgeousObjectVariable*& OutVariable, UObject*& OutOwner) const;
	UGorgeousObjectVariable* ResolveVariableForKey(const FName Key) const;
	bool CanControllerReceiveRPC(const FGorgeousQueuedRPC& QueuedRPC, AController* Controller) const;
	bool CanControllerReceivePropertyPayload(FName EntryKey, AController* Controller) const;
	void AutoSubscribeControllerForVariable(AController* Controller, UGorgeousObjectVariable* TargetVariable) const;

private:
	TWeakObjectPtr<UObject> Owner;
	TMap<FName, FGorgeousObjectVariableEntry>* AdditionalData;
	TArray<FGorgeousReplicatedVariableEntry>* ReplicatedVariables;
	TMap<FName, uint16> KeyToReplicationIndex;
	bool bNetworkingEnabled;
	bool bIsBound;
	/** Actor-bound component that mirrors RPC payloads to the desired network target. */
	TWeakObjectPtr<UGorgeousAutoReplicationRPCTransporter> RPCTransporter;
	/** Relay component for routing server-bound RPCs from clients. */
	TWeakObjectPtr<UGorgeousAutoReplicationRPCRelayComponent> RPCRelayComponent;

	TArray<FGorgeousQueuedRPC> PendingRPCs;

	/** One-shot timer used to introduce a brief dispatch delay (see EnqueueRPCInternal). */
	FTimerHandle PendingDispatchTimerHandle;
};

