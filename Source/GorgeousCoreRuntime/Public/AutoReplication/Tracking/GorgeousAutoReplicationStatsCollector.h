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
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

/**
 * Periodic bandwidth/traffic sample used by the stats collector ring buffer.
 */
struct GORGEOUSCORERUNTIME_API FGorgeousNetworkSample
{
	/** Wall-clock time the sample was taken. */
	double TimestampSec = 0.0;

	/** Bytes sent during this sample interval. */
	int64 BytesSent = 0;

	/** Bytes received during this sample interval. */
	int64 BytesReceived = 0;

	/** Number of property syncs during this interval. */
	int32 PropertySyncCount = 0;

	/** Number of RPCs dispatched during this interval. */
	int32 RPCSentCount = 0;

	/** Number of RPCs received during this interval. */
	int32 RPCReceivedCount = 0;
};

/**
 * Aggregated per-variable network stats.
 */
struct GORGEOUSCORERUNTIME_API FGorgeousVariableNetworkStats
{
	FString DisplayLabel;
	int64 TotalBytesSent = 0;
	int64 TotalBytesReceived = 0;
	int32 TotalSyncs = 0;
	int32 TotalRPCsSent = 0;
	int32 TotalRPCsReceived = 0;
	double FirstSeenSec = 0.0;
	double LastSeenSec = 0.0;
};

/**
 * Aggregate snapshot returned by GetCurrentStats().
 */
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationStatsSnapshot
{
	double TimestampSec = 0.0;
	int64 TotalBytesSent = 0;
	int64 TotalBytesReceived = 0;
	int32 TotalPropertySyncs = 0;
	int32 TotalRPCsSent = 0;
	int32 TotalRPCsReceived = 0;
	double AverageBandwidthKBps = 0.0;
};

/**
 * Lazy singleton collecting AR network traffic statistics.
 *
 * **Performance contract**: a static `bIsEnabled` flag is checked inline before
 * every record call. When disabled (default) the overhead is a single branch
 *, zero cost in production. Activation is driven by the Network Traffic
 * Inspector window lifecycle.
 */
class GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationStatsCollector
{
public:
	// ── Enable / Disable ────────────────────────────────────────────────
	static bool IsEnabled() { return bIsEnabled; }
	static void SetEnabled(bool bNewEnabled);

	// ── Singleton access ────────────────────────────────────────────────
	static FGorgeousAutoReplicationStatsCollector& Get();

	// ── Recording ───────────────────────────────────────────────────────

	/** Records a property sync event. */
	void RecordPropertySync(const FGuid& VariableId, const FString& DisplayLabel, int32 BytesSent);

	/** Records an RPC send event. */
	void RecordRPCSent(const FGuid& VariableId, const FString& DisplayLabel, int32 PayloadBytes);

	/** Records an RPC receive event. */
	void RecordRPCReceived(const FGuid& VariableId, const FString& DisplayLabel, int32 PayloadBytes);

	// ── Queries ─────────────────────────────────────────────────────────

	/** Returns an aggregate stats snapshot covering all tracked time. */
	FGorgeousAutoReplicationStatsSnapshot GetCurrentStats() const;

	/** Returns per-variable stats sorted by total bytes (descending). */
	TArray<FGorgeousVariableNetworkStats> GetTopBandwidthConsumers(int32 TopN = 20) const;

	/** Returns the full ring buffer for charting. */
	TArray<FGorgeousNetworkSample> GetSampleHistory() const;

	/** Clears all stats. */
	void Reset();

private:
	FGorgeousAutoReplicationStatsCollector() = default;

	static bool bIsEnabled;

	static constexpr int32 MaxSamples = 600; // ~10min at 1 sample/sec
	static constexpr double SampleIntervalSec = 1.0;

	mutable FCriticalSection Lock;

	TArray<FGorgeousNetworkSample> Samples;
	TMap<FGuid, FGorgeousVariableNetworkStats> PerVariableStats;

	// Accumulator for the current sample interval
	FGorgeousNetworkSample CurrentAccumulator;
	double LastFlushTime = 0.0;

	/** Flushes the accumulator to the ring buffer if the interval has elapsed. */
	void FlushIfNeeded();
};