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

#include "ObjectVariables/GorgeousObjectVariable.h"

#include "GorgeousRPC_OV.generated.h"

/** Lightweight container that captures the outcome of a AutoReplication RPC invocation. */
UCLASS(BlueprintType, EditInlineNew, DisplayName = "AutoReplication RPC OV", Category = "Gorgeous Core|AutoReplication|Networking")
class GORGEOUSCORERUNTIME_API UGorgeousRPC_OV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:
	UGorgeousRPC_OV();

	/** Copies the provided RPC metadata and resolved target into this container. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking")
	void CaptureResult(const FGorgeousAutoReplicationRPCResult& InResult);

	/** Copies an entire per-responder result set into this container. */
	void CaptureResultSet(const TMap<FString, FGorgeousAutoReplicationRPCResult>& InResults, const TArray<FGorgeousAutoReplicationRPCResult>& OrderedResults);

	/** Clears the cached data so this container can be reused. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking")
	void ResetResult();

	/** Returns true when a AutoReplication RPC result has been cached. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	bool HasResult() const { return bHasCachedResult; }

	/** Returns the cached RPC metadata (array index 0). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	const FGorgeousAutoReplicationRPCResult& GetCachedResult() const;

	/** Returns the cached object-variable target, if any (array index 0). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	UGorgeousObjectVariable* GetCachedTargetVariable() const { return HasResult() ? CachedResults[0].TargetVariable : nullptr; }

	/** Returns the QoL owner that executed the RPC when applicable (array index 0). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	UObject* GetCachedTargetOwner() const { return HasResult() ? CachedResults[0].TargetOwner : nullptr; }

	/** Returns how the payload was ultimately handled (array index 0). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	EGorgeousAutoReplicationTargetKind GetCachedTargetKind() const { return HasResult() ? CachedResults[0].TargetKind : EGorgeousAutoReplicationTargetKind::EAuto; }

	/** Returns every cached responder result captured by this container. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	const TArray<FGorgeousAutoReplicationRPCResult>& GetCachedResults() const { return CachedResults; }

	/** Returns the keyed responder map for lookup by connection key. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	const TMap<FString, FGorgeousAutoReplicationRPCResult>& GetCachedResultMap() const { return CachedResultMap; }

	/** Attempts to fetch a responder result by connection key. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	bool GetResultByConnectionKey(const FString& ConnectionKey, FGorgeousAutoReplicationRPCResult& OutResult) const;

	/**
	 * Returns the raw argument container stored on the cached payload.
	 * Use UGorgeousAutoReplicationRPCPayloadLibrary::GetArgumentValue (CustomThunk) in
	 * Blueprint to extract the typed value from the container.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|Networking")
	bool GetArgumentContainerByName(FName ArgumentName, FGorgeousRPCArgumentContainer& OutContainer) const;


private:
	/** Cached list of all responder results captured for this request. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous AutoReplication|Networking", meta = (AllowPrivateAccess = "true"))
	TArray<FGorgeousAutoReplicationRPCResult> CachedResults;

	/** Keyed responder map for quick connection lookups. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous AutoReplication|Networking", meta = (AllowPrivateAccess = "true"))
	TMap<FString, FGorgeousAutoReplicationRPCResult> CachedResultMap;

	/** Tracks whether the cached data is valid. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous AutoReplication|Networking", meta = (AllowPrivateAccess = "true"))
	bool bHasCachedResult;

	static const FGorgeousAutoReplicationRPCResult& GetEmptyResult();
};
