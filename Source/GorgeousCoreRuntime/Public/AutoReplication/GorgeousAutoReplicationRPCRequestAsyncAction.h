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

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "GorgeousAutoReplicationRPCRequestAsyncAction.generated.h"

class UGorgeousObjectVariable;
class FGorgeousAutoReplicationMixin;
class UGorgeousRPC_OV;

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationRPCAsyncResult
{
	GENERATED_BODY()

	FGorgeousAutoReplicationRPCAsyncResult()
		: ResultContainer(nullptr)
		, bIsLastResult(true)
		, TotalExpectedResponders(0)
		, TotalReceivedResponders(0)
	{
	}

	/**
	 * Primary result payload for convenience.
	 * When fired from OnSingleResponderCompleted this is the result of the responder
	 * that just arrived.  When fired from OnCompleted this is the first (server-side)
	 * result in the ordered set.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	FGorgeousAutoReplicationRPCResult Result;

	/** Ordered list of every responder result received so far (server-first on completion). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	TArray<FGorgeousAutoReplicationRPCResult> ResultSet;

	/** Keyed responder map for quick lookups by connection identifier. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	TMap<FString, FGorgeousAutoReplicationRPCResult> ResultMap;

	/** Object variable container that stores aggregated results.  Populated progressively
	 *  on each OnSingleResponderCompleted firing; contains the full final set on OnCompleted. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	UGorgeousRPC_OV* ResultContainer;

	/**
	 * True when this is the final delivery of results for the request.
	 * Always true for OnCompleted / OnFailed.  For OnSingleResponderCompleted it is
	 * true only when all expected responders have now replied.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	bool bIsLastResult;

	/** Total number of responders expected to reply (0 = unknown, e.g. multicast). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	int32 TotalExpectedResponders;

	/** Number of responders that have replied so far (including the current one). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	int32 TotalReceivedResponders;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGorgeousAutoReplicationRPCAsyncDelegate, const FGorgeousAutoReplicationRPCAsyncResult&, Result);

/** Blueprint async node that queues a AutoReplication RPC and fires when the payload is executed. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCRequestAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/** Starts an asynchronous AutoReplication RPC request and exposes completion/failure pins. */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Payload"), DisplayName = "Request AutoReplication RPC", Category = "Gorgeous Core|AutoReplication|Networking")
	static UGorgeousAutoReplicationRPCRequestAsyncAction* RequestAutoReplicationRPC(UObject* WorldContextObject, FName Key, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, const EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EAuto, AActor* OptionalTarget = nullptr);

	virtual void Activate() override;

	/** Invoked by the AutoReplication runtime whenever a queued RPC finishes executing. */
	static void NotifyRequestCompleted(const FGorgeousAutoReplicationRPCResult& Result);

	/** Allows transporters/mixins to declare responders that are expected to submit results. */
	static void RegisterExpectedResponder(const FGuid& RequestGuid, const FGorgeousAutoReplicationRPCResponderHandle& Responder);

	/**
	 * Stores the result from a deferred handler and marks it as NotReadyToCollect in the debug
	 * tracker.  The result is promoted to CollectedResults only when the handler signals readiness
	 * via MarkAutoReplicationRPCResponderReady.
	 */
	static void RegisterDeferredResult(const FGuid& RequestGuid, const FString& ResponderKey, const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Called by the relay component when it receives a ReadyForSingleResponderCallback
	 * relay from a remote client.  Inserts the result into the pending deferred map and
	 * fires OnSingleResponderCompleted without completing the overall request.
	 */
	static void NotifyDeferredSingleResponderCallback(const FGuid& RequestGuid, const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Marks the responder that is currently handling an AutoReplication RPC event as having
	 * reached the given readiness state.  Call this from within HandleAutoReplicationRPC
	 * (or from any async continuation started inside it) to signal processing progress.
	 *
	 * The state is stored per-responder and forwarded to the RPC debug tracker so the
	 * inspector panel can show live progress without affecting the completion pathway.
	 *
	 * @param WorldContextObject  The world context (used to derive the responder and relay results).
	 * @param QueuedRPC           The RPC descriptor passed to HandleAutoReplicationRPC.
	 * @param ReadyState          The new readiness state for this responder.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Mark RPC Responder Ready"))
	static void MarkAutoReplicationRPCResponderReady(UObject* WorldContextObject, const FGorgeousQueuedRPC& QueuedRPC, EGorgeousRPCReadyState ReadyState);

	/** Completion event that exposes the responder map through UGorgeousRPC_OV. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousAutoReplicationRPCAsyncDelegate OnCompleted;

	/** Failure event that mirrors the detailed completion payload. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousAutoReplicationRPCAsyncDelegate OnFailed;

	/**
	 * Fires each time a single responder (client or server) delivers its result,
	 * BEFORE OnCompleted fires.  The payload is the same FGorgeousAutoReplicationRPCAsyncResult
	 * used by OnCompleted so every library helper (GetPrimaryValueResult, GetAllValueResults,
	 * GetResultForResponder, …) works identically on both pins.
	 *
	 * Result.bIsLastResult is true when this firing is also the last one (all responders
	 * have replied).  ResultContainer is populated progressively — it holds all results
	 * received so far, including the one that just triggered this event.
	 */
	UPROPERTY(BlueprintAssignable)
	FGorgeousAutoReplicationRPCAsyncDelegate OnSingleResponderCompleted;

	/** Optional container that will receive a copy of the result when the request finishes. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|AutoReplication|Networking", meta = (ExposeOnSpawn = "true"))
	UGorgeousRPC_OV* ResultContainer;

	/** Cached responder results collected for the most recent request (may contain a single entry). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking", meta = (AllowPrivateAccess = "true"))
	TArray<FGorgeousAutoReplicationRPCResult> CachedResults;

	/** Returns the cached target variable that handled the most recent RPC (if any). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	UGorgeousObjectVariable* GetResolvedTargetVariable() const { return CachedResults.Num() > 0 ? CachedResults[0].TargetVariable : nullptr; }

	/** Returns the owning QoL object that handled the most recent RPC (when applicable). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	UObject* GetResolvedTargetOwner() const { return CachedResults.Num() > 0 ? CachedResults[0].TargetOwner : nullptr; }

	/** Provides the last completion payload including handler metadata. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	const FGorgeousAutoReplicationRPCResult& GetResolvedResult() const;

	/** Returns every per-connection result captured by this request (may contain a single entry). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	const TArray<FGorgeousAutoReplicationRPCResult>& GetResolvedResults() const { return CachedResults; }

	/** Returns the keyed responder map that backs the cached results. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	const TMap<FString, FGorgeousAutoReplicationRPCResult>& GetResolvedResultMap() const { return CachedResultMap; }

protected:
	virtual void BeginDestroy() override;

private:
	void FailRequest();
	void RegisterPendingRequest();
	void CompleteRequest(const TMap<FString, FGorgeousAutoReplicationRPCResult>& Results);
	void ResolveResultPointers(FGorgeousAutoReplicationRPCResult& Result);
	void ApplyRequestMetadata(FGorgeousAutoReplicationRPCResult& Result);
	static FString BuildResponderKey(const FGorgeousAutoReplicationRPCResponderHandle& Responder);
	static void TryCompletePendingRequest(const FGuid& RequestGuid);
	UGorgeousRPC_OV* GetOrCreateResultContainer();
	FGorgeousAutoReplicationRPCAsyncResult BuildAsyncResultPayload() const;

	/** Called when the per-request timeout elapses; triggers FailRequest(). */
	void OnTimeout();

	static FGorgeousAutoReplicationMixin* ResolveAutoReplicationMixin(UObject* Context);

	struct FGorgeousAutoReplicationPendingRequestState
	{
		FGorgeousAutoReplicationPendingRequestState()
			: bCompleted(false)
		{
		}

		TWeakObjectPtr<UGorgeousAutoReplicationRPCRequestAsyncAction> Action;
		TMap<FString, FGorgeousAutoReplicationRPCResult> CollectedResults;
		TSet<FString> ExpectedResponders;
		bool bCompleted;

		/** Results from deferred handlers that have not yet signalled readiness. */
		TMap<FString, FGorgeousAutoReplicationRPCResult> DeferredResults;
		/** Tracks which responder keys have already had their OnSingleResponderCompleted callback fired. */
		TSet<FString> FiredSingleResponderKeys;
		/**
		 * Ready-state signals queued from MarkAutoReplicationRPCResponderReady calls that arrived
		 * BEFORE RegisterDeferredResult was called for the same responder key.
		 * This happens when a Blueprint handler calls MarkReady synchronously (within the same
		 * execution frame as the RPC dispatch), before EmitResult has stored the deferred entry.
		 * Consumed and processed immediately in RegisterDeferredResult when the entry is stored.
		 */
		TMap<FString, EGorgeousRPCReadyState> PendingReadySignals;
	};

	/**
	 * Fires OnSingleResponderCompleted for one deferred responder without promoting it to
	 * CollectedResults or completing the overall request.
	 */
	static void ExecuteSingleResponderCallback(
		const FGorgeousQueuedRPC& QueuedRPC,
		const FString& ResponderKey,
		const FGorgeousAutoReplicationRPCResult& ResultCopy,
		FGorgeousAutoReplicationPendingRequestState& PendingState);

	static TMap<FGuid, FGorgeousAutoReplicationPendingRequestState> PendingRequests;

	/**
	 * Client-side cache for deferred results that have no matching PendingRequests entry
	 * (because the async action lives on the server). Keyed by RequestGuid → ResponderKey → Result.
	 * Populated in RegisterDeferredResult when called on a remote machine; consumed and cleared
	 * by MarkAutoReplicationRPCResponderReady when it builds the relay back to the server.
	 */
	static TMap<FGuid, TMap<FString, FGorgeousAutoReplicationRPCResult>> ClientDeferredResultCache;

	TWeakObjectPtr<UObject> WeakContext;
	FName RequestKey;
	EGorgeousAutoReplicationRPCType RequestType;
	FGorgeousRPCPayload RequestPayload;
	EGorgeousAutoReplicationTargetKind RequestTargetKind;
	FGuid RequestGuid;
	bool bActivated;

	/** Handle for the per-request timeout timer. Cleared on completion or failure. */
	FTimerHandle TimeoutHandle;

	UPROPERTY()
	TObjectPtr<UGorgeousRPC_OV> InternalResultContainer;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking", meta = (AllowPrivateAccess = "true"))
	TMap<FString, FGorgeousAutoReplicationRPCResult> CachedResultMap;
};

