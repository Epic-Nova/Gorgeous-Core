// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "Net/UnrealNetwork.h"
#include "GorgeousAutoReplicationTypes.generated.h"

class UGorgeousObjectVariable;
class UPackageMap;
class AGorgeousPlayerController;
UENUM(BlueprintType)
enum class EGorgeousAutoReplicationBackend : uint8
{
	Native UMETA(DisplayName = "UE Replication"),
	Iris UMETA(DisplayName = "Iris"),
	ReplicationGraph UMETA(DisplayName = "Replication Graph"),
};

/** Tunable settings for a single object-variable data stream. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationStreamConfig
{
	GENERATED_BODY()

	FGorgeousAutoReplicationStreamConfig()
		: Backend(EGorgeousAutoReplicationBackend::Native)
		, UpdateFrequency(30.f)
		, MinUpdateFrequency(10.f)
		, Priority(0)
		, bSupportsMulticast(true)
		, bRespectAccessPolicy(false)
		, BandwidthBudgetKB(4.f)
	{}

	/** Backend to use when replicating this object variable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	EGorgeousAutoReplicationBackend Backend;

	/** Desired updates per second when the backend supports rate control. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking", meta = (ClampMin = "1.0"))
	float UpdateFrequency;
	
	/** Lower bound applied when the scheduler attempts to throttle the stream. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking", meta = (ClampMin = "1.0"))
	float MinUpdateFrequency;
	
	/** Scheduler priority hint forwarded to backend integrations (higher means more important). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking", meta = (ClampMin = "-8", ClampMax = "8"))
	int32 Priority;

	/** Whether this stream should forward multicast events as well as owner-only state. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bSupportsMulticast;
	
	/**
	 * Mirrors the (shared) root network access stack under an Everyone policy.
	 * Non-Everyone policies always enforce the root stack, so this flag
	 * only matters when the policy allows all access. Enabling it keeps
	 * the variable in the strict relevancy pipeline even though its own
	 * policy doesn't filter, which is useful for runtime policy changes
	 * or when designers want this stream to participate in the root checks
	 * despite using an “allow all” policy, when enabled, Blueprint subclasses
	 * can override UGorgeousObjectVariable::ResolveRespectAccessPolicy to
	 * return an alternate policy for these Everyone streams.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bRespectAccessPolicy;

	/** Soft bandwidth budget for this stream when Iris throttling is available (kilobytes per second). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking", meta = (ClampMin = "0.5"))
	float BandwidthBudgetKB;
	
	float GetEffectiveUpdateFrequency() const { return FMath::Max(UpdateFrequency, MinUpdateFrequency); }
};

/** Simple description of an active data stream tracked by the coordinator. */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationActiveStream
{
	GENERATED_BODY()

	FGorgeousAutoReplicationActiveStream()
		: Target(nullptr)
		, Priority(0)
		, EffectiveUpdateFrequency(0.f)
		, bRespectsAccessPolicy(false)
		, bRegisteredWithReplicationGraph(false)
		, bRegisteredWithIris(false)
	{
	}

	FGorgeousAutoReplicationActiveStream(TObjectPtr<UGorgeousObjectVariable> InTarget, const FGorgeousAutoReplicationStreamConfig& InConfig);

	FGorgeousAutoReplicationActiveStream(UGorgeousObjectVariable* InTarget, const FGorgeousAutoReplicationStreamConfig& InConfig);

	bool IsRegisteredWithReplicationGraph() const { return bRegisteredWithReplicationGraph; }
	void SetRegisteredWithReplicationGraph(const bool bInRegistered) { bRegisteredWithReplicationGraph = bInRegistered; }
	bool IsRegisteredWithIris() const { return bRegisteredWithIris; }
	void SetRegisteredWithIris(const bool bInRegistered) { bRegisteredWithIris = bInRegistered; }
	FGuid GetStreamGuid() const { return StreamGuid; }
	void RefreshStreamGuid(UGorgeousObjectVariable* Variable);
	int32 GetPriority() const { return Priority; }
	float GetEffectiveUpdateFrequency() const { return EffectiveUpdateFrequency; }
	bool RespectsAccessPolicy() const { return bRespectsAccessPolicy; }

	UPROPERTY()
	TWeakObjectPtr<UGorgeousObjectVariable> Target;

	UPROPERTY()
	FGorgeousAutoReplicationStreamConfig Config;

	/** Optional logical channel used for root network stack routing. */
	UPROPERTY()
	FName RootNetworkChannel;

	UPROPERTY()
	int32 Priority;

	UPROPERTY()
	float EffectiveUpdateFrequency;

	UPROPERTY()
	bool bRespectsAccessPolicy;

private:
	UPROPERTY()
	bool bRegisteredWithReplicationGraph;

	UPROPERTY()
	bool bRegisteredWithIris;

	UPROPERTY()
	FGuid StreamGuid;
};

