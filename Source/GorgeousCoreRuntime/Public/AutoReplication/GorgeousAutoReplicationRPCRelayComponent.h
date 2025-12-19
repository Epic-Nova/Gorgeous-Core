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

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "GorgeousAutoReplicationRPCRelayComponent.generated.h"

class UGorgeousAutoReplicationRPCRequestAsyncAction;

/**
 * Component that lives on each player controller to shuttle AutoReplication RPC
 * results back to the authority so asynchronous requests can aggregate per-connection
 * responses.
 */
UCLASS(ClassGroup = (GorgeousCore), Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCRelayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGorgeousAutoReplicationRPCRelayComponent();

	/** Queues a local result so it can be forwarded to the authority instance. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking")
	void RelayResultToServer(const FGorgeousAutoReplicationRPCResult& Result);

protected:
	UFUNCTION(Server, Reliable)
	void ServerRelayAutoReplicationResult(const FGorgeousAutoReplicationSerializedRPCResult& SerializedResult);

private:
	void SerializeResult(const FGorgeousAutoReplicationRPCResult& Result, FGorgeousAutoReplicationSerializedRPCResult& OutSerialized) const;
};
