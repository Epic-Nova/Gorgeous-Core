// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|         Gorgeous Core - AutoReplication Stats Collector (Runtime)         |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "AutoReplication/Tracking/GorgeousAutoReplicationStatsCollector.h"

// ════════════════════════════════════════════════════════════════════════════
//  Static state
// ════════════════════════════════════════════════════════════════════════════

bool FGorgeousAutoReplicationStatsCollector::bIsEnabled = false;

void FGorgeousAutoReplicationStatsCollector::SetEnabled(const bool bNewEnabled)
{
	bIsEnabled = bNewEnabled;
}

FGorgeousAutoReplicationStatsCollector& FGorgeousAutoReplicationStatsCollector::Get()
{
	static FGorgeousAutoReplicationStatsCollector Instance;
	return Instance;
}

// ════════════════════════════════════════════════════════════════════════════
//  Recording
// ════════════════════════════════════════════════════════════════════════════

void FGorgeousAutoReplicationStatsCollector::RecordPropertySync(
	const FGuid& VariableId, const FString& DisplayLabel, int32 BytesSent)
{
	FScopeLock ScopedLock(&Lock);
	FlushIfNeeded();

	CurrentAccumulator.BytesSent += BytesSent;
	CurrentAccumulator.PropertySyncCount += 1;

	FGorgeousVariableNetworkStats& VarStats = PerVariableStats.FindOrAdd(VariableId);
	if (VarStats.DisplayLabel.IsEmpty())
	{
		VarStats.DisplayLabel = DisplayLabel;
		VarStats.FirstSeenSec = FPlatformTime::Seconds();
	}
	VarStats.TotalBytesSent += BytesSent;
	VarStats.TotalSyncs += 1;
	VarStats.LastSeenSec = FPlatformTime::Seconds();
}

void FGorgeousAutoReplicationStatsCollector::RecordRPCSent(
	const FGuid& VariableId, const FString& DisplayLabel, int32 PayloadBytes)
{
	FScopeLock ScopedLock(&Lock);
	FlushIfNeeded();

	CurrentAccumulator.BytesSent += PayloadBytes;
	CurrentAccumulator.RPCSentCount += 1;

	FGorgeousVariableNetworkStats& VarStats = PerVariableStats.FindOrAdd(VariableId);
	if (VarStats.DisplayLabel.IsEmpty())
	{
		VarStats.DisplayLabel = DisplayLabel;
		VarStats.FirstSeenSec = FPlatformTime::Seconds();
	}
	VarStats.TotalBytesSent += PayloadBytes;
	VarStats.TotalRPCsSent += 1;
	VarStats.LastSeenSec = FPlatformTime::Seconds();
}

void FGorgeousAutoReplicationStatsCollector::RecordRPCReceived(
	const FGuid& VariableId, const FString& DisplayLabel, int32 PayloadBytes)
{
	FScopeLock ScopedLock(&Lock);
	FlushIfNeeded();

	CurrentAccumulator.BytesReceived += PayloadBytes;
	CurrentAccumulator.RPCReceivedCount += 1;

	FGorgeousVariableNetworkStats& VarStats = PerVariableStats.FindOrAdd(VariableId);
	if (VarStats.DisplayLabel.IsEmpty())
	{
		VarStats.DisplayLabel = DisplayLabel;
		VarStats.FirstSeenSec = FPlatformTime::Seconds();
	}
	VarStats.TotalBytesReceived += PayloadBytes;
	VarStats.TotalRPCsReceived += 1;
	VarStats.LastSeenSec = FPlatformTime::Seconds();
}

// ════════════════════════════════════════════════════════════════════════════
//  Queries
// ════════════════════════════════════════════════════════════════════════════

