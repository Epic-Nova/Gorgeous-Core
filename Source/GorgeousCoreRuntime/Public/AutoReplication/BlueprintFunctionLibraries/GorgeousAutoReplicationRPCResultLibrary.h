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

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "GorgeousAutoReplicationRPCResultLibrary.generated.h"


/***
═══════════════════════════════════════════════════════════════════════════════════
  FGorgeousAutoReplicationRPCValueResult  (slim struct — shortest path to the value)
     .ReturnOV      UGorgeousObjectVariable*                ← actual handler-written OV
     .RPCContainer  UGorgeousRPC_OV*                        ← container (populated progressively)
     .Responder     FGorgeousAutoReplicationRPCResponderHandle
  NOTE: ReturnOV is always the leaf OV — ToValueResult drills through the RPC_OV
        container automatically so you never receive the container as the return value.
═══════════════════════════════════════════════════════════════════════════════════

═══════════════════════════════════════════════════════════════════════════════
  OnCompleted / OnFailed / OnSingleResponderCompleted  →  FGorgeousAutoReplicationRPCAsyncResult
  (all three pins deliver the SAME type)
═══════════════════════════════════════════════════════════════════════════════
  │
  ├─ GetPrimaryValueResult()    ──→ FGorgeousAutoReplicationRPCValueResult        ← SHORTEST PATH
  │        .ReturnOV      leaf OV the handler wrote (drilled through container)
  │        .RPCContainer  UGorgeousRPC_OV* (populated progressively from first responder onwards)
  │        .Responder     who produced the primary result
  │
  ├─ GetAllValueResults()       ──→ TArray<FGorgeousAutoReplicationRPCValueResult> ← SHORTEST LOOP
  │
  ├─[progress / ordering]──────────────────────────────────────────────────────────
  │  ├─ IsLastResult()                 →  bool    (false on per-responder callbacks until the final one)
  │  ├─ GetResponderProgress()         →  FString ("1/3")
  │  └─ GetResponderProgressFraction() →  float   (0.33)
  │
  ├─[primary shortcuts]────────────────────────────────────────────────────────────
  │  ├─ GetPrimaryTargetVariable()   →  UGorgeousObjectVariable*  (raw — may be RPC_OV)
  │  └─ GetPrimaryResponder()        →  FGorgeousAutoReplicationRPCResponderHandle
  │
  ├─[lookup by key]────────────────────────────────────────────────────────────────
  │  ├─ GetResultForResponder(FString)           →  bool + FGorgeousAutoReplicationRPCResult
  │  ├─ HasResultForResponder(FString)           →  bool
  │  ├─ GetResultForResponderHandle(Handle)      →  bool + FGorgeousAutoReplicationRPCResult
  │  └─ HasResultForResponderHandle(Handle)      →  bool
  │
  ├─[full set]─────────────────────────────────────────────────────────────────────
  │  ├─ GetAllResults()              →  TArray<FGorgeousAutoReplicationRPCResult>
  │  ├─ GetAllResponderKeys()        →  TArray<FString>
  │  └─ GetResultCount()             →  int32
  │
  ├─ GetResultContainer()       ──→ UGorgeousRPC_OV*  (same as .RPCContainer on slim struct)
  │
  └─ GetPrimaryResult() ──────────────────────────────────────────────────────────
         │
         └─► FGorgeousAutoReplicationRPCResult  (see below)


═══════════════════════════════════════════════════════════════════════════
  FGorgeousAutoReplicationRPCResult  (raw single-responder result)
═══════════════════════════════════════════════════════════════════════════
  │
  ├─ ToValueResult()             →  FGorgeousAutoReplicationRPCValueResult  ← collapse to slim
  │
  ├─[value]────────────────────────────────────────────────────────────────
  │  ├─ GetTargetVariable()      →  UGorgeousObjectVariable*  (raw — may be RPC_OV on completion path)
  │  └─ HasTargetVariable()      →  bool
  │
  ├─[who]──────────────────────────────────────────────────────────────────
  │  ├─ GetResponder()           →  FGorgeousAutoReplicationRPCResponderHandle
  │  ├─ IsServerResult()         →  bool
  │  └─ GetResponderDescription()→  FString  ("Server" / "Client_0")
  │
  └─[rpc metadata]─────────────────────────────────────────────────────────
     ├─ GetHandlerName()         →  FName   (handler function called)
     ├─ GetRPCKey()              →  FName   (AutoReplication channel key)
     └─ GetTargetKindInfo()      →  FString ("Owner" / "ObjectVariable" / ...)


Rule of thumb:
    Just need the return value?                       → GetPrimaryValueResult → .ReturnOV → Cast  (same node for all three pins)
    Need to loop all responders?                      → GetAllValueResults → ForEach → .ReturnOV
    Need to know who answered?                        → .Responder on the slim struct, or GetPrimaryResponder
    Need the full result set?                         → .RPCContainer → GetCachedResults()
    Need debug/channel metadata?                      → GetPrimaryResult → GetHandlerName / GetRPCKey / GetTargetKindInfo
    Need persistent OV storage?                       → GetResultContainer  (same object as .RPCContainer)
 */


