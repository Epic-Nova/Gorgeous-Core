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
#include "QualityOfLife/GorgeousPlayerState.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "AutoReplication//GorgeousAutoReplicationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// AGorgeousPlayerState Implementation
//=============================================================================

AGorgeousPlayerState::AGorgeousPlayerState()
	: bActivateNetworkingCapabilities(true)
{
	UE_QOL_INITIALIZE_ADDITIONAL_DATA();
	AutoReplicationRPCRelay = CreateDefaultSubobject<UGorgeousAutoReplicationRPCRelayComponent>(TEXT("AutoReplicationRPCRelay"));
}

UE_QOL_DEFINE_HANDLE_AUTOREPLICATION_RPC(AGorgeousPlayerState)

UE_QOL_DEFINE_BEGIN_PLAY_WITH_RELAY(AGorgeousPlayerState)

UE_QOL_DEFINE_REGISTER_AUTOREPLICATION_ENTRY(AGorgeousPlayerState)

UE_QOL_DEFINE_POST_INIT_AND_LOAD(AGorgeousPlayerState)

void AGorgeousPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGorgeousPlayerState, ReplicatedAutoReplicationVariables);
	DOREPLIFETIME(AGorgeousPlayerState, ReplicatedGorgeousStableId);
}

UE_DECLARE_AUTOREPLICATION_CLASS_ON_REP_VARIABLES(AGorgeousPlayerState)

UE_QOL_DEFINE_POST_EDIT_CHANGE_PROPERTY(AGorgeousPlayerState)

UE_QOL_DEFINE_END_PLAY(AGorgeousPlayerState)

// ── IGorgeousPlayerConnectionInfo_I ──────────────────────────────────────

bool AGorgeousPlayerState::IsRemoteNetConnection_Implementation() const
{
	const APlayerController* PC = Cast<APlayerController>(GetOwningController());
	if (!PC)
	{
		// No owning controller reachable — this is a remote player's PS on a client
		// (the matching PC only exists on the server).
		return true;
	}
	return !PC->IsLocalController();
}

FUniqueNetIdRepl AGorgeousPlayerState::GetPlayerNetId_Implementation() const
{
	return GetUniqueId();
}

FString AGorgeousPlayerState::GetGorgeousStablePlayerId_Implementation() const
{
	// Prefer the replicated stable ID — it is visible on all machines including
	// clients reading another player's PS.  Fall back to the local registry for
	// the local player before the first replication cycle completes.
	if (!ReplicatedGorgeousStableId.IsEmpty())
	{
		return ReplicatedGorgeousStableId;
	}
	if (const APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		return FGorgeousQualityOfLifeStatics::GetLocalPlayerStableId(PC);
	}
	return FString();
}

int32 AGorgeousPlayerState::GetPlayerConnectionIndex_Implementation() const
{
	// GetPlayerId() is replicated to all machines - it's the engine's authoritative
	// per-connection player index and works on both server and client.
	return GetPlayerId();
}

void AGorgeousPlayerState::SetGorgeousStablePlayerId(const FString& NewId)
{
	if (HasAuthority())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetOwningController()))
		{
			FGorgeousQualityOfLifeStatics::RegisterLocalPlayerStableId(PC, NewId);
		}
		ReplicatedGorgeousStableId = NewId;
		// Fire directly on server since OnRep only fires on clients.
		OnGorgeousStablePlayerIdChanged(NewId);
	}
	else
	{
		Server_SetGorgeousStablePlayerId(NewId);
	}
}

void AGorgeousPlayerState::Server_SetGorgeousStablePlayerId_Implementation(const FString& NewId)
{
	if (NewId.IsEmpty()) { return; }
	if (APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		FGorgeousQualityOfLifeStatics::RegisterLocalPlayerStableId(PC, NewId);
	}
	ReplicatedGorgeousStableId = NewId;
	OnGorgeousStablePlayerIdChanged(NewId);
}

void AGorgeousPlayerState::OnRep_GorgeousStableId()
{
	// Sync the local registry so GetLocalPlayerStableId(PC) stays consistent.
	if (APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		FGorgeousQualityOfLifeStatics::RegisterLocalPlayerStableId(PC, ReplicatedGorgeousStableId);
	}
	OnGorgeousStablePlayerIdChanged(ReplicatedGorgeousStableId);
}

void AGorgeousPlayerState::RefreshReplicatedStableId()
{
	if (!HasAuthority()) { return; }
	if (const APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		const FString CurrentId = FGorgeousQualityOfLifeStatics::GetLocalPlayerStableId(PC);
		if (!CurrentId.IsEmpty())
		{
			ReplicatedGorgeousStableId = CurrentId;
		}
	}
}

