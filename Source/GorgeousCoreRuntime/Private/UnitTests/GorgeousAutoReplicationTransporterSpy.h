// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationRPCTransporter.h"
#include "GorgeousAutoReplicationTransporterSpy.generated.h"

UCLASS()
class UGorgeousAutoReplicationTransporterSpy : public UGorgeousAutoReplicationRPCTransporter
{
	GENERATED_BODY()

public:
	void ResetFlags()
	{
		bServerRouted = false;
		bClientRouted = false;
		bMulticastRouted = false;
	}

	bool bServerRouted = false;
	bool bClientRouted = false;
	bool bMulticastRouted = false;

protected:
	virtual bool RouteServerBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable) override
	{
		bServerRouted = true;
		return true;
	}

	virtual bool RouteClientBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable) override
	{
		bClientRouted = true;
		return true;
	}

	virtual bool RouteMulticastRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable) override
	{
		bMulticastRouted = true;
		return true;
	}
};
