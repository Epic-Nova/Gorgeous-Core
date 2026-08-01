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
//<--------------------------=== Module Includes ===------------------------->
#include "UObject/NoExportTypes.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousSignalBridgeInsightHelper.generated.h"
//<-------------------------------------------------------------------------->

USTRUCT()
struct FGorgeousTestSignalPayload
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PrimeResult = 0;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Signal Bridge Insight Helper
| Functional Name: UGorgeousSignalBridgeInsightHelper
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A dummy UObject used by the Signal Bridge Insight test to perform heavy
| calculations and occasionally dispatch a result back via the bridge.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/GorgeousSignalBridgeInsightHelper",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Examples/"
		)
)
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