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

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "GorgeousAutoReplicationRPCResponder_I.generated.h"

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

