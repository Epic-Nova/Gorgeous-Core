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
#include "Components/ActorComponent.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "GorgeousAutoReplicationRPCRelayComponent.generated.h"

class UGorgeousAutoReplicationRPCRequestAsyncAction;
class FGorgeousAutoReplicationMixin;

/**
 * Component that lives on each player controller to shuttle AutoReplication RPC
 * results back to the authority so asynchronous requests can aggregate per-connection
 * responses. Also handles relaying property payloads from client to server.
 */
UCLASS(ClassGroup = (GorgeousCore), Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCRelayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGorgeousAutoReplicationRPCRelayComponent();

	/** Queues a local result so it can be forwarded to the authority instance. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking")
	void RelayResultToServer(const FGorgeousAutoReplicationRPCResult& Result);

	/** Relays a property payload from client to server. Returns true if the relay was initiated. */
	bool RelayPropertyPayloadToServer(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, FGorgeousAutoReplicationMixin* TargetMixin);

	/** Relays a property payload from server to owning client. Returns true if the relay was initiated. */
	bool RelayPropertyPayloadToClient(const FGorgeousAutoReplicationPropertyEnvelope& Envelope);

	/** Relays an RPC from client to server. Returns true if the relay was initiated. */
	bool RelayRPCToServer(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable, FGorgeousAutoReplicationMixin* TargetMixin);

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

	UFUNCTION(Client, Reliable)
	void ClientRelayPropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope);

private:
	void SerializeResult(const FGorgeousAutoReplicationRPCResult& Result, FGorgeousAutoReplicationSerializedRPCResult& OutSerialized) const;

	/** The mixin that will receive relayed property payloads on the server. */
	FGorgeousAutoReplicationMixin* TargetMixin = nullptr;
};
