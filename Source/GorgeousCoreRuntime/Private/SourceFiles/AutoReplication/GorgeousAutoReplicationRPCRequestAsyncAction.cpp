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
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "QualityOfLife/GorgeousGameInstance.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "InsightMatrix/GorgeousRPCDebugTracker.h"
#include "Algo/Sort.h"
#include "Engine/World.h"

TMap<FGuid, UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState> UGorgeousAutoReplicationRPCRequestAsyncAction::PendingRequests;
TMap<FGuid, TMap<FString, FGorgeousAutoReplicationRPCResult>> UGorgeousAutoReplicationRPCRequestAsyncAction::ClientDeferredResultCache;

UGorgeousAutoReplicationRPCRequestAsyncAction* UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(UObject* WorldContextObject, FName Key, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, const EGorgeousAutoReplicationTargetKind TargetKind, AActor* OptionalTarget)
{
	UGorgeousAutoReplicationRPCRequestAsyncAction* Action = NewObject<UGorgeousAutoReplicationRPCRequestAsyncAction>();
	Action->RequestKey = Key;
	Action->RequestType = Type;
	Action->RequestPayload = Payload;
	Action->RequestTargetKind = TargetKind;
	
	// Auto-resolve owner from world context
	UObject* AutoReplicationOwner = nullptr;
	
	if (OptionalTarget)
	{
		AutoReplicationOwner = OptionalTarget;
	}
	
	if (WorldContextObject && !AutoReplicationOwner)
	{
		// Try to resolve from common patterns
		if (APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			AutoReplicationOwner = PC;
		}
		else if (APlayerState* PS = Cast<APlayerState>(WorldContextObject))
		{
			// PlayerState - try to get the owning player controller
			AutoReplicationOwner = PS->GetOwningController();
			if (!AutoReplicationOwner)
			{
				AutoReplicationOwner = PS->GetPlayerController();
			}
			if (!AutoReplicationOwner)
			{
				AutoReplicationOwner = PS;
			}
		}
		else if (APawn* Pawn = Cast<APawn>(WorldContextObject))
		{
			AutoReplicationOwner = Pawn->GetController();
			if (!AutoReplicationOwner)
			{
				AutoReplicationOwner = Pawn;
			}
		}
		else if (UActorComponent* Component = Cast<UActorComponent>(WorldContextObject))
		{
			if (AActor* Owner = Component->GetOwner())
			{
				// If owner is a Pawn, get its controller
				if (APawn* OwnerPawn = Cast<APawn>(Owner))
				{
					AutoReplicationOwner = OwnerPawn->GetController();
				}
				// If owner is a PlayerState, get its controller
				else if (APlayerState* OwnerPS = Cast<APlayerState>(Owner))
				{
					AutoReplicationOwner = OwnerPS->GetOwningController();
					if (!AutoReplicationOwner)
					{
						AutoReplicationOwner = OwnerPS->GetPlayerController();
					}
				}
				if (!AutoReplicationOwner)
				{
					AutoReplicationOwner = Owner;
				}
			}
			if (!AutoReplicationOwner)
			{
				AutoReplicationOwner = Component;
			}
		}
		else
		{
			// Use the world context object directly
			AutoReplicationOwner = WorldContextObject;
		}
		
		// Final fallback: try to get local player controller from world
		if (!AutoReplicationOwner)
		{
			if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
			{
				AutoReplicationOwner = World->GetFirstPlayerController();
			}
		}
	}

	Action->WeakContext = AutoReplicationOwner;
	Action->ResultContainer = nullptr;
	Action->CachedResults.Reset();
	Action->CachedResultMap.Reset();
	Action->bActivated = false;
	Action->InternalResultContainer = nullptr;
	return Action;
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::Activate()
{
	if (bActivated)
	{
		return;
	}
	bActivated = true;

	UObject* Context = WeakContext.Get();
	if (!Context)
	{
		FailRequest();
		return;
	}

	FGorgeousAutoReplicationMixin* AutoReplicationMixin = ResolveAutoReplicationMixin(Context);
	if (!AutoReplicationMixin || !AutoReplicationMixin->IsNetworkingEnabled())
	{
		FailRequest();
		return;
	}

	RequestGuid = FGuid::NewGuid();
	RegisterPendingRequest();

	if (!AutoReplicationMixin->RequestRPC(RequestKey, RequestType, RequestPayload, RequestTargetKind, &RequestGuid))
	{
		FailRequest();
		return;
	}

	// Start the per-request timeout timer if a non-zero timeout was specified in the payload.
	if (RequestPayload.TimeoutSeconds > KINDA_SMALL_NUMBER)
	{
		if (UWorld* World = Context->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				TimeoutHandle, this,
				&UGorgeousAutoReplicationRPCRequestAsyncAction::OnTimeout,
				RequestPayload.TimeoutSeconds, false);
		}
	}
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyRequestCompleted(const FGorgeousAutoReplicationRPCResult& Result)
{
	if (!Result.QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(Result.QueuedRPC.RequestGuid);
	if (!PendingState || PendingState->bCompleted)
	{
		// No local pending state — this machine processed an RPC that was initiated on a
		// different (client) machine. Relay the result back to the originating client so
		// its async action can complete normally.
		UGorgeousAutoReplicationRPCRelayComponent::TryRelayResultToClientInitiator(Result);
		return;
	}

	if (!PendingState->Action.IsValid())
	{
		PendingRequests.Remove(Result.QueuedRPC.RequestGuid);
		return;
	}

	FGorgeousAutoReplicationRPCResult ResultCopy = Result;
	if (UGorgeousAutoReplicationRPCRequestAsyncAction* Action = PendingState->Action.Get())
	{
		Action->ApplyRequestMetadata(ResultCopy);
		Action->ResolveResultPointers(ResultCopy);
	}

	const FString ResponderKey = BuildResponderKey(ResultCopy.Responder);
	PendingState->CollectedResults.Add(ResponderKey, ResultCopy);
	// Remove any deferred entry that was injected by NotifyDeferredSingleResponderCallback
	// (ReadyForSingleResponderCallback relay path) — it's now promoted to CollectedResults.
	PendingState->DeferredResults.Remove(ResponderKey);
	if (!ResponderKey.IsEmpty())
	{
		PendingState->ExpectedResponders.Add(ResponderKey);
	}

	// If this result came from a remote responder (relayed from a client) the debug
	// tracker on this machine may never have received an OnRPCExecuted call for it.
	// Add a snapshot now so the server-side RPC Inspector shows all responders.
	if (FGorgeousRPCDebugTracker::IsEnabled() && Result.QueuedRPC.RequestGuid.IsValid())
	{
		// Only inject a snapshot if we don't already have one for this responder key —
		// local (same-machine) handlers already called OnRPCExecuted directly.
		bool bAlreadyTracked = false;
		if (const FGorgeousRPCDebugEntry* TrackerEntry =
			FGorgeousRPCDebugTracker::Get().FindByGuid(Result.QueuedRPC.RequestGuid))
		{
			bAlreadyTracked = TrackerEntry->Responses.ContainsByPredicate(
				[&](const FGorgeousRPCResponseSnapshot& S) { return S.ResponderKey == ResponderKey; });
		}

		if (!bAlreadyTracked)
		{
			// Build OV preview from the relayed result
			FString ReturnPreview;
			UGorgeousObjectVariable* OV = ResultCopy.TargetVariable;
			if (UGorgeousRPC_OV* Container = Cast<UGorgeousRPC_OV>(OV))
			{
				OV = Container->GetCachedTargetVariable();
			}
			// Guard against GC: OV is a raw pointer that may have been collected.
			if (IsValid(OV))
			{
				const FString OVName = OV->GetDisplayNameOrFallback();
				TArray<FString> PropertyEntries;
				const UClass* StopAtClass = UGorgeousObjectVariable::StaticClass();
				for (TFieldIterator<FProperty> PropIt(OV->GetClass()); PropIt; ++PropIt)
				{
					const FProperty* Prop = *PropIt;
					if (Prop->GetOwnerClass()->IsChildOf(StopAtClass)
						&& Prop->GetOwnerClass() != StopAtClass
						&& Prop->HasAnyPropertyFlags(CPF_BlueprintVisible))
					{
						FString ValueStr;
						Prop->ExportText_Direct(ValueStr, Prop->ContainerPtrToValuePtr<void>(OV), nullptr, OV, PPF_None);
						ValueStr.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
						ValueStr.ReplaceInline(TEXT("\""), TEXT("\\\""));
						PropertyEntries.Add(FString::Printf(TEXT("  \"%s\": \"%s\""), *Prop->GetName(), *ValueStr));
					}
				}
				if (PropertyEntries.Num() > 0)
				{
					ReturnPreview = OVName
						+ LINE_TERMINATOR TEXT("{")
						+ LINE_TERMINATOR + FString::Join(PropertyEntries, TEXT(",") LINE_TERMINATOR)
						+ LINE_TERMINATOR TEXT("}");
				}
				else
				{
					ReturnPreview = OVName;
				}
			}
			// bIsDeferred=false: this is the completed relay result (equivalent to "Ready")
			FGorgeousRPCDebugTracker::Get().OnRPCExecuted(
				Result.QueuedRPC.RequestGuid, ResultCopy.Responder, ReturnPreview, false);
		}
	}

	// ── Fire per-responder callback before checking completeness ──
	if (UGorgeousAutoReplicationRPCRequestAsyncAction* Action = PendingState->Action.Get())
	{
		// Snapshot the collected results into Action->CachedResults / Action->CachedResultMap NOW,
		// before broadcasting.  Without this, GetResolvedResults() / GetResolvedResult() etc.
		// return empty data when the user reads them from within OnSingleResponderCompleted.
		// CompleteRequest() will overwrite CachedResults again with proper Server-first ordering
		// and replace CachedResults[0].TargetVariable with the UGorgeousRPC_OV container —
		// this partial snapshot is intentionally not the final form.
		Action->CachedResultMap = PendingState->CollectedResults;
		Action->CachedResults.Reset();
		Action->CachedResults.Reserve(PendingState->CollectedResults.Num());
		TArray<FString> SnapshotKeys;
		PendingState->CollectedResults.GenerateKeyArray(SnapshotKeys);
		SnapshotKeys.Sort();
		for (const FString& K : SnapshotKeys)
		{
			if (const FGorgeousAutoReplicationRPCResult* R = PendingState->CollectedResults.Find(K))
			{
				Action->CachedResults.Add(*R);
			}
		}

		// Determine if this is the last responder
		bool bAllReceived = true;
		for (const FString& ExpectedKey : PendingState->ExpectedResponders)
		{
			if (!ExpectedKey.IsEmpty() && !PendingState->CollectedResults.Contains(ExpectedKey))
			{
				bAllReceived = false;
				break;
			}
		}

		// ── Build partial FGorgeousAutoReplicationRPCAsyncResult ──────────────────
		// Same type as OnCompleted so every library helper works identically on both
		// pins.  Result = the freshly arrived responder; ResultSet / ResultMap = all
		// results so far; ResultContainer = container populated up to this point.
		FGorgeousAutoReplicationRPCAsyncResult PartialPayload;
		PartialPayload.Result                = ResultCopy;
		PartialPayload.ResultSet             = Action->CachedResults;
		PartialPayload.ResultMap             = Action->CachedResultMap;
		PartialPayload.bIsLastResult         = bAllReceived;
		PartialPayload.TotalExpectedResponders = PendingState->ExpectedResponders.Num();
		PartialPayload.TotalReceivedResponders = PendingState->CollectedResults.Num();

		if (UGorgeousRPC_OV* Container = Action->GetOrCreateResultContainer())
		{
			Container->CaptureResult(ResultCopy);
			PartialPayload.ResultContainer = Container;
		}

		// Update debug tracker: ready for single-responder callback
		if (FGorgeousRPCDebugTracker::IsEnabled())
		{
			FGorgeousRPCDebugTracker::Get().OnRPCResponderReadyStateChanged(
				ResultCopy.QueuedRPC.RequestGuid, ResponderKey,
				EGorgeousRPCReadyState::ReadyForSingleResponderCallback);
		}

		// Guard against double-broadcast: NotifyDeferredSingleResponderCallback (fired
		// by the relay component on ReadyForSingleResponderCallback) already called
		// OnSingleResponderCompleted and recorded the key in FiredSingleResponderKeys.
		// Skip re-broadcasting on the subsequent Ready relay to avoid a second callback.
		if (!PendingState->FiredSingleResponderKeys.Contains(ResponderKey))
		{
			Action->OnSingleResponderCompleted.Broadcast(PartialPayload);
			PendingState->FiredSingleResponderKeys.Add(ResponderKey);
		}

		// Update debug tracker: fully ready after callback delivered
		if (FGorgeousRPCDebugTracker::IsEnabled())
		{
			FGorgeousRPCDebugTracker::Get().OnRPCResponderReadyStateChanged(
				ResultCopy.QueuedRPC.RequestGuid, ResponderKey,
				EGorgeousRPCReadyState::Ready);
		}
	}

	TryCompletePendingRequest(ResultCopy.QueuedRPC.RequestGuid);
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::BeginDestroy()
{
	if (RequestGuid.IsValid())
	{
		PendingRequests.Remove(RequestGuid);
	}

	if (IsRooted())
	{
		RemoveFromRoot();
	}

	Super::BeginDestroy();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::FailRequest()
{
	FGorgeousQueuedRPC FailedCopy;
	FailedCopy.Key = RequestKey;
	FailedCopy.Type = RequestType;
	FailedCopy.Payload = RequestPayload;
	FailedCopy.RequestGuid = RequestGuid;
	FailedCopy.TargetKind = RequestTargetKind;

	FGorgeousAutoReplicationRPCResult FailedResult;
	FailedResult.QueuedRPC = FailedCopy;
	FailedResult.TargetKind = RequestTargetKind;
	FailedResult.TargetVariable = nullptr;
	FailedResult.TargetOwner = nullptr;
	CachedResults.Reset();
	CachedResults.Add(FailedResult);
	CachedResultMap.Reset();
	if (UGorgeousRPC_OV* Container = GetOrCreateResultContainer())
	{
		Container->CaptureResult(FailedResult);
		CachedResults[0].TargetVariable = Container;
	}

	OnFailed.Broadcast(BuildAsyncResultPayload());
	if (RequestGuid.IsValid())
	{
		PendingRequests.Remove(RequestGuid);
	}
	RequestGuid.Invalidate();
	// Clear the timeout timer so it doesn't fire after we've already failed.
	if (UObject* Context = WeakContext.Get())
	{
		if (UWorld* World = Context->GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimeoutHandle);
		}
	}
	if (IsRooted())
	{
		RemoveFromRoot();
	}
	SetReadyToDestroy();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterPendingRequest()
{
	if (!RequestGuid.IsValid())
	{
		return;
	}

	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState& PendingState = PendingRequests.FindOrAdd(RequestGuid);
	PendingState.Action = this;
	PendingState.bCompleted = false;
	PendingState.CollectedResults.Reset();
	PendingState.ExpectedResponders.Reset();
	AddToRoot();
	TryCompletePendingRequest(RequestGuid);
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::OnTimeout()
{
	GT_W_LOG("GT.AutoReplication.RPC.Timeout",
		TEXT("AutoReplication RPC request '%s' (guid: %s) timed out after %.2f second(s). "
		     "No result was received within the allowed window."),
		*RequestKey.ToString(), *RequestGuid.ToString(), RequestPayload.TimeoutSeconds);

	// Let the debug tracker mark the entry as timed-out before the request is torn down.
	if (FGorgeousRPCDebugTracker::IsEnabled() && RequestGuid.IsValid())
	{
		FGorgeousRPCDebugTracker::Get().OnRPCTimedOut(RequestGuid);
	}

	FailRequest();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterExpectedResponder(const FGuid& RequestGuid, const FGorgeousAutoReplicationRPCResponderHandle& Responder)
{
	if (!RequestGuid.IsValid() || !Responder.IsValid())
	{
		return;
	}

	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(RequestGuid);
	if (!PendingState)
	{
		return;
	}

	const FString ResponderKey = BuildResponderKey(Responder);
	if (!ResponderKey.IsEmpty())
	{
		PendingState->ExpectedResponders.Add(ResponderKey);
	}

	TryCompletePendingRequest(RequestGuid);
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::CompleteRequest(const TMap<FString, FGorgeousAutoReplicationRPCResult>& Results)
{
	if (Results.Num() == 0)
	{
		FailRequest();
		return;
	}

	CachedResults.Reset();
	CachedResults.Reserve(Results.Num());

	const FGorgeousAutoReplicationRPCResult* ServerResult = Results.Find(TEXT("Server"));
	if (ServerResult)
	{
		CachedResults.Add(*ServerResult);
	}

	TArray<FString> OrderedKeys;
	Results.GenerateKeyArray(OrderedKeys);
	OrderedKeys.Sort();
	for (const FString& Key : OrderedKeys)
	{
		if (Key == TEXT("Server"))
		{
			continue;
		}
		if (const FGorgeousAutoReplicationRPCResult* ResultPtr = Results.Find(Key))
		{
			CachedResults.Add(*ResultPtr);
		}
	}

	if (CachedResults.Num() == 0)
	{
		FailRequest();
		return;
	}
	if (UGorgeousRPC_OV* Container = GetOrCreateResultContainer())
	{
		Container->CaptureResultSet(Results, CachedResults);
		CachedResults[0].TargetVariable = Container;
	}

	CachedResultMap = Results;

	const UEnum* TypeEnum = StaticEnum<EGorgeousAutoReplicationRPCType>();
	const FString ExpectedType = TypeEnum ? TypeEnum->GetNameStringByValue(static_cast<int64>(RequestType)) : FString(TEXT("<unknown>"));
	const FGorgeousAutoReplicationRPCResult& PrimaryResult = CachedResults[0];
	const FString CompletedType = TypeEnum ? TypeEnum->GetNameStringByValue(static_cast<int64>(PrimaryResult.QueuedRPC.Type)) : FString(TEXT("<unknown>"));
	ensureMsgf(PrimaryResult.QueuedRPC.Type == RequestType,
		TEXT("Request AutoReplication RPC type mismatch. Expected %s but completed with %s for key %s."),
		*ExpectedType,
		*CompletedType,
		*PrimaryResult.QueuedRPC.Key.ToString());

	OnCompleted.Broadcast(BuildAsyncResultPayload());

	// Notify the debug tracker that this RPC completed successfully
	if (FGorgeousRPCDebugTracker::IsEnabled() && RequestGuid.IsValid())
	{
		FGorgeousRPCDebugTracker::Get().OnRPCCompleted(RequestGuid);
	}

	RequestGuid.Invalidate();
	// Clear the timeout timer — the request completed successfully.
	if (UObject* Context = WeakContext.Get())
	{
		if (UWorld* World = Context->GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimeoutHandle);
		}
	}
	if (IsRooted())
	{
		RemoveFromRoot();
	}
	SetReadyToDestroy();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::ResolveResultPointers(FGorgeousAutoReplicationRPCResult& Result)
{
	if (!Result.TargetVariable && Result.TargetVariableIdentifier.IsValid())
	{
		if (UGorgeousObjectVariable* ResolvedVariable = UGorgeousRootObjectVariable::FindVariableByIdentifier(Result.TargetVariableIdentifier))
		{
			Result.TargetVariable = ResolvedVariable;
		}
	}

	if (!Result.TargetOwner && Result.TargetVariable)
	{
		Result.TargetOwner = Result.TargetVariable->GetAutoReplicationOwner();
	}
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::ApplyRequestMetadata(FGorgeousAutoReplicationRPCResult& Result)
{
	Result.QueuedRPC.Key = RequestKey;
	Result.QueuedRPC.Type = RequestType;
	Result.QueuedRPC.Payload = RequestPayload;
	Result.QueuedRPC.TargetKind = RequestTargetKind;
	Result.QueuedRPC.RequestGuid = RequestGuid;

	if (Result.TargetKind == EGorgeousAutoReplicationTargetKind::EAuto)
	{
		Result.TargetKind = RequestTargetKind;
	}
}

FString UGorgeousAutoReplicationRPCRequestAsyncAction::BuildResponderKey(const FGorgeousAutoReplicationRPCResponderHandle& Responder)
{
	return Responder.IsValid() ? Responder.GetStableKey() : FString();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::TryCompletePendingRequest(const FGuid& RequestGuid)
{
	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(RequestGuid);
	if (!PendingState || PendingState->bCompleted)
	{
		return;
	}

	if (!PendingState->Action.IsValid())
	{
		PendingRequests.Remove(RequestGuid);
		return;
	}

	if (PendingState->CollectedResults.Num() == 0)
	{
		return;
	}

	bool bHasOutstandingResponder = false;
	for (const FString& ExpectedKey : PendingState->ExpectedResponders)
	{
		if (ExpectedKey.IsEmpty())
		{
			continue;
		}

		if (!PendingState->CollectedResults.Contains(ExpectedKey))
		{
			bHasOutstandingResponder = true;
			break;
		}
	}

	if (bHasOutstandingResponder)
	{
		return;
	}

	PendingState->bCompleted = true;
	const TMap<FString, FGorgeousAutoReplicationRPCResult> CompletedResults = PendingState->CollectedResults;
	if (UGorgeousAutoReplicationRPCRequestAsyncAction* Action = PendingState->Action.Get())
	{
		Action->CompleteRequest(CompletedResults);
	}
	PendingRequests.Remove(RequestGuid);
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyDeferredSingleResponderCallback(
	const FGuid& RequestGuid, const FGorgeousAutoReplicationRPCResult& Result)
{
	if (!RequestGuid.IsValid())
	{
		return;
	}

	FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(RequestGuid);
	if (!PendingState || PendingState->bCompleted)
	{
		// No local pending state — relay back to the originating client (deferred/interim path).
		UGorgeousAutoReplicationRPCRelayComponent::TryRelayResultToClientInitiator(
			Result, EGorgeousRPCReadyState::ReadyForSingleResponderCallback);
		return;
	}

	const FString ResponderKey = BuildResponderKey(Result.Responder);
	if (ResponderKey.IsEmpty())
	{
		return;
	}

	// Insert into DeferredResults so ExecuteSingleResponderCallback can find the entry.
	PendingState->DeferredResults.Add(ResponderKey, Result);

	if (!PendingState->FiredSingleResponderKeys.Contains(ResponderKey))
	{
		ExecuteSingleResponderCallback(Result.QueuedRPC, ResponderKey, Result, *PendingState);
	}
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::ExecuteSingleResponderCallback(
	const FGorgeousQueuedRPC& QueuedRPC,
	const FString& ResponderKey,
	const FGorgeousAutoReplicationRPCResult& ResultCopy,
	FGorgeousAutoReplicationPendingRequestState& PendingState)
{
	UGorgeousAutoReplicationRPCRequestAsyncAction* Action = PendingState.Action.Get();
	if (!Action)
	{
		return;
	}

	// Build a combined snapshot: already-collected results + this deferred result
	TMap<FString, FGorgeousAutoReplicationRPCResult> SnapshotMap = PendingState.CollectedResults;
	SnapshotMap.Add(ResponderKey, ResultCopy);

	Action->CachedResultMap = SnapshotMap;
	Action->CachedResults.Reset();
	Action->CachedResults.Reserve(SnapshotMap.Num());
	TArray<FString> SnapshotKeys;
	SnapshotMap.GenerateKeyArray(SnapshotKeys);
	SnapshotKeys.Sort();
	for (const FString& K : SnapshotKeys)
	{
		if (const FGorgeousAutoReplicationRPCResult* R = SnapshotMap.Find(K))
		{
			Action->CachedResults.Add(*R);
		}
	}

	FGorgeousAutoReplicationRPCAsyncResult PartialPayload;
	PartialPayload.Result                  = ResultCopy;
	PartialPayload.ResultSet               = Action->CachedResults;
	PartialPayload.ResultMap               = Action->CachedResultMap;
	PartialPayload.bIsLastResult           = false;
	PartialPayload.TotalExpectedResponders = PendingState.ExpectedResponders.Num();
	PartialPayload.TotalReceivedResponders = PendingState.CollectedResults.Num() + 1;
	if (UGorgeousRPC_OV* Container = Action->GetOrCreateResultContainer())
	{
		Container->CaptureResult(ResultCopy);
		PartialPayload.ResultContainer = Container;
	}

	// Update the debug tracker with the now-populated OV preview
	if (FGorgeousRPCDebugTracker::IsEnabled())
	{
		FString ReturnPreview;
		UGorgeousObjectVariable* OV = ResultCopy.TargetVariable;
		if (!IsValid(OV))
		{
			if (UGorgeousRPC_OV* Container = Action->GetOrCreateResultContainer())
			{
				OV = Container->GetCachedTargetVariable();
			}
		}
		// Guard against GC: the OV may have been collected between RegisterDeferredResult
		// and MarkAutoReplicationRPCResponderReady (deferred handlers with Blueprint delays).
		if (IsValid(OV))
		{
			const FString OVName = OV->GetDisplayNameOrFallback();
			TArray<FString> PropertyEntries;
			const UClass* StopAtClass = UGorgeousObjectVariable::StaticClass();
			for (TFieldIterator<FProperty> PropIt(OV->GetClass()); PropIt; ++PropIt)
			{
				const FProperty* Prop = *PropIt;
				if (Prop->GetOwnerClass()->IsChildOf(StopAtClass)
					&& Prop->GetOwnerClass() != StopAtClass
					&& Prop->HasAnyPropertyFlags(CPF_BlueprintVisible))
				{
					FString ValueStr;
					Prop->ExportText_Direct(ValueStr, Prop->ContainerPtrToValuePtr<void>(OV), nullptr, OV, PPF_None);
					ValueStr.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
					ValueStr.ReplaceInline(TEXT("\""), TEXT("\\\""));
					PropertyEntries.Add(FString::Printf(TEXT("  \"%s\": \"%s\""), *Prop->GetName(), *ValueStr));
				}
			}
			if (PropertyEntries.Num() > 0)
			{
				ReturnPreview = OVName
					+ LINE_TERMINATOR TEXT("{")
					+ LINE_TERMINATOR + FString::Join(PropertyEntries, TEXT(",") LINE_TERMINATOR)
					+ LINE_TERMINATOR TEXT("}");
			}
			else
			{
				ReturnPreview = OVName;
			}
		}
		FGorgeousRPCDebugTracker::Get().OnRPCExecuted(
			QueuedRPC.RequestGuid, ResultCopy.Responder, ReturnPreview, false);
	}

	if (FGorgeousRPCDebugTracker::IsEnabled())
	{
		FGorgeousRPCDebugTracker::Get().OnRPCResponderReadyStateChanged(
			QueuedRPC.RequestGuid, ResponderKey,
			EGorgeousRPCReadyState::ReadyForSingleResponderCallback);
	}

	Action->OnSingleResponderCompleted.Broadcast(PartialPayload);
	PendingState.FiredSingleResponderKeys.Add(ResponderKey);

	if (FGorgeousRPCDebugTracker::IsEnabled())
	{
		FGorgeousRPCDebugTracker::Get().OnRPCResponderReadyStateChanged(
			QueuedRPC.RequestGuid, ResponderKey, EGorgeousRPCReadyState::Ready);
	}
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterDeferredResult(
	const FGuid& RequestGuid, const FString& ResponderKey, const FGorgeousAutoReplicationRPCResult& Result)
{
	if (!RequestGuid.IsValid() || ResponderKey.IsEmpty())
	{
		return;
	}

	FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(RequestGuid);
	if (!PendingState || PendingState->bCompleted)
	{
		// No local pending state: this machine is a remote responder (async action lives on
		// another machine, typically the server). Cache the result locally so that when
		// MarkAutoReplicationRPCResponderReady fires it can relay the populated OV back.
		ClientDeferredResultCache.FindOrAdd(RequestGuid).Add(ResponderKey, Result);
		return;
	}

	PendingState->DeferredResults.Add(ResponderKey, Result);

	// Mark this responder as awaiting an explicit readiness signal in the debug tracker
	if (FGorgeousRPCDebugTracker::IsEnabled())
	{
		FGorgeousRPCDebugTracker::Get().OnRPCResponderReadyStateChanged(
			RequestGuid, ResponderKey, EGorgeousRPCReadyState::NotReadyToCollect);
	}

	// If MarkAutoReplicationRPCResponderReady was called synchronously from within the handler
	// (before this RegisterDeferredResult call completed), process the queued ready-signal now.
	if (EGorgeousRPCReadyState* QueuedSignal = PendingState->PendingReadySignals.Find(ResponderKey))
	{
		const EGorgeousRPCReadyState QueuedReadyState = *QueuedSignal;
		PendingState->PendingReadySignals.Remove(ResponderKey);

		FGorgeousAutoReplicationRPCResult ResultCopy = PendingState->DeferredResults.FindChecked(ResponderKey);
		if (QueuedReadyState == EGorgeousRPCReadyState::ReadyForSingleResponderCallback)
		{
			if (!PendingState->FiredSingleResponderKeys.Contains(ResponderKey))
			{
				ExecuteSingleResponderCallback(Result.QueuedRPC, ResponderKey, ResultCopy, *PendingState);
			}
		}
		else if (QueuedReadyState == EGorgeousRPCReadyState::Ready)
		{
			if (!PendingState->FiredSingleResponderKeys.Contains(ResponderKey))
			{
				ExecuteSingleResponderCallback(Result.QueuedRPC, ResponderKey, ResultCopy, *PendingState);
			}
			PendingState->DeferredResults.Remove(ResponderKey);
			PendingState->CollectedResults.Add(ResponderKey, ResultCopy);
			TryCompletePendingRequest(RequestGuid);
		}
	}
}

UGorgeousRPC_OV* UGorgeousAutoReplicationRPCRequestAsyncAction::GetOrCreateResultContainer()
{
	if (ResultContainer)
	{
		return ResultContainer;
	}

	if (!InternalResultContainer)
	{
		InternalResultContainer = NewObject<UGorgeousRPC_OV>(this);
	}

	return InternalResultContainer.Get();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::MarkAutoReplicationRPCResponderReady(
	UObject* WorldContextObject, const FGorgeousQueuedRPC& QueuedRPC, EGorgeousRPCReadyState ReadyState)
{
	if (!QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	if (!WorldContextObject)
	{
		GT_W_LOG("GT.AutoReplication.RPC",
			TEXT("MarkAutoReplicationRPCResponderReady called with null WorldContextObject for RPC %s. Responder ready state: %s"),
			*QueuedRPC.Key.ToString(),
			*UEnum::GetValueAsString(ReadyState));
		return;
	}

	// Derive the responder key from the world context
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		GT_W_LOG("GT.AutoReplication.RPC",
			TEXT("MarkAutoReplicationRPCResponderReady: Could not get World from context for RPC %s. Responder ready state: %s"),
			*QueuedRPC.Key.ToString(),
			*UEnum::GetValueAsString(ReadyState));
		return;
	}

	FString ResponderKey;
	APlayerController* PC = nullptr;
	
	// Try to get the first local player controller
	if (World->GetNetMode() == NM_Client || World->GetNetMode() == NM_Standalone)
	{
		PC = World->GetFirstPlayerController();
	}

	if (PC)
	{
		const FGorgeousAutoReplicationRPCResponderHandle Handle =
			FGorgeousAutoReplicationRPCResponderHandle::FromController(PC);
		ResponderKey = Handle.GetStableKey();
	}
	else if (World->GetAuthGameMode() != nullptr)
	{
		// On authority without a player controller — treat as Server.
		ResponderKey = TEXT("Server");
	}

	// Always update the debug tracker when enabled
	if (FGorgeousRPCDebugTracker::IsEnabled())
	{
		FGorgeousRPCDebugTracker::Get().OnRPCResponderReadyStateChanged(
			QueuedRPC.RequestGuid, ResponderKey, ReadyState);
	}

	// Relay ready-state to the server debug tracker so the RPC Inspector updates
	// instantly regardless of which machine called Mark RPC Responder Ready.
	if (World->GetNetMode() == NM_Client && !ResponderKey.IsEmpty() && PC)
	{
		if (UGorgeousAutoReplicationRPCRelayComponent* Relay = PC->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>())
		{
			Relay->RelayResponderReadyStateToServer(QueuedRPC.RequestGuid, ResponderKey, ReadyState);
		}
	}

	// Drive deferred dispatch: promote stored deferred results when the handler signals readiness
	FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(QueuedRPC.RequestGuid);
	if (!PendingState || PendingState->bCompleted || !PendingState->Action.IsValid())
	{
		// No pending state means this is a remote responder (not the RPC initiator).
		// Build and relay the result back to the server so it can forward to the initiator.
		const bool bShouldRelay =
			(ReadyState == EGorgeousRPCReadyState::Ready ||
			 ReadyState == EGorgeousRPCReadyState::ReadyForSingleResponderCallback)
			&& !ResponderKey.IsEmpty();

		if (bShouldRelay)
		{
			// Check if we're not on authority - only clients should relay back
			if (World->GetNetMode() == NM_Client)
			{
				// Start from any deferred result we cached in RegisterDeferredResult.
				// This ensures the populated OV (written by the handler after its delay)
				// is included in the relay to the server.
				FGorgeousAutoReplicationRPCResult Result;
				if (TMap<FString, FGorgeousAutoReplicationRPCResult>* ByResponder =
					ClientDeferredResultCache.Find(QueuedRPC.RequestGuid))
				{
					if (FGorgeousAutoReplicationRPCResult* Cached = ByResponder->Find(ResponderKey))
					{
						Result = *Cached;
					}
					// Only consume from the cache on final Ready.
					// ReadyForSingleResponderCallback is interim — the entry must stay so
					// the subsequent Ready relay can still find and consume it.
					if (ReadyState == EGorgeousRPCReadyState::Ready)
					{
						ByResponder->Remove(ResponderKey);
						if (ByResponder->IsEmpty())
						{
							ClientDeferredResultCache.Remove(QueuedRPC.RequestGuid);
						}
					}
				}

				// Ensure required fields are set (may already be set from cached data)
				Result.QueuedRPC = QueuedRPC;
				if (PC && !Result.Responder.IsValid())
				{
					Result.Responder = FGorgeousAutoReplicationRPCResponderHandle::FromController(PC);
				}
				
				// Relay the result back to server, carrying the ready state so the server
				// knows whether to fire the single-responder callback or complete normally.
				if (PC)
				{
					if (UGorgeousAutoReplicationRPCRelayComponent* Relay = PC->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>())
					{
						Relay->RelayResultToServer(Result, ReadyState);
					}
				}
			}
		}
		return;
	}

	const FGorgeousAutoReplicationRPCResult* DeferredEntry = PendingState->DeferredResults.Find(ResponderKey);
	if (!DeferredEntry)
	{
		// The handler called MarkReady synchronously — RegisterDeferredResult has not run yet.
		// Queue the signal so it is processed immediately when the deferred entry is registered.
		if (!ResponderKey.IsEmpty())
		{
			PendingState->PendingReadySignals.Add(ResponderKey, ReadyState);
		}
		return;
	}

	FGorgeousAutoReplicationRPCResult ResultCopy = *DeferredEntry;
	UGorgeousAutoReplicationRPCRequestAsyncAction* Action = PendingState->Action.Get();

	// Helper: fire OnSingleResponderCompleted with a partial snapshot that includes this result.
	// Delegates to the static ExecuteSingleResponderCallback so the same logic is reusable
	// by the relay component when a remote client signals ReadyForSingleResponderCallback.
	auto FireSingleResponderCallback = [&]()
	{
		ExecuteSingleResponderCallback(QueuedRPC, ResponderKey, ResultCopy, *PendingState);
	};

	if (ReadyState == EGorgeousRPCReadyState::ReadyForSingleResponderCallback)
	{
		// Fire the single-responder callback now, but keep the result in DeferredResults until Ready
		if (!PendingState->FiredSingleResponderKeys.Contains(ResponderKey))
		{
			FireSingleResponderCallback();
		}
	}
	else if (ReadyState == EGorgeousRPCReadyState::Ready)
	{
		// Fire single-responder callback if not already fired at ReadyForSingleResponderCallback
		if (!PendingState->FiredSingleResponderKeys.Contains(ResponderKey))
		{
			FireSingleResponderCallback();
		}
		// Promote deferred result into the main collected map and check for overall completion
		PendingState->DeferredResults.Remove(ResponderKey);
		PendingState->CollectedResults.Add(ResponderKey, ResultCopy);
		TryCompletePendingRequest(QueuedRPC.RequestGuid);
	}
}

FGorgeousAutoReplicationRPCAsyncResult UGorgeousAutoReplicationRPCRequestAsyncAction::BuildAsyncResultPayload() const
{
	FGorgeousAutoReplicationRPCAsyncResult Payload;
	Payload.Result = GetResolvedResult();
	Payload.ResultSet = CachedResults;
	Payload.ResultMap = CachedResultMap;
	Payload.ResultContainer = ResultContainer ? ResultContainer : InternalResultContainer.Get();
	Payload.bIsLastResult = true;
	Payload.TotalExpectedResponders = CachedResults.Num();
	Payload.TotalReceivedResponders = CachedResults.Num();
	return Payload;
}

const FGorgeousAutoReplicationRPCResult& UGorgeousAutoReplicationRPCRequestAsyncAction::GetResolvedResult() const
{
	if (CachedResults.Num() > 0)
	{
		return CachedResults[0];
	}

	static const FGorgeousAutoReplicationRPCResult EmptyResult;
	return EmptyResult;
}

FGorgeousAutoReplicationMixin* UGorgeousAutoReplicationRPCRequestAsyncAction::ResolveAutoReplicationMixin(UObject* Context)
{
	auto ResolveFromContext = [](UObject* Object) -> FGorgeousAutoReplicationMixin*
	{
		if (!Object)
		{
			return nullptr;
		}
		
		if (AGorgeousGameMode* GameMode = Cast<AGorgeousGameMode>(Object))
		{
			return &GameMode->GetAutoReplicationMixin();
		}
		if (AGorgeousGameState* GameState = Cast<AGorgeousGameState>(Object))
		{
			return &GameState->GetAutoReplicationMixin();
		}
		if (AGorgeousPlayerController* PlayerController = Cast<AGorgeousPlayerController>(Object))
		{
			return &PlayerController->GetAutoReplicationMixin();
		}
		if (AGorgeousPlayerState* PlayerState = Cast<AGorgeousPlayerState>(Object))
		{
			return &PlayerState->GetAutoReplicationMixin();
		}
		if (AGorgeousWorldSettings* WorldSettings = Cast<AGorgeousWorldSettings>(Object))
		{
			return &WorldSettings->GetAutoReplicationMixin();
		}

		return nullptr;
	};

	if (FGorgeousAutoReplicationMixin* DirectMixin = ResolveFromContext(Context))
	{
		return DirectMixin;
	}

	if (AGorgeousGameState* WorldGameState = Cast<AGorgeousGameState>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(Context, AGorgeousGameState::StaticClass())))
	{
		return &WorldGameState->GetAutoReplicationMixin();
	}
	if (AGorgeousGameMode* WorldGameMode = Cast<AGorgeousGameMode>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(Context, AGorgeousGameMode::StaticClass())))
	{
		return &WorldGameMode->GetAutoReplicationMixin();
	}
	if (AGorgeousWorldSettings* WorldSettings = UGorgeousCoreRuntimeGlobals::GetGorgeousWorldSettings(Context))
	{
		return &WorldSettings->GetAutoReplicationMixin();
	}

	return nullptr;
}

