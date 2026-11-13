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
#include "ObjectVariables/GorgeousObjectVariable.h"

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
	// Snapshot payloads use the existing binary OV format on this beacon connection.
}

void AGorgeousReplicationBeaconClient::OnConnected()
{
	UE_LOG(LogGorgeousReplicationBeacon, Log,
		TEXT("Replication snapshot beacon connected. Claiming configured managed roots."));

	ClaimManagedRoots();
}

void AGorgeousReplicationBeaconClient::OnFailure()
{
	UE_LOG(LogGorgeousReplicationBeacon, Warning,
		TEXT("Replication snapshot beacon connection failed. Releasing managed roots."));

	ReleaseManagedRoots();

	Super::OnFailure();
}

// ── Bounded binary Object Variable snapshot RPCs ───────────────────────────

void AGorgeousReplicationBeaconClient::ClientReceiveVariableSnapshot_Implementation(const FGuid& Identifier,const TArray<uint8>& Payload)
{
    if (!Identifier.IsValid() || Payload.IsEmpty() || Payload.Num()>1024*1024) return;
    UGorgeousObjectVariable* Snapshot=UGorgeousObjectVariable::DeserializeOVFromRPCArgumentBytes(Payload,false);
    if (!Snapshot) return;
    if (auto* Existing=ReceivedVariableSnapshots.FindRef(Identifier).Get()) UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Existing);
    Snapshot->SetFallbackOwner(this);
    ReceivedVariableSnapshots.Add(Identifier,Snapshot);
}
bool AGorgeousReplicationBeaconClient::ServerRequestFullSync_Validate(const FName& RootName)
{
    // The host must explicitly expose a root. A beacon connection never grants private gameplay access.
    return !RootName.IsNone() && ClaimedRootNames.Contains(RootName) && UGorgeousRootObjectVariable::TryGetExistingRoot(RootName)!=nullptr;
}
void AGorgeousReplicationBeaconClient::ServerRequestFullSync_Implementation(const FName& RootName)
{
    if (!ServerRequestFullSync_Validate(RootName)) return;
    UGorgeousRootObjectVariable* Root=UGorgeousRootObjectVariable::TryGetExistingRoot(RootName);
    for (auto* Variable:UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(RootName))
        if (Variable && Variable->GetEffectiveAccessPolicy()!=EGorgeousObjectVariableAccessPolicy::Everyone) return;
    TArray<uint8> Bytes;
    if (UGorgeousObjectVariable::SerializeOVToRPCArgumentBytes(Root,Bytes) && Bytes.Num()<=1024*1024) ClientReceiveFullSync(RootName,Bytes);
}
void AGorgeousReplicationBeaconClient::ClientReceiveFullSync_Implementation(const FName& RootName,const TArray<uint8>& Payload)
{
    if (RootName.IsNone() || Payload.IsEmpty() || Payload.Num()>1024*1024) return;
    UGorgeousObjectVariable* Snapshot=UGorgeousObjectVariable::DeserializeOVFromRPCArgumentBytes(Payload,false);
    if (!Snapshot) return;
    if (auto* Existing=ReceivedRootSnapshots.FindRef(RootName).Get()) UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Existing);
    Snapshot->SetFallbackOwner(this);
    ReceivedRootSnapshots.Add(RootName,Snapshot);
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

    // No implicit exposure of every project root. Managed roots are an explicit host opt-in.

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
    for (const auto& Pair:ReceivedRootSnapshots) if (Pair.Value) UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Pair.Value);
    for (const auto& Pair:ReceivedVariableSnapshots) if (Pair.Value) UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Pair.Value);
    ReceivedRootSnapshots.Reset(); ReceivedVariableSnapshots.Reset();

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
	// to the correct host object. The host exposes configured roots through
	// binary snapshot RPCs; external HTTP/JSON adapters are outside this class.
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
		TEXT("Replication snapshot beacon client connected: %s"),
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
		TEXT("Replication snapshot beacon client disconnected: %s"),
		*LeavingClientActor->GetName());
}

void AGorgeousReplicationBeaconClient::EndPlay(const EEndPlayReason::Type Reason)
{
    ReleaseManagedRoots();
    Super::EndPlay(Reason);
}