class UGorgeousObjectVariable;

/**
 * Slim result containing only the essentials you need to work with an RPC return value.
 *
 * Obtain via ToValueResult / GetPrimaryValueResult / GetAllValueResults.
 * Use GetPrimaryResult / GetAllResults / GetResult when you need the full raw data
 * (QueuedRPC metadata, TargetKind, TargetOwner, TargetVariableIdentifier, ...).
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationRPCValueResult
{
	GENERATED_BODY()

	/**
	 * The actual OV the handler wrote its return value into (e.g. UInteger_SOV).
	 * Drills through the UGorgeousRPC_OV container automatically — always gives you the
	 * leaf value, never the container itself. Null if the handler returned nothing.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	TObjectPtr<UGorgeousObjectVariable> ReturnOV = nullptr;

	/**
	 * The UGorgeousRPC_OV container that aggregates all cached results for this request.
	 * Populated progressively — present on OnSingleResponderCompleted from the first
	 * responder onwards, and fully populated by the time OnCompleted fires.
	 * Use this when you need the full result set, persistent storage, or Blueprint inspection
	 * of every responder result.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	TObjectPtr<UGorgeousRPC_OV> RPCContainer = nullptr;

	/** Identifies who produced this value (server / which client). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	FGorgeousAutoReplicationRPCResponderHandle Responder;
};

/**
 * Blueprint helper library for working with AutoReplication RPC result structures.
 *
 * Three result types are covered:
 *  - FGorgeousAutoReplicationRPCResult          — a single responder's raw result (TargetVariable, Responder, ...)
 *  - FGorgeousAutoReplicationRPCAsyncResult     — what OnCompleted / OnFailed / OnSingleResponderCompleted delivers (aggregated result set)
 *
 * Use these helpers instead of manually breaking the structs apart — they handle null-safety, provide
 * human-readable descriptions, and make ForEach-style iteration straightforward in Blueprint.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCResultLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// =========================================================================
	// FGorgeousAutoReplicationRPCResult — single-responder raw result
	// =========================================================================

	/**
	 * Returns the object variable that was computed / returned by the RPC handler.
	 * For EOwner, EObjectVariable, and EActorComponent paths this is the handler-written return OV
	 * (e.g. UInteger_SOV) — whichever OV the handler wrote into its first parameter.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static UGorgeousObjectVariable* GetTargetVariable(const FGorgeousAutoReplicationRPCResult& Result);

	/** Returns true when the result carries a valid handler-written target variable. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static bool HasTargetVariable(const FGorgeousAutoReplicationRPCResult& Result);

	/** Returns true when this result was produced by the server-side handler. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static bool IsServerResult(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns the full responder handle identifying who produced this result.
	 * The handle exposes: bIsServer, ConnectionKey, PlayerDisplayName, PlayerControllerId, DebugLabel.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FGorgeousAutoReplicationRPCResponderHandle GetResponder(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns a human-readable description of who produced this result.
	 * Examples: "Server", "Client_0", "Client_1", "<unknown>".
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FString GetResponderDescription(const FGorgeousAutoReplicationRPCResult& Result);

	/** Returns the handler function name that was invoked for this RPC. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FName GetHandlerName(const FGorgeousAutoReplicationRPCResult& Result);

	/** Returns the key that identifies the AutoReplication channel for this RPC. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FName GetRPCKey(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns a short string describing the dispatch target kind.
	 * Examples: "ObjectVariable", "Owner", "ActorComponent", "Auto".
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FString GetTargetKindInfo(const FGorgeousAutoReplicationRPCResult& Result);

	// =========================================================================
	// FGorgeousAutoReplicationRPCAsyncResult — aggregated completion payload
	// (from OnCompleted / OnFailed / OnSingleResponderCompleted)
	// =========================================================================

	/**
	 * Returns the primary (first) result from the aggregated result set.
	 * For Server→Client RPCs the server entry appears first.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static FGorgeousAutoReplicationRPCResult GetPrimaryResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Convenience shortcut: returns the primary handler-written return OV directly.
	 * Equivalent to GetTargetVariable(GetPrimaryResult(AsyncResult)).
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static UGorgeousObjectVariable* GetPrimaryTargetVariable(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Convenience shortcut: returns the responder handle for the primary result directly.
	 * Equivalent to GetResponder(GetPrimaryResult(AsyncResult)).
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static FGorgeousAutoReplicationRPCResponderHandle GetPrimaryResponder(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Looks up the result for a specific responder key (e.g. "Server", "Client_0").
	 * Returns false if no result exists for that key.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool GetResultForResponder(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FString& ResponderKey, FGorgeousAutoReplicationRPCResult& OutResult);

	/** Returns true when a result exists for the given responder key. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool HasResultForResponder(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FString& ResponderKey);

	/**
	 * Looks up the result for a specific responder handle (uses GetStableKey() internally).
	 * Returns false if no result exists for that responder.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool GetResultForResponderHandle(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FGorgeousAutoReplicationRPCResponderHandle& ResponderHandle, FGorgeousAutoReplicationRPCResult& OutResult);

	/** Returns true when a result exists for the given responder handle. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool HasResultForResponderHandle(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FGorgeousAutoReplicationRPCResponderHandle& ResponderHandle);

	/**
	 * Returns every individual responder result as an ordered array (server-first, then clients).
	 * Use this for ForEach iteration in Blueprint.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static TArray<FGorgeousAutoReplicationRPCResult> GetAllResults(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/** Returns all responder keys that produced a result (e.g. ["Server", "Client_0"]). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static TArray<FString> GetAllResponderKeys(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/** Returns the total number of responders whose results are contained in this payload. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static int32 GetResultCount(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns the UGorgeousRPC_OV container that aggregates all results.
	 * Populated progressively on OnSingleResponderCompleted and finalized on OnCompleted.
	 * May be null if the request failed before a container could be created.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static UGorgeousRPC_OV* GetResultContainer(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns true when this is the final delivery — all expected responders have replied
	 * or the request has failed.  Always true on OnCompleted / OnFailed.
	 * On OnSingleResponderCompleted it is true only for the last arriving responder.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult",
		meta = (CompactNodeTitle = "Is Last?"))
	static bool IsLastResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns a progress string indicating how many responders have replied so far.
	 * Example: "1/3" — also useful inside OnSingleResponderCompleted to show partial progress.
	 * Returns "?" when TotalExpectedResponders is 0 (unknown, e.g. multicast).
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult",
		meta = (CompactNodeTitle = "Progress"))
	static FString GetResponderProgress(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns the fraction of received responders relative to total expected (0.0 – 1.0).
	 * Returns 1.0 when TotalExpectedResponders is 0.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult",
		meta = (CompactNodeTitle = "Progress %"))
	static float GetResponderProgressFraction(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	// =========================================================================
	// FGorgeousAutoReplicationRPCValueResult — slim result (shortest path to the value)
	// =========================================================================

	/**
	 * Collapses a raw FGorgeousAutoReplicationRPCResult into the slim FGorgeousAutoReplicationRPCValueResult
	 * (ReturnOV + Responder only). Use when you don't need the full QueuedRPC / TargetKind metadata.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	static FGorgeousAutoReplicationRPCValueResult ToValueResult(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Shortest path to the primary return value from any result payload (OnCompleted,
	 * OnFailed, or OnSingleResponderCompleted).
	 * Equivalent to ToValueResult(GetPrimaryResult(AsyncResult)).
	 * On OnSingleResponderCompleted, Result holds the freshly arrived responder so this
	 * returns the value for that specific responder.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	static FGorgeousAutoReplicationRPCValueResult GetPrimaryValueResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);
	
	/**
	 * Returns every responder's slim result as an ordered array (server-first, then clients).
	 * Use for ForEach iteration when you only care about ReturnOV + Responder per entry.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	static TArray<FGorgeousAutoReplicationRPCValueResult> GetAllValueResults(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);
};
