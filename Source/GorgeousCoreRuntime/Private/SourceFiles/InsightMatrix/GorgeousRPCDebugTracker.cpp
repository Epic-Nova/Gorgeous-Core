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

#include "InsightMatrix/GorgeousRPCDebugTracker.h"

// ════════════════════════════════════════════════════════════════════════════
//  Static state
// ════════════════════════════════════════════════════════════════════════════

bool FGorgeousRPCDebugTracker::bIsEnabled = false;

void FGorgeousRPCDebugTracker::SetEnabled(const bool bNewEnabled)
{
	bIsEnabled = bNewEnabled;

	if (!bNewEnabled)
	{
		// When disabled, keep data alive so the window can still browse history.
		// Data is only cleared on explicit Reset().
	}
}

FGorgeousRPCDebugTracker& FGorgeousRPCDebugTracker::Get()
{
	static FGorgeousRPCDebugTracker Instance;
	return Instance;
}

// ════════════════════════════════════════════════════════════════════════════
//  Recording
// ════════════════════════════════════════════════════════════════════════════

void FGorgeousRPCDebugTracker::OnRPCInitiated(
	const FGuid& RequestGuid, FName Key, FName HandlerName,
	EGorgeousAutoReplicationRPCType Type,
	const TArray<FGorgeousRPCArgumentContainer>& Arguments,
	const FString& InitiatorDescription)
{
	FScopeLock ScopedLock(&Lock);

	// ── De-duplication ──────────────────────────────────────────────────
	// Multicast and relay flows can cause the same GUID to hit OnRPCInitiated
	// more than once (client dispatches → relay → server re-dispatches with
	// the same GUID).  When the GUID already exists, merge instead of adding
	// a second orphaned entry.
	if (const int32* ExistingIdx = GuidToIndex.Find(RequestGuid))
	{
		if (Entries.IsValidIndex(*ExistingIdx))
		{
			// Entry already exists — nothing to do.
			return;
		}
	}

	EvictOldestIfNeeded();

	FGorgeousRPCDebugEntry Entry;
	Entry.RequestGuid = RequestGuid;
	Entry.Key = Key;
	Entry.HandlerName = HandlerName;
	Entry.Type = Type;
	Entry.DirectionLabel = MakeDirectionLabel(Type);
	Entry.Status = EGorgeousRPCDebugStatus::Pending;
	Entry.InitiatedTimeSec = FPlatformTime::Seconds();
	Entry.InitiatorDescription = InitiatorDescription;

	for (const FGorgeousRPCArgumentContainer& Arg : Arguments)
	{
		Entry.ArgumentNames.Add(Arg.ArgumentName.ToString());
	}

	// Server-bound RPCs expect exactly 1 response (the server).
	// Client-bound RPCs expect 1 response (the targeted client).
	// Multicast RPCs have an unknown response count (set to -1).
	switch (Type)
	{
	case EGorgeousAutoReplicationRPCType::EReliableServer:
	case EGorgeousAutoReplicationRPCType::EUnreliableServer:
	case EGorgeousAutoReplicationRPCType::EReliableClient:
	case EGorgeousAutoReplicationRPCType::EUnreliableClient:
		Entry.ExpectedResponseCount = 1;
		break;
	default:
		Entry.ExpectedResponseCount = -1; // Multicast: unknown
		break;
	}

	const int32 Index = Entries.Add(MoveTemp(Entry));
	GuidToIndex.Add(RequestGuid, Index);
}

void FGorgeousRPCDebugTracker::OnRPCRelayed(const FGuid& RequestGuid)
{
	FScopeLock ScopedLock(&Lock);

	if (const int32* IndexPtr = GuidToIndex.Find(RequestGuid))
	{
		// Mark as still pending but record that it was relayed (no state change needed,
		// the entry was already created in OnRPCInitiated).
	}
}

