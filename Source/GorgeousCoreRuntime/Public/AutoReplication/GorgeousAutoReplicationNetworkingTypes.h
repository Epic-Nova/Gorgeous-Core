// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"

#include "GorgeousAutoReplicationNetworkingTypes.generated.h"

class APlayerController;
/** Identifies which endpoint produced an AutoReplication RPC result. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationRPCResponderHandle
{
	GENERATED_BODY()

	FGorgeousAutoReplicationRPCResponderHandle()
		: bIsServer(false)
		, PlayerControllerId(INDEX_NONE)
	{
	}

	/** True when the responder executed on the authority role. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	bool bIsServer;

	/** Stable key that uniquely identifies the responding connection. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FString ConnectionKey;

	/** Friendly label for debug/UI purposes. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FString PlayerDisplayName;

	/** PlayerController Id captured when available (otherwise -1). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	int32 PlayerControllerId;

	/** Optional debug label describing the responder. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FString DebugLabel;

	bool IsValid() const { return bIsServer || !ConnectionKey.IsEmpty(); }

	/** Returns the key used internally to correlate responses. */
	FString GetStableKey() const { return bIsServer ? TEXT("Server") : ConnectionKey; }

	/** Builds a responder handle representing the authority endpoint. */
	static FGorgeousAutoReplicationRPCResponderHandle MakeServerHandle();

	/** Builds a responder handle for the provided controller (on either server or client). */
	static FGorgeousAutoReplicationRPCResponderHandle FromController(const APlayerController* Controller);
};

class UGorgeousObjectVariable;

/** Defines how auto-replicated RPC requests should be dispatched over the network. */
UENUM(BlueprintType)
enum class EGorgeousAutoReplicationRPCType : uint8
{
	EReliableServer UMETA(DisplayName = "Reliable Server", ToolTip = "Reliable RPC sent to the server."),
	EReliableClient UMETA(DisplayName = "Reliable Client", ToolTip = "Reliable RPC sent to the client."),
	EReliableMulticast UMETA(DisplayName = "Reliable Milticast", ToolTip = "Reliable RPC sent to all clients."),
	EUnreliableServer UMETA(DisplayName = "Unreliable Server", ToolTip = "Unreliable RPC sent to the server."),
	EUnreliableClient UMETA(DisplayName = "Unreliable Client", ToolTip = "Unreliable RPC sent to the client."),
	EUnreliableMulticast UMETA(DisplayName = "Unreliable Multicast", ToolTip = "Unreliable RPC sent to all clients.")
};

/** Defines how a Gorgeous object variable should push data across the network. */
UENUM(BlueprintType)
enum class EGorgeousReplicationMode : uint8
{
	EProperty UMETA(DisplayName = "Property", ToolTip = "Standard property replication using Unreal's built-in serialization."),
	ENetSerialize UMETA(DisplayName = "Net Serialize", ToolTip = "Custom serialization using the property's NetSerialize method."),
	ECustomPayload UMETA(DisplayName = "Custom Payload", ToolTip = "User-defined payload built through Blueprint/C++ overrides.")
};

/** Describes which networking backends an object variable should leverage. */
UENUM(BlueprintType)
enum class EGorgeousObjectVariableReplicationMode : uint8
{
	EFullAutoReplication UMETA(DisplayName = "Full Auto Replication", ToolTip = "Full Auto Replication mode."),
	EHybrid UMETA(DisplayName = "Hybrid (Auto + Legacy)", ToolTip = "Hybrid mode that leverages both Auto Replication and legacy replication."),
	EManual UMETA(DisplayName = "Manual (Legacy Only)", ToolTip = "Manual mode that only uses legacy replication.")
};

/** Specifies which endpoint should handle a queued AutoReplication action. */
UENUM(BlueprintType)
enum class EGorgeousAutoReplicationTargetKind : uint8
{
	EAuto = 0 UMETA(DisplayName = "Auto", ToolTip = "Automatically determine the target based on context."),
	EObjectVariable = 1 UMETA(DisplayName = "Object Variable", ToolTip = "The object variable instance will handle the RPC."),
	EOwner = 2 UMETA(DisplayName = "Owner", ToolTip = "The owning object will handle the RPC."),
	EActorComponent = 3 UMETA(DisplayName = "Actor Component", ToolTip = "The Actor Component attached to the owning Actor will handle the RPC.") //@TODO: Implement ActorComponent target handling
};

USTRUCT(BlueprintType)
struct FGorgeousRPCArgumentContainer
{
	GENERATED_BODY()

