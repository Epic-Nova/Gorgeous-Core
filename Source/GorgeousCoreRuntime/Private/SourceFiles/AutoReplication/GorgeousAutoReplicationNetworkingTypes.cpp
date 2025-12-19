// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
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

	Handle.bIsServer = Controller->HasAuthority();
	Handle.ConnectionKey = GorgeousAutoReplicationNetworkingTypes_Private::BuildConnectionKeyFromController(Controller);
	if (const ULocalPlayer* LocalPlayer = Controller->GetLocalPlayer())
	{
		Handle.PlayerControllerId = LocalPlayer->GetControllerId();
	}
	else
	{
		Handle.PlayerControllerId = INDEX_NONE;
	}
	Handle.PlayerDisplayName = Controller->PlayerState ? Controller->PlayerState->GetPlayerName() : Controller->GetName();
	Handle.DebugLabel = FString::Printf(TEXT("%s_%s"), Handle.bIsServer ? TEXT("Srv") : TEXT("Cli"), *Handle.PlayerDisplayName);
	return Handle;
}