void FGorgeousRPCDebugTracker::OnRPCExecuted(
	const FGuid& RequestGuid,
	const FGorgeousAutoReplicationRPCResponderHandle& Responder,
	const FString& ReturnValuePreview,
	bool bIsDeferred)
{
	FScopeLock ScopedLock(&Lock);

	const int32* IndexPtr = GuidToIndex.Find(RequestGuid);
	if (!IndexPtr || !Entries.IsValidIndex(*IndexPtr))
	{
		return;
	}

	FGorgeousRPCDebugEntry& Entry = Entries[*IndexPtr];

	FGorgeousRPCResponseSnapshot Snapshot;
	Snapshot.ResponderKey = Responder.GetStableKey();
	Snapshot.ResponderDisplayName = Responder.PlayerDisplayName.IsEmpty()
		? Responder.GetStableKey()
		: Responder.PlayerDisplayName;
	Snapshot.bIsServer = Responder.bIsServer;
	Snapshot.ReceivedTimeSec = FPlatformTime::Seconds();
	Snapshot.LatencyMs = (Snapshot.ReceivedTimeSec - Entry.InitiatedTimeSec) * 1000.0;
	Snapshot.bHasReturnValue = !ReturnValuePreview.IsEmpty();
	Snapshot.ReturnValuePreview = ReturnValuePreview;

	// Determine whether a snapshot for this responder already exists.
	// If it does, this call is promoting a deferred entry to "Ready".
	// If it doesn't and bIsDeferred is true, this is the initial "Prepared" snapshot.
	const bool bAlreadyHasSnapshot = Entry.Responses.ContainsByPredicate(
		[&](const FGorgeousRPCResponseSnapshot& S) { return S.ResponderKey == Snapshot.ResponderKey; });

	if (bIsDeferred)
	{
		if (!bAlreadyHasSnapshot)
		{
			Snapshot.Phase = TEXT("Prepared");
			// Start with NotReadyToCollect so IsFullyResolved() does not immediately
			// auto-complete the entry before the handler signals readiness.
			Snapshot.ReadyState = EGorgeousRPCReadyState::NotReadyToCollect;
			Entry.Responses.Add(MoveTemp(Snapshot));
		}
		// else: a PendingReadySignals promotion already added a non-deferred Ready
		// snapshot for this responder before EmitResult's tracker call ran.
		// Avoid replacing the Ready entry with a Prepared/NotReadyToCollect duplicate.
	}
	else if (bAlreadyHasSnapshot)
	{
		// Either a deferred → Ready promotion or a duplicate non-deferred dispatch
		// (e.g. the server-side relay loop dispatching the same multicast N times).
		// Update the existing snapshot in-place instead of appending a new row so the
		// timeline never shows duplicate "Server" entries for the same responder key.
		for (FGorgeousRPCResponseSnapshot& Existing : Entry.Responses)
		{
			if (Existing.ResponderKey == Snapshot.ResponderKey)
			{
				// Only promote a Prepared (deferred) snapshot; ignore plain duplicates.
				if (Existing.Phase == TEXT("Prepared"))
				{
					Existing.Phase             = TEXT("Ready");
					Existing.bHasReturnValue   = Snapshot.bHasReturnValue;
					Existing.ReturnValuePreview = Snapshot.ReturnValuePreview;
					Existing.ReadyState        = EGorgeousRPCReadyState::Ready;
				}
				// Non-deferred duplicate: silently discard — keeps the timeline clean.
				break;
			}
		}
	}
	else
	{
		// Fresh non-deferred snapshot — no snapshot for this responder yet.
		Entry.Responses.Add(MoveTemp(Snapshot));
	}

	// Auto-complete when all expected responses have arrived
	if (Entry.IsFullyResolved() && Entry.Status == EGorgeousRPCDebugStatus::Pending)
	{
		Entry.Status = EGorgeousRPCDebugStatus::Complete;
		Entry.CompletedTimeSec = FPlatformTime::Seconds();
	}
}

void FGorgeousRPCDebugTracker::OnRPCCompleted(const FGuid& RequestGuid)
{
	FScopeLock ScopedLock(&Lock);

	const int32* IndexPtr = GuidToIndex.Find(RequestGuid);
	if (!IndexPtr || !Entries.IsValidIndex(*IndexPtr))
	{
		return;
	}

	FGorgeousRPCDebugEntry& Entry = Entries[*IndexPtr];
	// Also promote PartialSuccess → Complete: the async action completed normally but
	// the debug sweep may have fired before the final relay result arrived, marking
	// the entry as partial in the meantime. OnRPCCompleted is the authoritative signal.
	if (Entry.Status == EGorgeousRPCDebugStatus::Pending ||
		Entry.Status == EGorgeousRPCDebugStatus::PartialSuccess)
	{
		// For broadcast / multicast RPCs ExpectedResponseCount may still be -1 because
		// it was never set explicitly. Fix it now with the actual received count so the
		// status badge shows a real number instead of "-1".
		if (Entry.ExpectedResponseCount <= 0)
		{
			Entry.ExpectedResponseCount = Entry.Responses.Num();
		}
		Entry.Status = EGorgeousRPCDebugStatus::Complete;
		Entry.CompletedTimeSec = FPlatformTime::Seconds();
	}
}

void FGorgeousRPCDebugTracker::OnRPCTimedOut(const FGuid& RequestGuid)
{
	FScopeLock ScopedLock(&Lock);

	const int32* IndexPtr = GuidToIndex.Find(RequestGuid);
	if (!IndexPtr || !Entries.IsValidIndex(*IndexPtr))
	{
		return;
	}

	FGorgeousRPCDebugEntry& Entry = Entries[*IndexPtr];
	if (Entry.Status == EGorgeousRPCDebugStatus::Pending)
	{
		Entry.Status = EGorgeousRPCDebugStatus::TimedOut;
		Entry.CompletedTimeSec = FPlatformTime::Seconds();
	}
}