FGorgeousAutoReplicationStatsSnapshot FGorgeousAutoReplicationStatsCollector::GetCurrentStats() const
{
	FScopeLock ScopedLock(&Lock);

	FGorgeousAutoReplicationStatsSnapshot Snapshot;
	Snapshot.TimestampSec = FPlatformTime::Seconds();

	for (const FGorgeousNetworkSample& Sample : Samples)
	{
		Snapshot.TotalBytesSent += Sample.BytesSent;
		Snapshot.TotalBytesReceived += Sample.BytesReceived;
		Snapshot.TotalPropertySyncs += Sample.PropertySyncCount;
		Snapshot.TotalRPCsSent += Sample.RPCSentCount;
		Snapshot.TotalRPCsReceived += Sample.RPCReceivedCount;
	}

	// Include the current in-progress accumulator
	Snapshot.TotalBytesSent += CurrentAccumulator.BytesSent;
	Snapshot.TotalBytesReceived += CurrentAccumulator.BytesReceived;
	Snapshot.TotalPropertySyncs += CurrentAccumulator.PropertySyncCount;
	Snapshot.TotalRPCsSent += CurrentAccumulator.RPCSentCount;
	Snapshot.TotalRPCsReceived += CurrentAccumulator.RPCReceivedCount;

	// Average bandwidth over the last 10 seconds
	const double Now = FPlatformTime::Seconds();
	const double WindowStart = Now - 10.0;
	int64 WindowBytes = 0;
	double WindowDuration = 0.0;

	for (const FGorgeousNetworkSample& Sample : Samples)
	{
		if (Sample.TimestampSec >= WindowStart)
		{
			WindowBytes += Sample.BytesSent + Sample.BytesReceived;
			WindowDuration = FMath::Max(WindowDuration, Now - Sample.TimestampSec);
		}
	}

	if (WindowDuration > 0.0)
	{
		Snapshot.AverageBandwidthKBps = static_cast<double>(WindowBytes) / 1024.0 / WindowDuration;
	}

	return Snapshot;
}

TArray<FGorgeousVariableNetworkStats> FGorgeousAutoReplicationStatsCollector::GetTopBandwidthConsumers(int32 TopN) const
{
	FScopeLock ScopedLock(&Lock);

	TArray<FGorgeousVariableNetworkStats> Result;
	Result.Reserve(PerVariableStats.Num());

	for (const auto& Pair : PerVariableStats)
	{
		Result.Add(Pair.Value);
	}

	Result.Sort([](const FGorgeousVariableNetworkStats& A, const FGorgeousVariableNetworkStats& B)
	{
		return (A.TotalBytesSent + A.TotalBytesReceived) > (B.TotalBytesSent + B.TotalBytesReceived);
	});

	if (Result.Num() > TopN)
	{
		Result.SetNum(TopN);
	}

	return Result;
}

TArray<FGorgeousNetworkSample> FGorgeousAutoReplicationStatsCollector::GetSampleHistory() const
{
	FScopeLock ScopedLock(&Lock);
	return Samples;
}

void FGorgeousAutoReplicationStatsCollector::Reset()
{
	FScopeLock ScopedLock(&Lock);
	Samples.Reset();
	PerVariableStats.Reset();
	CurrentAccumulator = FGorgeousNetworkSample();
	LastFlushTime = 0.0;
}

// ════════════════════════════════════════════════════════════════════════════
//  Internal
// ════════════════════════════════════════════════════════════════════════════

void FGorgeousAutoReplicationStatsCollector::FlushIfNeeded()
{
	// Caller must hold Lock
	const double Now = FPlatformTime::Seconds();

	if (LastFlushTime == 0.0)
	{
		LastFlushTime = Now;
		return;
	}

	if ((Now - LastFlushTime) < SampleIntervalSec)
	{
		return;
	}

	CurrentAccumulator.TimestampSec = Now;

	if (Samples.Num() >= MaxSamples)
	{
		Samples.RemoveAt(0, 1, EAllowShrinking::No);
	}

	Samples.Add(MoveTemp(CurrentAccumulator));
	CurrentAccumulator = FGorgeousNetworkSample();
	LastFlushTime = Now;
}
