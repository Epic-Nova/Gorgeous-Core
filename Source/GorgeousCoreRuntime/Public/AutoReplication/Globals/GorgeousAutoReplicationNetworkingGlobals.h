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
#include "GorgeousCoreRuntimeGlobals.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousAutoReplicationNetworkingGlobals.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Auto Replication Networking Globals
| Functional Name: UGorgeousAutoReplicationNetworkingGlobals
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Auto Replication Networking
| Globals.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/NetworkingLibrary/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/NetworkingLibrary/",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/AutoReplication/NetworkingLibrary/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationNetworkingGlobals : public UGorgeous
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
#pragma region Blueprint Functions
public:

	/**
	 * Returns the authenticated platform user ID when available, otherwise a persisted random GUID for the local player slot.
	 * Local fallback identifiers are stored under Saved/Gorgeous/ConnectionIds and distinguish local player slots without exposing machine identifiers.
	 *
	 * @param PlayerController The player controller to build the identifier for.
	 * @return A stable identifier for the player's connection, which can be used for tracking players across disconnects/reconnects.
	 *
	 * @note This function is designed to be stable for the same player on the same machine, but will produce different identifiers for different players or the same player on a different machine.
	 * A local GUID is a reconnect hint, never proof of identity or authorization.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	static FString MakeStablePlayerConnectionId(const APlayerController* PlayerController);
	//<------------------------------------------------------------------------->
#pragma endregion Blueprint Functions
};

using GT_AR_NetworkingLibrary = UGorgeousAutoReplicationNetworkingGlobals;