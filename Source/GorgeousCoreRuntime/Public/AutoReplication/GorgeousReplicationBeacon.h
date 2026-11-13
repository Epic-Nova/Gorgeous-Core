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
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "OnlineBeaconHostObject.h"
#include "OnlineBeaconClient.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousReplicationBeacon.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousObjectVariable;
class UGorgeousRootObjectVariable;
//<------------------------------------------------------------->
// Bounded binary Object Variable snapshots over an explicitly configured Unreal beacon.
// This class does not provide an HTTP server/client or automatic JSON conversion.

// ════════════════════════════════════════════════════════════════════════════
//  AGorgeousReplicationBeaconClient
//
//  A dedicated beacon Actor operating on its own NetDriver, completely
//  independent of the game's primary networking channel.
//
//  Purpose:
//    • Request a binary snapshot of a root explicitly exposed by the host.
//    • Receive binary variable/root trees into this connection's snapshot maps.
//
//  The host application configures the beacon connection and exposed roots.
//  External services and JSON-to-OV mapping require a separate application
//  adapter; no such service integration is performed by these snapshot RPCs.
//
//  Once connected, the beacon can claim its configured local managed roots.
//  Received snapshots are retained per connection and released on teardown.
// ════════════════════════════════════════════════════════════════════════════

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Replication Beacon Client
| Functional Name: AGorgeousReplicationBeaconClient
| Parent Class: AOnlineBeaconClient
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Replication Beacon Client.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Transient, NotPlaceable, Config = Engine,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Reference/KnownLimitations/#placeholders",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/Overview/#usage-examples"
		)
)
class GORGEOUSCORERUNTIME_API AGorgeousReplicationBeaconClient : public AOnlineBeaconClient
{
	GENERATED_BODY()

public:
	AGorgeousReplicationBeaconClient(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ── AOnlineBeaconClient interface ──────────────────────────────────────

	/** Called when the beacon connection is fully established. */
	virtual void OnConnected() override;

	/** Called on connection failure, releases ownership claims. */
	virtual void OnFailure() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    /** Last received snapshot for this connection; independent of process-wide gameplay roots. */
    UFUNCTION(BlueprintPure,Category="Gorgeous Core|AutoReplication|Beacon")
    UGorgeousObjectVariable* GetReceivedRootSnapshot(FName RootName) const { return ReceivedRootSnapshots.FindRef(RootName); }
    UFUNCTION(BlueprintPure,Category="Gorgeous Core|AutoReplication|Beacon")
    UGorgeousObjectVariable* GetReceivedVariableSnapshot(FGuid Identifier) const { return ReceivedVariableSnapshots.FindRef(Identifier); }
    UPROPERTY(Transient)
    TMap<FName,TObjectPtr<UGorgeousObjectVariable>> ReceivedRootSnapshots;
    UPROPERTY(Transient)
    TMap<FGuid,TObjectPtr<UGorgeousObjectVariable>> ReceivedVariableSnapshots;


	// ── Binary Object Variable snapshot RPCs ───────────────────────────────
	// Uses the OV snapshot format, not an HTTP body or a JSON document.

	/** Server → Client: receive a bounded binary snapshot for a specific variable. */
	UFUNCTION(Client, Reliable)
	void ClientReceiveVariableSnapshot(const FGuid& VariableIdentifier, const TArray<uint8>& Payload);

	/** Client → Server: request a full variable tree for an explicitly exposed root. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestFullSync(const FName& RootName);

	/** Server → Client: receive the root's binary tree in this connection's snapshot map. */
	UFUNCTION(Client, Reliable)
	void ClientReceiveFullSync(const FName& RootName, const TArray<uint8>& Payload);

	// ── Root ownership ─────────────────────────────────────────────────────

	// The root names this beacon has claimed ownership of.
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
//  connections and configures their explicitly exposed root names.
//
//  This is an Unreal beacon host object, not an HTTP endpoint. It does not
//  send external API requests or convert JSON automatically. Applications
//  which need those features must provide their own service adapter.
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

	/** Override: called after handshake, track the connected client. */
	virtual void OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) override;

	/** Override: called when a client disconnects. */
	virtual void NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) override;

	// ── Configuration ──────────────────────────────────────────────────────

	// Root names that connected beacons should claim ownership of.
	UPROPERTY(Config)
	TArray<FName> ManagedRootNames;
};
