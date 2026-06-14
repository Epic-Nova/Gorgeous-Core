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
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreRuntimeGlobals.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousAutoReplicationNetworkingGlobals.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous AutoReplication Networking Globals
| Functional Name: UGorgeousAutoReplicationNetworkingGlobals
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides Blueprint-accessible helper functions for 
| networking-related tasks in the context of AutoReplication.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationNetworkingGlobals : public UGorgeous
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
public:

	/**
	 * Builds an identifier by combining the AutoReplication connection key with platform fingerprints (OS/Login/hostname).
	 * This keeps the value stable across reconnects on the same PC while still differentiating individual controllers.
	 * 
	 * @param PlayerController The player controller to build the identifier for.
	 * @return A stable identifier for the player's connection, which can be used for tracking players across disconnects/reconnects.
	 * 
	 * @note This function is designed to be stable for the same player on the same machine, but will produce different identifiers for different players or the same player on a different machine. 
	 * It is not intended to be globally unique or secure, but rather to provide a consistent identifier for a player's connection within the context of AutoReplication.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	static FString MakeStablePlayerConnectionId(const APlayerController* PlayerController);
	//<------------------------------------------------------------------------->
};

using GT_AR_NetworkingLibrary = UGorgeousAutoReplicationNetworkingGlobals;