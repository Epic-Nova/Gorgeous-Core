// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
| <==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GorgeousAutoReplicationSpawnHook_I.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousAutoReplicationSpawnHook_I : public UInterface
{
	GENERATED_BODY()
};

/** Interface implemented by objects that wish to react to being instantiated via the auto-replication system. */
class GORGEOUSCORERUNTIME_API IGorgeousAutoReplicationSpawnHook_I
{
	GENERATED_BODY()

public:
	/** 
	 * Invoked on the client when this object has been instantiated and its replicated properties 
	 * have been initialized from the server-provided snapshot.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|AutoReplication|Networking")
	void OnSpawnedThroughAutoReplication();
};
