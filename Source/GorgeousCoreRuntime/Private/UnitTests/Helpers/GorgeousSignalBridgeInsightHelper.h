// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GorgeousSignalBridgeInsightHelper.generated.h"

USTRUCT()
struct FGorgeousTestSignalPayload
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PrimeResult = 0;
};

/**
 * A dummy UObject used by the Signal Bridge Insight test to perform heavy
 * calculations and occasionally dispatch a result back via the bridge.
 */
UCLASS()
class UGorgeousSignalBridgeInsightHelper : public UObject
{
	GENERATED_BODY()

public:
	// Accumulated calculation result to prevent optimization
	UPROPERTY()
	int32 HitCount = 0;
	
	// Number of results sent back
	UPROPERTY()
	int32 ResultsDispatched = 0;

	// Total calculations successfully returned and verified
	UPROPERTY()
	int64 TotalVerifiedCalculations = 0;

	// Reference to the bridge to send results back
	UPROPERTY()
	USignalBridgeStorage_OV* BridgeRef = nullptr;

	/** Handles the incoming test signal */
	UFUNCTION()
	void HandleTestSignal(FGameplayTag SignalTag, const FInstancedStruct& Payload);
	
	/** Handles the returned sub-dispatch signal */
	UFUNCTION()
	void HandleTestResult(FGameplayTag SignalTag, const FInstancedStruct& Payload);
};
