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
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousAutoReplicationRPCResponder_I.generated.h"
//<-------------------------------------------------------------------------->

UINTERFACE(Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCResponder_I : public UInterface
{
	GENERATED_BODY()
};

/** Interface implemented by AutoReplication classes that wish to receive async RPC callbacks automatically. */
class GORGEOUSCORERUNTIME_API IGorgeousAutoReplicationRPCResponder_I
{
	GENERATED_BODY()

public:
	/** Invoked whenever the AutoReplication mixin drains a pending RPC entry for this owner. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|AutoReplication|Networking")
	void HandleAutoReplicationRPC(const FGorgeousQueuedRPC& QueuedRPC);
};