// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Core - RPC Debug Tracker (Runtime)                  |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"

/**
 * Status of a tracked RPC request throughout its lifecycle.
 */
enum class EGorgeousRPCDebugStatus : uint8
{
	Pending,
	Complete,
	Failed,
	TimedOut,
	PartialSuccess  // Some responders replied, others timed out
};

/**
 * Snapshot of a single responder's result within a tracked RPC.
 */
struct GORGEOUSCORERUNTIME_API FGorgeousRPCResponseSnapshot
{
	/** Stable key identifying the responder (Server / ConnectionKey). */
	FString ResponderKey;

	/** Friendly label for display. */
	FString ResponderDisplayName;

	/** True if this response came from the authority role. */
	bool bIsServer = false;

	/** Time at which the response arrived. */
	double ReceivedTimeSec = 0.0;

	/** Latency from RPC initiation to receipt of this response. */
	double LatencyMs = 0.0;

	/** True when a return value has been captured for this response. */
	bool bHasReturnValue = false;

	/** Short preview string of the return value (when available). */
	FString ReturnValuePreview;

	/** Latest readiness state signalled by the responder via MarkAutoReplicationRPCResponderReady. */
	EGorgeousRPCReadyState ReadyState = EGorgeousRPCReadyState::Ready;

	/**
	 * Phase label for deferred responses, used to distinguish the two timeline entries
	 * produced per deferred responder:
	 *   "Prepared" — container created, handler hasn't signalled readiness yet (OV empty)
	 *   "Ready"    — handler called MarkAutoReplicationRPCResponderReady (OV populated)
	 * Empty for non-deferred (immediate) responses.
	 */
	FString Phase;
};

/**
 * Full debug snapshot of a single RPC lifecycle.
 */
struct GORGEOUSCORERUNTIME_API FGorgeousRPCDebugEntry
{
	/** Correlation GUID assigned when the RPC was queued. */
	FGuid RequestGuid;

	/** Entry key the RPC was dispatched under. */
	FName Key;

	/** Handler function name from the payload. */
	FName HandlerName;

	/** RPC type (Reliable Server, Unreliable Multicast, etc.). */
	EGorgeousAutoReplicationRPCType Type = EGorgeousAutoReplicationRPCType::EReliableServer;

	/** Direction description string (e.g., "->S", "<-M"). */
	FString DirectionLabel;

	/** Current status of the RPC. */
	EGorgeousRPCDebugStatus Status = EGorgeousRPCDebugStatus::Pending;

	/** Wall-clock time at which the RPC was initiated. */
	double InitiatedTimeSec = 0.0;

	/** Wall-clock time at which all expected responses completed (or the RPC timed out). */
	double CompletedTimeSec = 0.0;

	/** Description of the initiator (actor/controller name). */
	FString InitiatorDescription;

	/** Argument names captured at initiation. */
	TArray<FString> ArgumentNames;

	/** Individual responder results received so far. */
	TArray<FGorgeousRPCResponseSnapshot> Responses;

	/** Expected number of responses (-1 if unknown / broadcast). */
	int32 ExpectedResponseCount = -1;

	/** Convenience: how many responses have arrived. */
	int32 GetReceivedResponseCount() const { return Responses.Num(); }

	/** True when all expected responses have been received and all are ready. */
	bool IsFullyResolved() const
	{
		if (ExpectedResponseCount <= 0 || Responses.Num() < ExpectedResponseCount)
		{
			return false;
		}
		// All responses must have reached the Ready state (i.e. not deferred/pending)
		for (const FGorgeousRPCResponseSnapshot& Resp : Responses)
		{
			if (Resp.ReadyState != EGorgeousRPCReadyState::Ready)
			{
				return false;
			}
		}
		return true;
	}
};

/**
 * Lazy singleton that tracks live RPC lifecycles for debug tooling.
 *
 * **Performance contract**: a static `bIsEnabled` flag is checked inline before
 * any allocation or map lookup. When disabled (the default) the overhead is a
 * single branch per RPC dispatch — effectively zero-cost in production runtimes.
 *
 * Enable/disable is driven by opening/closing the RPC Inspector window.
 */
class GORGEOUSCORERUNTIME_API FGorgeousRPCDebugTracker
{
public:
	// ── Enable / Disable ────────────────────────────────────────────────
	static bool IsEnabled() { return bIsEnabled; }
	static void SetEnabled(bool bNewEnabled);

	// ── Singleton access ────────────────────────────────────────────────
	static FGorgeousRPCDebugTracker& Get();

	// ── Recording (called from Mixin / Transporter hooks) ───────────────

	/** Called when an RPC is first queued via RequestRPC. */
	void OnRPCInitiated(const FGuid& RequestGuid, FName Key, FName HandlerName,
		EGorgeousAutoReplicationRPCType Type, const TArray<FGorgeousRPCArgumentContainer>& Arguments,
		const FString& InitiatorDescription);

	/** Called when an RPC has been relayed to the server via a relay component. */
	void OnRPCRelayed(const FGuid& RequestGuid);

	/** Called when an RPC has been executed on a target (variable/owner/component). */
	void OnRPCExecuted(const FGuid& RequestGuid, const FGorgeousAutoReplicationRPCResponderHandle& Responder,
		const FString& ReturnValuePreview, bool bIsDeferred = false);

	/** Mark an RPC as fully completed. */
	void OnRPCCompleted(const FGuid& RequestGuid);

	/** Mark an RPC as timed out. */
	void OnRPCTimedOut(const FGuid& RequestGuid);

	/**
	 * Update the readiness state of a specific responder snapshot.
	 * Called when a handler invokes MarkAutoReplicationRPCResponderReady().
	 */
	void OnRPCResponderReadyStateChanged(const FGuid& RequestGuid, const FString& ResponderKey, EGorgeousRPCReadyState ReadyState);

	// ── Queries (called from RPC Inspector UI) ──────────────────────────

	/** Returns recent RPCs ordered newest-first (capped at Count). */
	TArray<FGorgeousRPCDebugEntry> GetRecentRPCs(int32 Count = 200) const;

	/** Returns only pending (in-flight) RPCs ordered by initiation time. */
	TArray<FGorgeousRPCDebugEntry> GetPendingRPCs() const;

	/** Finds a specific entry by request GUID. */
	const FGorgeousRPCDebugEntry* FindByGuid(const FGuid& Guid) const;

	/** Clears all tracked entries. */
	void Reset();

	/** Sweeps entries that have been pending longer than TimeoutSec. */
	void SweepTimedOut(double TimeoutSec = 30.0);

private:
	FGorgeousRPCDebugTracker() = default;

	static bool bIsEnabled;

	/** Maximum tracked entries before the oldest is evicted. */
	static constexpr int32 MaxTrackedEntries = 2000;

	mutable FCriticalSection Lock;
	TArray<FGorgeousRPCDebugEntry> Entries;
	TMap<FGuid, int32> GuidToIndex;

	/** Evicts oldest entry when the ring buffer overflows. */
	void EvictOldestIfNeeded();

	/** Rebuilds the GUID->index map after an eviction or reset. */
	void RebuildIndexMap();

	/** Resolves a direction label from the RPC type. */
	static FString MakeDirectionLabel(EGorgeousAutoReplicationRPCType Type);
};
