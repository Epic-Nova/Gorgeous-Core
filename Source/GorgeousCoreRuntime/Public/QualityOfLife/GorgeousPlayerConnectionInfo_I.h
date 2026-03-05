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

//<=============================--- Includes ---=============================>
#include "UObject/Interface.h"
#include "GameFramework/OnlineReplStructs.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousPlayerConnectionInfo_I.generated.h"
//<-------------------------------------------------------------------------->

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousPlayerConnectionInfo_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface implemented by AGorgeousPlayerController and AGorgeousPlayerState.
 *
 * Provides a unified, Blueprint-accessible way to query per-player connection metadata
 * directly from an object returned by GetQualityOfLifeReferences().  Because
 * GetQualityOfLifeReferences() returns a plain TArray<UObject*> (which keeps the
 * wildcard pin), callers cast each entry to this interface to get connection details
 * rather than forcing a struct return type that would break DeterminesOutputType.
 *
 * Asymmetry note:
 *   - PlayerController instances only exist for the local player(s) on a client.
 *     Remote proxy PCs only exist on the server, so IsRemoteNetConnection() is
 *     meaningful for PC queries on the server only.
 *   - PlayerState instances are replicated to ALL machines, so querying via PS
 *     is the correct approach for "get all connected players" on any machine.
 */
class GORGEOUSCORERUNTIME_API IGorgeousPlayerConnectionInfo_I
{
	GENERATED_BODY()

public:

	/**
	 * Returns true when this object is a server-side proxy for a remotely connected player.
	 * For PlayerController: true when the PC has no local ULocalPlayer (i.e. !IsLocalController()).
	 * For PlayerState:      true when the owning PC is absent locally (remote player's PS on a client)
	 *                       or the owning PC is not a local controller (remote proxy on the server).
	 * Always returns false on a client for objects belonging to that client's own player.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gorgeous Core|Player Connection",
		meta = (CompactNodeTitle = "Is Remote?"))
	bool IsRemoteNetConnection() const;

	/**
	 * Returns the engine-level unique net ID for this player.
	 * Valid on all machines when queried through PlayerState.
	 * When queried through PlayerController, only available where valid PlayerState exists.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gorgeous Core|Player Connection",
		meta = (CompactNodeTitle = "Net ID"))
	FUniqueNetIdRepl GetPlayerNetId() const;

	/**
	 * Returns the stable string ID registered in GorgeousLocalPlayerRegistry_GIS.
	 * Non-empty only for locally controlled players on the machine that owns them.
	 * Returns an empty string for remote players since they are never registered
	 * in the local machine's stable ID registry.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gorgeous Core|Player Connection",
		meta = (CompactNodeTitle = "Stable ID"))
	FString GetGorgeousStablePlayerId() const;

	/**
	 * Returns a numeric player index for this connection.
	 * For local players: the splitscreen slot index from ULocalPlayer.
	 * For remote players (via PlayerState): APlayerState::GetPlayerId().
	 * Returns -1 if the index cannot be determined.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gorgeous Core|Player Connection",
		meta = (CompactNodeTitle = "Connection Index"))
	int32 GetPlayerConnectionIndex() const;
};
