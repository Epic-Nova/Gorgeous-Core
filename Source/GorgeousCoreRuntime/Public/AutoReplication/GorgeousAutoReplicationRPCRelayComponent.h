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

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Components/ActorComponent.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousAutoReplicationRPCRelayComponent.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousAutoReplicationRPCRequestAsyncAction;
class FGorgeousAutoReplicationMixin;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Auto Replication RPCRelay Component
| Functional Name: UGorgeousAutoReplicationRPCRelayComponent
| Parent Class: UActorComponent
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Component that lives on each player controller to shuttle AutoReplication
| RPC results back to the authority so asynchronous requests can aggregate
| per-connection responses. Also handles relaying property payloads from
| client to server.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(ClassGroup = (GorgeousCore), Blueprintable, BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/GorgeousAutoReplicationRPCRelayComponent",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCRelayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGorgeousAutoReplicationRPCRelayComponent();

	/** Queues a local result so it can be forwarded to the authority instance. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking")
	void RelayResultToServer(const FGorgeousAutoReplicationRPCResult& Result,
		EGorgeousRPCReadyState ReadyState = EGorgeousRPCReadyState::Ready);

	/** Relays a property payload from client to server. Returns true if the relay was initiated. */
	bool RelayPropertyPayloadToServer(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, FGorgeousAutoReplicationMixin* TargetMixin);

	/** Relays a property payload from server to owning client. Returns true if the relay was initiated. */
	bool RelayPropertyPayloadToClient(const FGorgeousAutoReplicationPropertyEnvelope& Envelope);

	/** Relays an RPC from client to server. Returns true if the relay was initiated. */
	bool RelayRPCToServer(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable, FGorgeousAutoReplicationMixin* TargetMixin);

	/**
	 * Sends the responder's current ready-state to the server so the authority-side debug
	 * tracker (RPC Inspector) reflects the change instantly.  Only sends when on a client
	 * and the debug tracker is enabled.
	 */
	void RelayResponderReadyStateToServer(const FGuid& RequestGuid, const FString& ResponderKey, EGorgeousRPCReadyState ReadyState);

	/**
	 * Register that a client-initiated RPC arrived from OriginPC so any resulting
	 * NotifyRequestCompleted calls on the server can be relayed back to that client.
	 * Must be called on the server before the RPC is dispatched.
	 */
	static void RegisterClientInitiatedRPCOrigin(const FGuid& RequestGuid, APlayerController* OriginPC);

	/**
	 * If RequestGuid was registered as a client-initiated RPC, relay the result to the
	 * originating client via ClientRelayAutoReplicationResult.  Otherwise a no-op.
	 * Safe to call from any static context on the server.
	 */
	static void TryRelayResultToClientInitiator(const FGorgeousAutoReplicationRPCResult& Result,
		EGorgeousRPCReadyState ReadyState = EGorgeousRPCReadyState::Ready);

	/** Sets the target mixin for incoming property payloads. */
	void SetTargetMixin(FGorgeousAutoReplicationMixin* InMixin) { TargetMixin = InMixin; }

protected:
	UFUNCTION(Server, Reliable)
	void ServerRelayAutoReplicationResult(const FGorgeousAutoReplicationSerializedRPCResult& SerializedResult);

	UFUNCTION(Server, Reliable)
	void ServerRelayPropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope);

	UFUNCTION(Server, Reliable)
	void ServerRelayRPCReliable(const FGorgeousQueuedRPC& QueuedRPC);

	UFUNCTION(Server, Unreliable)
	void ServerRelayRPCUnreliable(const FGorgeousQueuedRPC& QueuedRPC);

	/**
	 * Relays a responder ready-state change from client to server so the debug tracker
	 * on the authority machine updates instantly when Mark RPC Responder Ready is called
	 * from a remote machine.  Only fires when the debug tracker is enabled.
	 */
	UFUNCTION(Server, Reliable)
	void ServerRelayResponderReadyState(FGuid RequestGuid, const FString& ResponderKey, EGorgeousRPCReadyState ReadyState);

	UFUNCTION(Client, Reliable)
	void ClientRelayPropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope);

	/**
	 * Delivers a fully-formed RPC result to the client that originally initiated the RPC.
	 * Called by the server whenever it resolves a result for a client-initiated RPC.
	 */
	UFUNCTION(Client, Reliable)
	void ClientRelayAutoReplicationResult(const FGorgeousAutoReplicationSerializedRPCResult& SerializedResult);

	/**
	 * Universal native RPC handler exercising the first-parameter OV return-value convention.
	 * Parameter names match BuildVerifiablePayload exactly so that InvokeNativeHandler can
	 * map them without error when the EActorComponent dispatch path iterates PC components.
	 *
	 * The backend constructs a UInteger_SOV* and injects it as ReturnValue. This handler sets
	 *   ReturnValue->Value = TestInputInt * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD
	 * so the E9 assertion in RunAsyncRPCActionTest can verify the full round-trip.
	 */
	UFUNCTION()
	void Automation_HandleRPC_WithReturnOV(UInteger_SOV* ReturnValue, int32 TestInputInt,
		const FString& TestInputString, int32 Sequence, const FString& Origin, const FString& Timestamp);

private:
	void SerializeResult(const FGorgeousAutoReplicationRPCResult& Result, FGorgeousAutoReplicationSerializedRPCResult& OutSerialized) const;

	/** The mixin that will receive relayed property payloads on the server. */
	FGorgeousAutoReplicationMixin* TargetMixin = nullptr;

	/**
	 * Server-side map: RequestGuid → the PlayerController that originally relayed the RPC
	 * via ServerRelayRPCReliable/Unreliable.  Used to route result callbacks back to the
	 * initiating client when the PendingRequests entry does not exist on the server.
	 */
	static TMap<FGuid, TWeakObjectPtr<APlayerController>> ClientInitiatedRPCOrigins;
};