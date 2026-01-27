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
	{
	}

	/** Primary result payload for convenience. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	FGorgeousAutoReplicationRPCResult Result;

	/** Ordered list of every responder result captured for this request. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	TArray<FGorgeousAutoReplicationRPCResult> ResultSet;

	/** Keyed responder map for quick lookups by connection identifier. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	TMap<FString, FGorgeousAutoReplicationRPCResult> ResultMap;

	/** Object variable container that stores the aggregated results (always UGorgeousRPC_OV for async requests). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking")
	UGorgeousRPC_OV* ResultContainer;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGorgeousAutoReplicationRPCAsyncDelegate, const FGorgeousAutoReplicationRPCAsyncResult&, Result);

/** Blueprint async node that queues a AutoReplication RPC and fires when the payload is executed. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCRequestAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/** Starts an asynchronous AutoReplication RPC request and exposes completion/failure pins. */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "Context", DisplayName = "Request AutoReplication RPC"), Category = "Gorgeous Core|AutoReplication|Networking")
	static UGorgeousAutoReplicationRPCRequestAsyncAction* RequestAutoReplicationRPC(UObject* Context, FName Key, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EAuto);

	virtual void Activate() override;

	/** Invoked by the AutoReplication runtime whenever a queued RPC finishes executing. */
	static void NotifyRequestCompleted(const FGorgeousAutoReplicationRPCResult& Result);

	/** Allows transporters/mixins to declare responders that are expected to submit results. */
	static void RegisterExpectedResponder(const FGuid& RequestGuid, const FGorgeousAutoReplicationRPCResponderHandle& Responder);

	/** Completion event that exposes the responder map through UGorgeousRPC_OV. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousAutoReplicationRPCAsyncDelegate OnCompleted;

	/** Failure event that mirrors the detailed completion payload. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousAutoReplicationRPCAsyncDelegate OnFailed;

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
	};

	static TMap<FGuid, FGorgeousAutoReplicationPendingRequestState> PendingRequests;

	TWeakObjectPtr<UObject> WeakContext;
	FName RequestKey;
	EGorgeousAutoReplicationRPCType RequestType;
	FGorgeousRPCPayload RequestPayload;
	EGorgeousAutoReplicationTargetKind RequestTargetKind;
	FGuid RequestGuid;
	bool bActivated;

	UPROPERTY()
	TObjectPtr<UGorgeousRPC_OV> InternalResultContainer;

	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|Networking", meta = (AllowPrivateAccess = "true"))
	TMap<FString, FGorgeousAutoReplicationRPCResult> CachedResultMap;
};

