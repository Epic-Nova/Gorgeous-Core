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

#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Player.h"
#include "Engine/LocalPlayer.h"

namespace GorgeousAutoReplicationNetworkingTypes_Private
{
	static FString BuildConnectionKeyFromController(const APlayerController* Controller)
	{
		if (!Controller)
		{
			return FString();
		}

		if (const APlayerState* PlayerState = Controller->PlayerState)
		{
			FString NetIdString;
			if (PlayerState->GetUniqueId().IsValid())
			{
				NetIdString = PlayerState->GetUniqueId()->ToString();
			}

			if (!NetIdString.IsEmpty())
			{
				return NetIdString;
			}

			return FString::Printf(TEXT("PC_%d_%s"), PlayerState->GetPlayerId(), *PlayerState->GetPlayerName());
		}

		return Controller->GetName();
	}
}

FGorgeousAutoReplicationRPCResponderHandle FGorgeousAutoReplicationRPCResponderHandle::MakeServerHandle()
{
	FGorgeousAutoReplicationRPCResponderHandle Handle;
	Handle.bIsServer = true;
	Handle.ConnectionKey = TEXT("Server");
	Handle.PlayerDisplayName = TEXT("Server");
	Handle.PlayerControllerId = INDEX_NONE;
	Handle.DebugLabel = TEXT("Authority");
	return Handle;
}

FGorgeousAutoReplicationRPCResponderHandle FGorgeousAutoReplicationRPCResponderHandle::FromController(const APlayerController* Controller)
{
	FGorgeousAutoReplicationRPCResponderHandle Handle;
	if (!Controller)
	{
		Handle.DebugLabel = TEXT("UnknownController");
		return Handle;
	}

	// bIsServer is true only when this controller IS the local authority endpoint
	// (i.e. the listen-server host's own PC).  Remote client PCs that live on the
	// server also have HasAuthority()==true, but they represent client-side responders
	// and must use their ConnectionKey so the debug timeline shows distinct labels.
	Handle.bIsServer = Controller->HasAuthority() && Controller->IsLocalController();
	Handle.ConnectionKey = GorgeousAutoReplicationNetworkingTypes_Private::BuildConnectionKeyFromController(Controller);
	if (const ULocalPlayer* LocalPlayer = Controller->GetLocalPlayer())
	{
		Handle.PlayerControllerId = LocalPlayer->GetControllerId();
	}
	else
	{
		Handle.PlayerControllerId = INDEX_NONE;
	}
	// Use the UObject name (e.g. "BP_GPC_0") so the timeline shows a stable,
	// immediately recognisable identifier rather than the player-set display name.
	Handle.PlayerDisplayName = Controller->GetName();
	Handle.DebugLabel = FString::Printf(TEXT("%s_%s"), Handle.bIsServer ? TEXT("Srv") : TEXT("Cli"), *Handle.PlayerDisplayName);
	return Handle;
}
