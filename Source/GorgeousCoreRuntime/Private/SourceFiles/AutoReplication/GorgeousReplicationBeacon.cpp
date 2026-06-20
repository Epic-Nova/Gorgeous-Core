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

#include "AutoReplication/GorgeousReplicationBeacon.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousReplicationBeacon, Log, All);

// ════════════════════════════════════════════════════════════════════════════
//  AGorgeousReplicationBeaconClient
// ════════════════════════════════════════════════════════════════════════════

AGorgeousReplicationBeaconClient::AGorgeousReplicationBeaconClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The beacon type is derived from the class name by the engine.
	// No additional setup required, the host object sets
	// ClientBeaconActorClass in its own constructor.
	// This beacon provides the networking bridge for HTTP API ↔ OV conversion.
}

void AGorgeousReplicationBeaconClient::OnConnected()
{
	UE_LOG(LogGorgeousReplicationBeacon, Log,
		TEXT("HTTP API beacon connected. Claiming managed roots."));

	ClaimManagedRoots();
}

void AGorgeousReplicationBeaconClient::OnFailure()
{
	UE_LOG(LogGorgeousReplicationBeacon, Warning,
		TEXT("HTTP API beacon connection failed. Releasing managed roots."));

	ReleaseManagedRoots();

	Super::OnFailure();
}

// ── HTTP ↔ Object Variable RPCs (stubs, JSON→OV conversion to be implemented) ──

void AGorgeousReplicationBeaconClient::ClientReceiveVariableSnapshot_Implementation(
	const FGuid& VariableIdentifier, const TArray<uint8>& Payload)
{
	// TODO: Deserialize the HTTP API response payload and populate the local OV.
	//       Typically the payload is JSON that should be parsed into an OV tree.
	UE_LOG(LogGorgeousReplicationBeacon, Verbose,
		TEXT("Received API response for variable %s (%d bytes)."),
		*VariableIdentifier.ToString(), Payload.Num());
}

bool AGorgeousReplicationBeaconClient::ServerRequestFullSync_Validate(const FName& RootName)
{
	// Allow any root name, the handler will validate existence.
	return true;
}

void AGorgeousReplicationBeaconClient::ServerRequestFullSync_Implementation(const FName& RootName)
{
	UE_LOG(LogGorgeousReplicationBeacon, Log,
		TEXT("Full variable tree requested for root '%s'."), *RootName.ToString());

	// TODO: Serialize the root's OV tree (built from API responses) and deliver.
	TArray<uint8> EmptyPayload;
	ClientReceiveFullSync(RootName, EmptyPayload);
}

void AGorgeousReplicationBeaconClient::ClientReceiveFullSync_Implementation(
	const FName& RootName, const TArray<uint8>& Payload)
{
	UE_LOG(LogGorgeousReplicationBeacon, Verbose,
		TEXT("Received full OV tree for root '%s' (%d bytes)."),
		*RootName.ToString(), Payload.Num());

	// TODO: Parse the payload (JSON→OV) and rebuild the local variable tree.
}

// ── Root ownership ─────────────────────────────────────────────────────────

void AGorgeousReplicationBeaconClient::ClaimManagedRoots()
{
	// If spawned by a host object, inherit its managed root list.
	if (const AGorgeousReplicationBeaconHostObject* HostObj =
		Cast<AGorgeousReplicationBeaconHostObject>(GetBeaconOwner()))
	{
		ClaimedRootNames = HostObj->ManagedRootNames;
	}

	// If no roots configured, claim all registered roots.
	if (ClaimedRootNames.IsEmpty())
	{
		ClaimedRootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	}

	for (const FName& RootName : ClaimedRootNames)
	{
		const FString StableId = FString::Printf(TEXT("ReplicationBeacon_%s_%s"),
			*RootName.ToString(), *GetName());

		UGorgeousRootObjectVariable::ClaimRootRegistryOwnership(RootName, StableId, this);

		UE_LOG(LogGorgeousReplicationBeacon, Verbose,
			TEXT("Claimed root '%s' with stable ID '%s'."),
			*RootName.ToString(), *StableId);
	}
}

void AGorgeousReplicationBeaconClient::ReleaseManagedRoots()
{
	for (const FName& RootName : ClaimedRootNames)
	{
		if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(RootName))
		{
			// Release back to the GameInstance as fallback.
			if (UGameInstance* GI = GetGameInstance())
			{
				UGorgeousRootObjectVariable::PromoteRootRegistryOwner(RootName, GI);
			}
		}
	}

	ClaimedRootNames.Reset();
}


// ════════════════════════════════════════════════════════════════════════════
//  AGorgeousReplicationBeaconHostObject
// ════════════════════════════════════════════════════════════════════════════

AGorgeousReplicationBeaconHostObject::AGorgeousReplicationBeaconHostObject(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The engine uses these to route incoming beacon connections
	// to the correct host object.  This host manages the HTTP API
	// bridge lifecycle, dispatching requests and converting responses.
	ClientBeaconActorClass = AGorgeousReplicationBeaconClient::StaticClass();
	BeaconTypeName = ClientBeaconActorClass->GetName();
}

AOnlineBeaconClient* AGorgeousReplicationBeaconHostObject::SpawnBeaconActor(
	UNetConnection* ClientConnection)
{
	AOnlineBeaconClient* BeaconActor = Super::SpawnBeaconActor(ClientConnection);

	if (AGorgeousReplicationBeaconClient* GorgeousBeacon =
		Cast<AGorgeousReplicationBeaconClient>(BeaconActor))
	{
		// Pass our managed root configuration to the spawned client.
		GorgeousBeacon->ClaimedRootNames = ManagedRootNames;
	}

	return BeaconActor;
}

void AGorgeousReplicationBeaconHostObject::OnClientConnected(
	AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection)
{
	Super::OnClientConnected(NewClientActor, ClientConnection);

	UE_LOG(LogGorgeousReplicationBeacon, Log,
		TEXT("HTTP API beacon client connected: %s"),
		*NewClientActor->GetName());
}

void AGorgeousReplicationBeaconHostObject::NotifyClientDisconnected(
	AOnlineBeaconClient* LeavingClientActor)
{
	if (AGorgeousReplicationBeaconClient* GorgeousBeacon =
		Cast<AGorgeousReplicationBeaconClient>(LeavingClientActor))
	{
		GorgeousBeacon->ReleaseManagedRoots();
	}

	Super::NotifyClientDisconnected(LeavingClientActor);

	UE_LOG(LogGorgeousReplicationBeacon, Log,
		TEXT("HTTP API beacon client disconnected: %s"),
		*LeavingClientActor->GetName());
}
