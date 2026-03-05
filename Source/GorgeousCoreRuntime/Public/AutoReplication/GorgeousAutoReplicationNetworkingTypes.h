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
	EReliableMulticast UMETA(DisplayName = "Reliable Multicast", ToolTip = "Reliable RPC sent to all clients."),
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

/**
 * Readiness state a responder can report for an in-flight AutoReplication RPC.
 *
 * Call MarkAutoReplicationRPCResponderReady() from inside HandleAutoReplicationRPC
 * (or from any async continuation) to signal how far along the handler is.  The
 * state is forwarded to the RPC debug tracker so the inspector panel can show
 * per-responder progress in real time.
 *
 * NotReadyToCollect          — default; the handler has not finished processing yet.
 * ReadyForSingleResponderCallback — the return value has been produced and will be
 *                                   delivered via OnSingleResponderCompleted shortly.
 * Ready                      — the handler is fully done; results may be collected.
 */
UENUM(BlueprintType)
enum class EGorgeousRPCReadyState : uint8
{
	NotReadyToCollect               UMETA(DisplayName = "Not Ready To Collect"),
	ReadyForSingleResponderCallback UMETA(DisplayName = "Ready For Single Responder Callback"),
	Ready                           UMETA(DisplayName = "Ready")
};

/**
 * A single named argument passed to an AutoReplication RPC handler.
 *
 * The value is stored as a plain byte array so that:
 *  - No UObject wrapper (and therefore no GC risk) is needed for simple scalars.
 *  - The container can be trivially network-serialized without UObject lifetime concerns.
 *
 * Use UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCArgument (CustomThunk)
 * in Blueprint to fill this struct, or the typed helpers (AddAutoReplicationRPCIntArgument etc.).
 *
 * On dispatch the runtime matches ArgumentName to the handler function's parameter FName and
 * copies the bytes directly into the pre-allocated parameter slot via FProperty::CopyCompleteValue
 * (for scalar/struct types) or FMemoryReader deserialization (for FString / dynamic types).
 */
USTRUCT(BlueprintType)
struct FGorgeousRPCArgumentContainer
{
	GENERATED_BODY()

	explicit FGorgeousRPCArgumentContainer(FName InArgumentName = NAME_None)
		: ArgumentName(InArgumentName)
	{
	}

	/** Name must match the handler function parameter exactly (case-insensitive FName compare). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Auto Replication")
	FName ArgumentName;

	/**
	 * Raw serialized value bytes.
	 * - Scalar/struct types: raw memory layout (identical to what FProperty::CopyCompleteValue expects).
	 * - FString / FText: binary UE archive (FMemoryWriter << Value).
	 */
	UPROPERTY()
	TArray<uint8> ValueBytes;

	/**
	 * FProperty subclass name of the stored value, e.g. "IntProperty", "FloatProperty",
	 * "DoubleProperty", "BoolProperty", "StrProperty", "NameProperty", "StructProperty".
	 * Used by the dispatch code to verify compatibility before copying.
	 */
	UPROPERTY()
	FName PropertyClassName;

	/**
	 * For StructProperty arguments: the UScriptStruct name, e.g. "Vector", "Transform",
	 * "Rotator".  Empty for all non-struct types.
	 */
	UPROPERTY()
	FName StructTypeName;
};

/** Serializable payload forwarded through async auto-replication RPC requests. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousRPCPayload
{
	GENERATED_BODY()

    FGorgeousRPCPayload()
		: HandlerName(NAME_None)
		, TimeoutSeconds(0.f)
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
	 * Maximum time in seconds to wait for all expected responders before the request is
	 * considered timed-out.  Set to 0 to use the global default (30 seconds).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Gorgeous Core|Auto Replication",
		meta = (ClampMin = "0", UIMin = "0", UIMax = "120"))
	float TimeoutSeconds;

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

/**
 * Handler context struct for RPC handlers that need deferred completion.
 *
 * DEPRECATED: This struct is no longer auto-injected into handler signatures.
 * Handler functions should simply take const FGorgeousQueuedRPC& as a parameter.
 *
 * To signal readiness, call:
 *   MarkAutoReplicationRPCResponderReady(this, QueuedRPC, Ready)
 *
 * @code
 *   // Blueprint or C++ handler with deferred mode:
 *   void MyHandler(const FGorgeousQueuedRPC& QueuedRPC)
 *   {
 *       DoAsyncWork([this, QueuedRPC](){ MarkAutoReplicationRPCResponderReady(this, QueuedRPC, Ready); });
 *   }
 * @endcode
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationRPCHandlerContext
{
	GENERATED_BODY()

	/** Opaque RPC descriptor — pass directly to MarkAutoReplicationRPCResponderReady. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	FGorgeousQueuedRPC QueuedRPC;

	/** Name of the handler function being invoked (useful for logging / assertions). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	FName HandlerName;

	/**
	 * Stable responder key for this invocation ("Server" / "Client_0" / ...).
	 * Matches what the RPC debug tracker and MarkAutoReplicationRPCResponderReady use.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	FString ResponderKey;
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

/** One name/value property entry inside a serialized OV node. */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousOVPropertyCapture
{
	GENERATED_BODY()

	/** Property name as returned by FProperty::GetName(). */
	UPROPERTY()
	FString PropertyName;

	/** Value serialized via FProperty::ExportTextItem. */
	UPROPERTY()
	FString ExportedValue;
};

/**
 * One node of a captured OV tree.
 * The tree is flattened into an ordered array; the root has an invalid ParentIdentifier.
 * Nodes always appear before their children so reconstruction can proceed in-order.
 */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousSerializedOVNode
{
	GENERATED_BODY()

	/** This OV's GUID — preserved on the reconstructed instance. */
	UPROPERTY()
	FGuid Identifier;

	/** Parent OV's GUID. Invalid for the root node. */
	UPROPERTY()
	FGuid ParentIdentifier;

	/** Full class path for reconstruction (e.g. "/Script/MyModule.UInteger_SOV"). */
	UPROPERTY()
	FString ClassPath;

	/**
	 * The key under which this node was registered in its parent's VariableRegistry TMap.
	 * Invalid (NAME_None) for the root node.
	 */
	UPROPERTY()
	FName RegistryKey;

	/**
	 * All eligible non-structural UPROPERTY values exported as text.
	 * Skipped: VariableRegistry, UniqueIdentifier, object-pointer properties,
	 *          Transient / EditorOnly / Config flags.
	 */
	UPROPERTY()
	TArray<FGorgeousOVPropertyCapture> Properties;
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

	/**
	 * GUID of the root return OV — kept for backwards-compat / same-machine optimisation
	 * (when the OV already lives in the receiver's registry, the tree is not re-created).
	 */
	UPROPERTY()
	FGuid TargetVariableIdentifier;

	/**
	 * Full recursive capture of the handler's return OV tree.
	 * Non-empty only when the result originated on a remote endpoint (client-side handler)
	 * and the OV therefore does not exist in the receiving registry.
	 * The first entry is always the root (return OV); subsequent entries are children in
	 * depth-first order.
	 */
	UPROPERTY()
	TArray<FGorgeousSerializedOVNode> OVTree;

	/**
	 * The ready state under which this result was relayed.
	 *   Ready (default) — result is final; server should call NotifyRequestCompleted.
	 *   ReadyForSingleResponderCallback — interim signal; server should fire
	 *     ExecuteSingleResponderCallback without completing the overall request.
	 */
	UPROPERTY()
	EGorgeousRPCReadyState RelayedReadyState = EGorgeousRPCReadyState::Ready;
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
