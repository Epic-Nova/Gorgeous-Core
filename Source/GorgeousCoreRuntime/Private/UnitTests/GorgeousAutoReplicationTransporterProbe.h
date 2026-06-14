// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - AutoReplication Transporter Probe           |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

/**
 * Enhanced test double that records RPC execution origin, destination, and
 * reliability for every routed call.  Unlike the lightweight TransporterSpy
 * this probe captures a full trace log — each entry records WHAT happened,
 * WHERE the call started (server/client), and WHERE it was dispatched plus
 * whether it was reliable or unreliable.
 */

#pragma once

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationRPCTransporter.h"
#include "GorgeousAutoReplicationTransporterProbe.generated.h"

/** One captured RPC routing event. */
USTRUCT()
struct FGorgeousTransporterProbeEntry
{
	GENERATED_BODY()

	/** Key from the QueuedRPC. */
	UPROPERTY()
	FName Key;

	/** What type was requested (EReliableServer, EUnreliableMulticast etc.). */
	UPROPERTY()
	EGorgeousAutoReplicationRPCType RequestedType = EGorgeousAutoReplicationRPCType::EReliableServer;

	/** The TargetKind carried by the RPC. */
	UPROPERTY()
	EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EAuto;

	/** Which internal route was invoked: "Server", "Client", or "Multicast". */
	UPROPERTY()
	FString RoutedDirection;

	/** Was the route reliable? */
	UPROPERTY()
	bool bReliable = false;

	/** Was the caller running on the server role (HasAuthority)? */
	UPROPERTY()
	bool bCallerIsServer = false;

	/** Human-readable label of the caller endpoint. */
	UPROPERTY()
	FString CallerLabel;

	/** Wall-clock timestamp (seconds since epoch). */
	UPROPERTY()
	double TimestampSeconds = 0.0;

	/** Handler name from the payload (if any). */
	UPROPERTY()
	FName PayloadHandlerName;
};

/** Enhanced transporter probe that captures a full log of every routing event. */
UCLASS()
class UGorgeousAutoReplicationTransporterProbe : public UGorgeousAutoReplicationRPCTransporter
{
	GENERATED_BODY()

public:
	/** Full log of every routing event captured by this probe. */
	UPROPERTY()
	TArray<FGorgeousTransporterProbeEntry> RouteLog;

	/** Counters. */
	int32 ServerRouteCount = 0;
	int32 ClientRouteCount = 0;
	int32 MulticastRouteCount = 0;
	int32 ReliableCount = 0;
	int32 UnreliableCount = 0;

	void ResetProbe()
	{
		RouteLog.Reset();
		ServerRouteCount = 0;
		ClientRouteCount = 0;
		MulticastRouteCount = 0;
		ReliableCount = 0;
		UnreliableCount = 0;
	}

	/** Build a summary string describing what was routed. */
	FString BuildSummary() const
	{
		return FString::Printf(
			TEXT("Probe: Server=%d Client=%d Multicast=%d | Reliable=%d Unreliable=%d | Total=%d"),
			ServerRouteCount, ClientRouteCount, MulticastRouteCount,
			ReliableCount, UnreliableCount, RouteLog.Num());
	}

	/** Returns true if the probe captured at least one route in the given direction. */
	bool HasRouteIn(const FString& Direction) const
	{
		for (const FGorgeousTransporterProbeEntry& E : RouteLog)
		{
			if (E.RoutedDirection == Direction) return true;
		}
		return false;
	}

	/** Returns all entries for a given direction. */
	TArray<FGorgeousTransporterProbeEntry> GetEntriesForDirection(const FString& Direction) const
	{
		TArray<FGorgeousTransporterProbeEntry> Out;
		for (const FGorgeousTransporterProbeEntry& E : RouteLog)
		{
			if (E.RoutedDirection == Direction) Out.Add(E);
		}
		return Out;
	}

protected:
	virtual bool RouteServerBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable) override
	{
		RecordEntry(QueuedRPC, TEXT("Server"), bReliable);
		++ServerRouteCount;
		bReliable ? ++ReliableCount : ++UnreliableCount;
		return true;
	}

	virtual bool RouteClientBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable) override
	{
		RecordEntry(QueuedRPC, TEXT("Client"), bReliable);
		++ClientRouteCount;
		bReliable ? ++ReliableCount : ++UnreliableCount;
		return true;
	}

	virtual bool RouteMulticastRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable) override
	{
		RecordEntry(QueuedRPC, TEXT("Multicast"), bReliable);
		++MulticastRouteCount;
		bReliable ? ++ReliableCount : ++UnreliableCount;
		return true;
	}

private:
	void RecordEntry(const FGorgeousQueuedRPC& QueuedRPC, const TCHAR* Direction, bool bReliable)
	{
		FGorgeousTransporterProbeEntry Entry;
		Entry.Key = QueuedRPC.Key;
		Entry.RequestedType = QueuedRPC.Type;
		Entry.TargetKind = QueuedRPC.TargetKind;
		Entry.RoutedDirection = Direction;
		Entry.bReliable = bReliable;
		Entry.bCallerIsServer = IsRunningDedicatedServer();
		Entry.CallerLabel = IsRunningDedicatedServer() ? TEXT("DedicatedServer") : TEXT("Client");
		Entry.TimestampSeconds = FPlatformTime::Seconds();
		Entry.PayloadHandlerName = QueuedRPC.Payload.HandlerName;
		RouteLog.Add(MoveTemp(Entry));
	}
};
