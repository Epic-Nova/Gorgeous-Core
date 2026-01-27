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

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "GorgeousAutoReplicationNetworkingLibrary.generated.h"

class APlayerController;

/** Blueprint helpers for AutoReplication networking tasks. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationNetworkingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Builds an identifier by combining the AutoReplication connection key with platform fingerprints (OS/Login/hostname).
	 * This keeps the value stable across reconnects on the same PC while still differentiating individual controllers.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	static FString MakeStablePlayerConnectionId(const APlayerController* PlayerController);
};