/** Utility helpers for reasoning about backend feature support. */
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationBackendTraits
{
	static bool UsesIris(EGorgeousAutoReplicationBackend Backend);
	static bool UsesReplicationGraph(EGorgeousAutoReplicationBackend Backend);
};

/** Serialized value for a single registered property when using the custom payload path. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationPropertyValue
{
	GENERATED_BODY()

	FGorgeousAutoReplicationPropertyValue()
		: Mode(EGorgeousReplicationMode::EProperty)
		, ReplicationCondition(COND_None)
		, bIsInitialState(false)
	{
	}

	/** Name of the property as registered through RegisterReplicatedProperty. */
	UPROPERTY()
	FName PropertyName;

	/** Serialization strategy requested by the registration. */
	UPROPERTY()
	EGorgeousReplicationMode Mode;

	/** Lifetime condition that mirrors native replication filters. */
	UPROPERTY()
	TEnumAsByte<ELifetimeCondition> ReplicationCondition;

	/** True if this blob represents the initial replicated state. */
	UPROPERTY()
	uint8 bIsInitialState : 1;

	/** Raw serialized payload for the property. */
	UPROPERTY()
	TArray<uint8> Payload;
};

/** Batched payload streamed over AutoReplication when bypassing the native UE path. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationPropertyPayload
{
	GENERATED_BODY()

	FGorgeousAutoReplicationPropertyPayload()
		: StreamGuid()
	{
	}

	bool IsEmpty() const { return Properties.Num() == 0; }
	void Reset() { StreamGuid.Invalidate(); Properties.Reset(); }

	/** Identifier that matches the AutoReplication stream the data belongs to. */
	UPROPERTY()
	FGuid StreamGuid;

	/** Serialized properties that changed since the previous dispatch. */
	UPROPERTY()
	TArray<FGorgeousAutoReplicationPropertyValue> Properties;
};

/** Envelope used by the transporter to resolve the destination variable. */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationPropertyEnvelope
{
	GENERATED_BODY()

	UPROPERTY()
	FName EntryKey;

	UPROPERTY()
	FGorgeousAutoReplicationPropertyPayload Payload;
};

/** Runtime context used to evaluate lifetime conditions & serialization helpers. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationConditionContext
{
	GENERATED_BODY()

	FGorgeousAutoReplicationConditionContext()
		: bIsInitialState(false)
		, bIsOwnerConnection(false)
		, bIsAutonomousProxy(false)
		, bIsSimulatedProxy(false)
		, bHasReplicatedPhysics(false)
		, bIsReplayConnection(false)
		, bCustomConditionResult(false)
		, bDynamicConditionResult(false)
		, bNetGroupMatch(false)
		, PackageMap(nullptr)
		, TargetController(nullptr)
	{
	}

	bool PassesLifetimeCondition(const ELifetimeCondition Condition) const
	{
		switch (Condition)
		{
		case COND_None:
			return true;
		case COND_InitialOnly:
			return bIsInitialState;
		case COND_OwnerOnly:
			return bIsOwnerConnection;
		case COND_SkipOwner:
			return !bIsOwnerConnection;
		case COND_SimulatedOnly:
			return bIsSimulatedProxy;
		case COND_AutonomousOnly:
			return bIsAutonomousProxy;
		case COND_SimulatedOrPhysics:
			return bIsSimulatedProxy || bHasReplicatedPhysics;
		case COND_InitialOrOwner:
			return bIsInitialState || bIsOwnerConnection;
		case COND_Custom:
			return bCustomConditionResult;
		case COND_ReplayOrOwner:
			return bIsReplayConnection || bIsOwnerConnection;
		case COND_ReplayOnly:
			return bIsReplayConnection;
		case COND_SimulatedOnlyNoReplay:
			return bIsSimulatedProxy && !bIsReplayConnection;
		case COND_SimulatedOrPhysicsNoReplay:
			return (bIsSimulatedProxy || bHasReplicatedPhysics) && !bIsReplayConnection;
		case COND_SkipReplay:
			return !bIsReplayConnection;
		case COND_Dynamic:
			return bDynamicConditionResult;
		case COND_Never:
			return false;
		case COND_NetGroup:
			return bNetGroupMatch;
		case COND_Max:
		default:
			return true;
		}
	}

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bIsInitialState;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bIsOwnerConnection;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bIsAutonomousProxy;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bIsSimulatedProxy;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bHasReplicatedPhysics;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bIsReplayConnection;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bCustomConditionResult;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bDynamicConditionResult;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	bool bNetGroupMatch;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	FGuid StreamGuid;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	TObjectPtr<UPackageMap> PackageMap;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Object Variable|Networking")
	TObjectPtr<AGorgeousPlayerController> TargetController;
};
