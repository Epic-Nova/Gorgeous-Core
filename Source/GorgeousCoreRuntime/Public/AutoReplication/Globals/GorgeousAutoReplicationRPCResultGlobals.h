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
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreRuntimeGlobals.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousAutoReplicationRPCResultGlobals.generated.h"
//<-------------------------------------------------------------------------->

// @todo: when writing the doc. then note that the rpc dispatching is somehow wierd in consitency. but we are working on it to fix this

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
 * 
 * @author Nils Bergemann
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

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Auto Replication RPC Result Globals
| Functional Name: UGorgeousAutoReplicationRPCResultGlobals
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Blueprint helper library for working with AutoReplication RPC result structures.
|
| Use these helpers instead of manually breaking the structs apart,
| they handle null-safety, provide human-readable descriptions, 
| and make ForEach-style iteration straightforward in Blueprint.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCResultGlobals : public UGorgeous
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
public:

	// =========================================================================
	// FGorgeousAutoReplicationRPCResult — single-responder raw result
	// =========================================================================

	/**
	 * Returns the object variable that was computed / returned by the RPC handler.
	 * For EOwner, EObjectVariable, and EActorComponent paths this is the handler-written return OV (e.g. UInteger_SOV) — whichever OV the handler wrote into its first parameter.
	 * 
	 * @param Result The raw result struct from a single responder (e.g. from GetPrimaryResult or GetResultForResponder).
	 * @return The handler-written return OV for EOwner/EObjectVariable/EActorComponent target kinds, 
	 * or the internal RPC_OV container for EAuto target kind. May be null if the handler returned nothing or if this is an intermediate result for an EAuto dispatch.
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
	 * Use this when you need to know exactly which client produced the result, or when you need to perform lookups by responder handle (e.g. with GetResultForResponderHandle).
	 * 
	 * @param Result The raw result struct from a single responder (e.g. from GetPrimaryResult or GetResultForResponder).
	 * @return The full responder handle for this result, which includes detailed information about the responder.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FGorgeousAutoReplicationRPCResponderHandle GetResponder(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns a human-readable description of who produced this result, for debugging purposes.
	 * Examples: "Server", "Client_0", "Client_1", "<unknown>".
	 * 
	 * @param Result The raw result struct from a single responder (e.g. from GetPrimaryResult or GetResultForResponder).
	 * @return A human-readable string describing the responder who produced this result, useful for debugging and logging.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FString GetResponderDescription(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns the name of the handler function that was called to produce this result.
	 * 
	 * @param Result The raw result struct from a single responder (e.g. from GetPrimaryResult or GetResultForResponder).
	 * @return The name of the handler function that processed this RPC and produced this result.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FName GetHandlerName(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns the AutoReplication channel key associated with this RPC result. The key is determined by the dispatching system based on the target's properties and the RPC's configuration.
	 * 
	 * @param Result The raw result struct from a single responder (e.g. from GetPrimaryResult or GetResultForResponder).
	 * @return The FName key that identifies the AutoReplication channel for this RPC, which can be used for debugging, logging, or conditional logic based on the channel.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FName GetRPCKey(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns a short string describing the dispatch target kind for this RPC result.
	 * 
	 * @param Result The raw result struct from a single responder (e.g. from GetPrimaryResult or GetResultForResponder).
	 * @return A human-readable string describing the dispatch target kind for this RPC result, such
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|Result")
	static FString GetTargetKindInfo(const FGorgeousAutoReplicationRPCResult& Result);

	// =========================================================================
	// FGorgeousAutoReplicationRPCAsyncResult — aggregated completion payload
	// (from OnCompleted / OnFailed / OnSingleResponderCompleted)
	// =========================================================================

	/**
	 * Convenience shortcut for getting the primary result from the aggregated async result payload.
 	 * The primary result is defined as the first responder's result in the aggregated set, 
	 * which is typically the server for Server→Client RPCs. 
	 * Use this when you just need to access the main result without iterating through all responders.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return The primary result struct from the aggregated result set, which contains the handler-written return value and metadata for the first responder.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static FGorgeousAutoReplicationRPCResult GetPrimaryResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns the primary handler-written return OV directly from the aggregated async result payload.
	 * This is a shortcut for GetTargetVariable(GetPrimaryResult(AsyncResult)) and provides direct access to the main return value without needing to first get the primary result struct.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return The primary handler-written return OV for the first responder in the aggregated result set
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static UGorgeousObjectVariable* GetPrimaryTargetVariable(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns the responder handle for the primary result directly from the aggregated async result payload.
	 * This is a shortcut for GetResponder(GetPrimaryResult(AsyncResult)) and provides direct access to the main responder's handle without needing to first get the primary result struct.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return The responder handle for the primary result, which identifies who produced the main result
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static FGorgeousAutoReplicationRPCResponderHandle GetPrimaryResponder(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Looks up the result for a specific responder key (e.g. "Server", "Client_0") in the aggregated async result payload.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @param ResponderKey The responder key to look up (e.g. "Server", "Client_0"). This is a human-readable identifier for the responder, not the full responder handle.
	 * @param OutResult The output parameter that will hold the result struct for the specified responder if found.
	 * @return True if a result exists for the given responder key and was successfully retrieved, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool GetResultForResponder(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FString& ResponderKey, FGorgeousAutoReplicationRPCResult& OutResult);

	/**
	 * Checks if a result exists for a specific responder key (e.g. "Server", "Client_0") in the aggregated async result payload.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @param ResponderKey The responder key to check for (e.g. "Server", "Client_0"). This is a human-readable identifier for the responder, not the full responder handle.
	 * @return True if a result exists for the given responder key, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool HasResultForResponder(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FString& ResponderKey);

	/**
	 * Looks up the result for a specific responder handle in the aggregated async result payload.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @param ResponderHandle The full responder handle to look up, which includes detailed information about the responder.
	 * @param OutResult The output parameter that will hold the result struct for the specified responder handle if found.
	 * @return True if a result exists for the given responder handle and was successfully retrieved, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool GetResultForResponderHandle(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FGorgeousAutoReplicationRPCResponderHandle& ResponderHandle, FGorgeousAutoReplicationRPCResult& OutResult);

	/** Returns true when a result exists for the given responder handle. */
	/**
	 * Checks if a result exists for a specific responder handle in the aggregated async result payload.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @param ResponderHandle The full responder handle to check for, which includes detailed information about the responde.
	 * @return True if a result exists for the given responder handle, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static bool HasResultForResponderHandle(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult, const FGorgeousAutoReplicationRPCResponderHandle& ResponderHandle);

	/**
	 * Returns every individual responder result as an ordered array (server-first, then clients) from the aggregated async result payload.
	 * This provides access to the full set of raw results for all responders, which can be useful for iteration, debugging, or when you need to work with responders beyond just the primary one.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return An ordered array of raw result structs for each responder, with the server's result first (if present), followed by client results in an unspecified order.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static TArray<FGorgeousAutoReplicationRPCResult> GetAllResults(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns an array of all responder keys that have produced a result in the aggregated async result payload. Responder keys are human-readable identifiers for responders, such as "Server" or "Client_0".
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return An array of strings representing the responder keys for all responders that have produced a result in this payload. Examples include "Server", "Client_0", "Client_1", etc.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static TArray<FString> GetAllResponderKeys(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns the total count of responders that have produced results in the aggregated async result payload. 
	 * This indicates how many individual responder results are included in this payload, which can be useful for tracking progress, displaying counts, or validating that all expected responders have replied.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return The total number of responders whose results are contained in this payload.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static int32 GetResultCount(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns the UGorgeousRPC_OV container that aggregates all results for this RPC request. 
	 * This container is populated progressively as responders reply, starting from the first arriving responder on OnSingleResponderCompleted, 
	 * and is fully populated by the time OnCompleted fires. 
	 * The container provides access to the complete set of results for all responders and can be used for persistent storage or detailed inspection in Blueprint. 
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return The UGorgeousRPC_OV container that holds all cached results for this RPC request.
	 * @note This may be null if the request failed before a container could be created, so it's important to check for null before using it.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult")
	static UGorgeousRPC_OV* GetResultContainer(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns true if this AsyncResult represents the final delivery for the RPC request, 
	 * meaning that all expected responders have replied or the request has failed. 
	 * This will always return true when called from an OnCompleted or OnFailed event, 
	 * as those events signify the completion of the request. However, when called from an OnSingleResponderCompleted event, 
	 * it will return true only if the responder that just completed is the last one expected to reply, 
	 * indicating that this is the final result being delivered for this request.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return True if this is the final delivery for the RPC request (all responders have
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult",
		meta = (CompactNodeTitle = "Is Last?"))
	static bool IsLastResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns a progress string indicating how many responders have replied so far in the aggregated async result payload.
	 * The format of the string is "X/Y", where X is the number of responders that have produced results so far, and Y is the total number of expected responders for this RPC request.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return A string in the format "X/Y" representing the number of responders that have replied (X) out of the total expected responders (Y). If the total expected responders is unknown, returns "X/?".
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|AsyncResult",
		meta = (CompactNodeTitle = "Progress"))
	static FString GetResponderProgress(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);

	/**
	 * Returns the fraction of responders that have produced results so far in the aggregated async result payload, relative to the total number of expected responders for this RPC request.
	 * The return value is a float between 0.0 and 1.0, where 0.0 means no responders have replied yet, and 1.0 means all expected responders have replied.
	 * If the total number of expected responders is zero (which can happen if the system cannot determine how many responders to expect), this function will return 1.0 to indicate that there are no pending responders.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return A float between 0.0 and 1.0 representing the fraction of responders that have produced results so far (X) relative to the total expected responders (Y). Returns 1.0 if Y is 0.
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
	/**
	 * Converts a raw FGorgeousAutoReplicationRPCResult struct into a slim FGorgeousAutoReplicationRPCValueResult struct that contains only the essential information needed to work with an RPC return value.
	 * 
	 * @param Result The raw result struct from a single responder (e.g. from GetPrimaryResult or GetResultForResponder).
	 * @return A FGorgeousAutoReplicationRPCValueResult struct that contains the handler-written return OV and the responder handle, providing a simplified view of the result focused on the return value and its producer.
	 * @note This function extracts only the ReturnOV and Responder information from the raw result, 
	 * and does not include the full metadata such as HandlerName, RPCKey, or TargetKindInfo.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	static FGorgeousAutoReplicationRPCValueResult ToValueResult(const FGorgeousAutoReplicationRPCResult& Result);

	/**
	 * Returns the primary handler-written return value as a slim FGorgeousAutoReplicationRPCValueResult struct directly from the aggregated async result payload delivered by OnCompleted, OnFailed, or OnSingleResponderCompleted.
	 * This is a convenience function that provides the shortest path to access the main return value and its producer without needing to first get the primary result struct and then convert it.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return A FGorgeousAutoReplicationRPCValueResult struct containing the primary handler-written return OV and the responder handle for the first responder in the aggregated result set. 
	 * When called from OnSingleResponderCompleted, this will return the value for the specific responder that just completed.
	 * @note This function is equivalent to calling ToValueResult(GetPrimaryResult(AsyncResult)), but provides a direct shortcut for convenience.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	static FGorgeousAutoReplicationRPCValueResult GetPrimaryValueResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);
	
	/**
	 * Returns an ordered array of slim FGorgeousAutoReplicationRPCValueResult structs for each responder in the aggregated async result payload, with the server's result first (if present), followed by client results in an unspecified order.
	 * This provides a simplified view of the results focused on the handler-written return OV and the responder handle for each responder, which can be useful for iteration in Blueprint when you don't need the full raw result metadata.
	 * 
	 * @param AsyncResult The aggregated async result payload delivered by OnCompleted / OnFailed / OnSingleResponderCompleted.
	 * @return An ordered array of FGorgeousAutoReplicationRPCValueResult structs for each responder, containing only the handler-written return OV and the responder handle for each responder in the aggregated result set.
	 * @note This function extracts only the ReturnOV and Responder information for each responder, and does not include the full metadata such as HandlerName, RPCKey, or TargetKindInfo. Use GetAllResults if you need the full raw result structs instead.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC|ValueResult")
	static TArray<FGorgeousAutoReplicationRPCValueResult> GetAllValueResults(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult);
	//<------------------------------------------------------------------------->
};