void FGorgeousRPCDebugTracker::OnRPCResponderReadyStateChanged(
	const FGuid& RequestGuid, const FString& ResponderKey, EGorgeousRPCReadyState ReadyState)
{
	FScopeLock ScopedLock(&Lock);

	const int32* IndexPtr = GuidToIndex.Find(RequestGuid);
	if (!IndexPtr || !Entries.IsValidIndex(*IndexPtr))
	{
		return;
	}

	FGorgeousRPCDebugEntry& Entry = Entries[*IndexPtr];

	// Update the matching responder snapshot's ready state.
	// When the key is empty (context could not be resolved) update all snapshots.
	for (FGorgeousRPCResponseSnapshot& Snapshot : Entry.Responses)
	{
		if (ResponderKey.IsEmpty() || Snapshot.ResponderKey == ResponderKey)
		{
			Snapshot.ReadyState = ReadyState;
			if (!ResponderKey.IsEmpty())
			{
				break; // Found the specific responder — stop.
			}
		}
	}
}

// ════════════════════════════════════════════════════════════════════════════
//  Queries
// ════════════════════════════════════════════════════════════════════════════

TArray<FGorgeousRPCDebugEntry> FGorgeousRPCDebugTracker::GetRecentRPCs(int32 Count) const
{
	FScopeLock ScopedLock(&Lock);

	const int32 StartIndex = FMath::Max(0, Entries.Num() - Count);
	TArray<FGorgeousRPCDebugEntry> Result;
	Result.Reserve(Entries.Num() - StartIndex);

	// Newest first
	for (int32 i = Entries.Num() - 1; i >= StartIndex; --i)
	{
		Result.Add(Entries[i]);
	}
	return Result;
}

TArray<FGorgeousRPCDebugEntry> FGorgeousRPCDebugTracker::GetPendingRPCs() const
{
	FScopeLock ScopedLock(&Lock);

	TArray<FGorgeousRPCDebugEntry> Result;
	for (const FGorgeousRPCDebugEntry& Entry : Entries)
	{
		if (Entry.Status == EGorgeousRPCDebugStatus::Pending)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

const FGorgeousRPCDebugEntry* FGorgeousRPCDebugTracker::FindByGuid(const FGuid& Guid) const
{
	FScopeLock ScopedLock(&Lock);

	const int32* IndexPtr = GuidToIndex.Find(Guid);
	if (IndexPtr && Entries.IsValidIndex(*IndexPtr))
	{
		return &Entries[*IndexPtr];
	}
	return nullptr;
}

void FGorgeousRPCDebugTracker::Reset()
{
	FScopeLock ScopedLock(&Lock);
	Entries.Reset();
	GuidToIndex.Reset();
}

void FGorgeousRPCDebugTracker::SweepTimedOut(double TimeoutSec)
{
	FScopeLock ScopedLock(&Lock);

	const double Now = FPlatformTime::Seconds();
	bool bAnyChanged = false;

	for (FGorgeousRPCDebugEntry& Entry : Entries)
	{
		if (Entry.Status == EGorgeousRPCDebugStatus::Pending &&
			(Now - Entry.InitiatedTimeSec) > TimeoutSec)
		{
			// If some responses were already received, it's a partial success.
			Entry.Status = (Entry.Responses.Num() > 0)
				? EGorgeousRPCDebugStatus::PartialSuccess
				: EGorgeousRPCDebugStatus::TimedOut;
			Entry.CompletedTimeSec = Now;
			bAnyChanged = true;
		}
	}
}

// ════════════════════════════════════════════════════════════════════════════
//  Internal helpers
// ════════════════════════════════════════════════════════════════════════════

void FGorgeousRPCDebugTracker::EvictOldestIfNeeded()
{
	// Caller must hold Lock
	while (Entries.Num() >= MaxTrackedEntries)
	{
		Entries.RemoveAt(0, 1, EAllowShrinking::No);
		RebuildIndexMap();
	}
}

void FGorgeousRPCDebugTracker::RebuildIndexMap()
{
	// Caller must hold Lock
	GuidToIndex.Reset();
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		GuidToIndex.Add(Entries[i].RequestGuid, i);
	}
}

FString FGorgeousRPCDebugTracker::MakeDirectionLabel(EGorgeousAutoReplicationRPCType Type)
{
	switch (Type)
	{
	case EGorgeousAutoReplicationRPCType::EReliableServer:
	case EGorgeousAutoReplicationRPCType::EUnreliableServer:
		return TEXT("\x2192") TEXT("S"); // →S
	case EGorgeousAutoReplicationRPCType::EReliableClient:
	case EGorgeousAutoReplicationRPCType::EUnreliableClient:
		return TEXT("\x2190") TEXT("C"); // ←C
	case EGorgeousAutoReplicationRPCType::EReliableMulticast:
	case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
		return TEXT("\x2194") TEXT("M"); // ↔M
	default:
		return TEXT("?");
	}
}