	explicit FGorgeousRPCArgumentContainer(FName InArgumentName = NAME_None, UGorgeousObjectVariable* InArgumentValue = nullptr)
		: ArgumentName(InArgumentName)
		, ArgumentValue(InArgumentValue)
	{
	}

	/** Name of the argument as defined in the handler signature. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ArgumentName;

	/** Value passed into the argument. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<UGorgeousObjectVariable> ArgumentValue;
};

/** Serializable payload forwarded through async auto-replication RPC requests. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousRPCPayload
{
	GENERATED_BODY()

    FGorgeousRPCPayload()
		: HandlerName(NAME_None)
		, bReplicateResultToAllConnections(true)
	{
	}

	/** Blueprint/Native handler invoked when the payload arrives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Auto Replication")
	FName HandlerName;

	/** Named arguments passed into the handler. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Auto Replication", meta = (ShowOnlyInnerProperties))
	TArray<FGorgeousRPCArgumentContainer> Arguments;

	/**
	 * When enabled, the generated RPC result container replicates to every connection.
	 * Otherwise it is filtered to the invoker-only lifetime condition.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Gorgeous Core|Auto Replication")
	bool bReplicateResultToAllConnections;
};

/** Serialized RPC waiting to be dispatched through Blueprint hooks. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousQueuedRPC
{
	GENERATED_BODY()

	FGorgeousQueuedRPC()
		: Key(NAME_None)
		, Type(EGorgeousAutoReplicationRPCType::EReliableServer)
		, TargetKind(EGorgeousAutoReplicationTargetKind::EAuto)
		, RequestGuid()
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FName Key;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	EGorgeousAutoReplicationRPCType Type;

	/** Indicates whether this RPC should be handled by a variable entry or the owning QoL class. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	EGorgeousAutoReplicationTargetKind TargetKind;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FGorgeousRPCPayload Payload;

	/** Correlation identifier assigned when the RPC is queued so async nodes can await completion. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FGuid RequestGuid;
};

/** Rich result returned once an AutoReplication RPC finishes executing. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationRPCResult
{
	GENERATED_BODY()

	FGorgeousAutoReplicationRPCResult()
		: TargetKind(EGorgeousAutoReplicationTargetKind::EAuto)
		, TargetVariable(nullptr)
		, TargetOwner(nullptr)
	{
	}

	/** Handle describing which endpoint produced this result. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FGorgeousAutoReplicationRPCResponderHandle Responder;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FGorgeousQueuedRPC QueuedRPC;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	EGorgeousAutoReplicationTargetKind TargetKind;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	TObjectPtr<UGorgeousObjectVariable> TargetVariable;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	TObjectPtr<UObject> TargetOwner;

	/** Unique identifier of the resolved target variable (when available). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FGuid TargetVariableIdentifier;
	
	bool WasHandledByObjectVariable() const { return TargetKind == EGorgeousAutoReplicationTargetKind::EObjectVariable && TargetVariable != nullptr; }
	
	bool WasHandledByOwner() const { return TargetKind == EGorgeousAutoReplicationTargetKind::EOwner && TargetOwner != nullptr; }

	bool WasHandledByActorComponent() const { return TargetKind == EGorgeousAutoReplicationTargetKind::EActorComponent && TargetOwner != nullptr; }
};

/** Lightweight payload replicated through the relay component so the server can rebuild the full result. */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationSerializedRPCResult
{
	GENERATED_BODY()

	FGorgeousAutoReplicationSerializedRPCResult()
		: TargetKind(EGorgeousAutoReplicationTargetKind::EAuto)
	{
	}

	UPROPERTY()
	FGuid RequestGuid;

	UPROPERTY()
	EGorgeousAutoReplicationTargetKind TargetKind;

	UPROPERTY()
	FGorgeousAutoReplicationRPCResponderHandle Responder;

	UPROPERTY()
	FGuid TargetVariableIdentifier;
};

/** Context passed into object variables when auto replication activates. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationContext
{
	GENERATED_BODY()

	FGorgeousAutoReplicationContext()
		: OwningObject(nullptr)
		, EntryKey(NAME_None)
		, ReplicationIndex(MAX_uint32)
	{
	}

	/** Actor/object that owns this variable instance. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	TObjectPtr<UObject> OwningObject;

	/** Key of the AdditionalGorgeousData entry this variable belongs to. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	FName EntryKey;

	/** Replication slot index assigned by the mixin. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Auto Replication")
	int32 ReplicationIndex;
};
