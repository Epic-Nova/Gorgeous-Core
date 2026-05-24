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
#include "OnlineBeaconHostObject.h"
#include "OnlineBeaconClient.h"

#include "GorgeousReplicationBeacon.generated.h"

class UGorgeousObjectVariable;
class UGorgeousRootObjectVariable;

//@TODO: This is a highly theoretical concept, this will probably introduced in v1.3

// ════════════════════════════════════════════════════════════════════════════
//  AGorgeousReplicationBeaconClient
//
//  A dedicated beacon Actor operating on its own NetDriver, completely
//  independent of the game's primary networking channel.
//
//  Purpose:
//    • Send outgoing HTTP requests (e.g. to LLM APIs, REST endpoints)
//      and receive the response structured as an Object Variable tree.
//      JSON responses are automatically parsed into OVs for convenience.
//    • Receive incoming HTTP API calls and populate Object Variable
//      values from the request payload.
//
//  This is the stable networking bridge for handling large data sets
//  through external HTTP services while keeping all results accessible
//  as first-class Object Variables within the Gorgeous ecosystem.
//
//  Once connected, the beacon claims root registry ownership so that
//  all Object Variables under managed roots use it as the networking
//  context (CachedOwner / FallbackOwner).
// ════════════════════════════════════════════════════════════════════════════

UCLASS(Transient, NotPlaceable, Config = Engine)
class GORGEOUSCORERUNTIME_API AGorgeousReplicationBeaconClient : public AOnlineBeaconClient
{
	GENERATED_BODY()

public:
	AGorgeousReplicationBeaconClient(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ── AOnlineBeaconClient interface ──────────────────────────────────────

	/** Called when the beacon connection is fully established. */
	virtual void OnConnected() override;

	/** Called on connection failure — releases ownership claims. */
	virtual void OnFailure() override;

	// ── HTTP ↔ Object Variable RPCs ────────────────────────────────────────
	// Transport layer for bridging HTTP API calls to/from Object Variables.
	// Responses (typically JSON) are structured into OV trees automatically.

	/** Server → Client: push an API response payload for a specific variable. */
	UFUNCTION(Client, Reliable)
	void ClientReceiveVariableSnapshot(const FGuid& VariableIdentifier, const TArray<uint8>& Payload);

	/** Client → Server: request a full variable tree for a root (e.g. after an API response). */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestFullSync(const FName& RootName);

	/** Server → Client: deliver the full variable tree built from an API response. */
	UFUNCTION(Client, Reliable)
	void ClientReceiveFullSync(const FName& RootName, const TArray<uint8>& Payload);

	// ── Root ownership ─────────────────────────────────────────────────────

	/** The root names this beacon has claimed ownership of. */
	UPROPERTY()
	TArray<FName> ClaimedRootNames;

	/** Releases any root registry ownership this beacon held. */
	void ReleaseManagedRoots();
	
protected:
	/** Claims root registry ownership for all configured roots. */
	void ClaimManagedRoots();

};


// ════════════════════════════════════════════════════════════════════════════
//  AGorgeousReplicationBeaconHostObject
//
//  Server-side factory/manager registered with AOnlineBeaconHost.
//  Spawns AGorgeousReplicationBeaconClient actors for incoming
//  connections and manages the HTTP ↔ Object Variable bridge lifecycle.
//
//  This is the entry point for external HTTP API integration:
//  incoming API calls are received here and their payloads are
//  converted into Object Variable trees.  Outgoing HTTP requests
//  (e.g. to LLM APIs for processing large data sets) are dispatched
//  through connected beacon clients, and their responses flow back
//  as structured OVs.
// ════════════════════════════════════════════════════════════════════════════

UCLASS(Transient, NotPlaceable, Config = Engine)
class GORGEOUSCORERUNTIME_API AGorgeousReplicationBeaconHostObject : public AOnlineBeaconHostObject
{
	GENERATED_BODY()

public:
	AGorgeousReplicationBeaconHostObject(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ── AOnlineBeaconHostObject interface ──────────────────────────────────

	/** Override: spawn our custom beacon client for the incoming connection. */
	virtual AOnlineBeaconClient* SpawnBeaconActor(UNetConnection* ClientConnection) override;

	/** Override: called after handshake — track the connected client. */
	virtual void OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) override;

	/** Override: called when a client disconnects. */
	virtual void NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) override;

	// ── Configuration ──────────────────────────────────────────────────────

	/** Root names that connected beacons should claim ownership of. */
	UPROPERTY(Config)
	TArray<FName> ManagedRootNames;
};
