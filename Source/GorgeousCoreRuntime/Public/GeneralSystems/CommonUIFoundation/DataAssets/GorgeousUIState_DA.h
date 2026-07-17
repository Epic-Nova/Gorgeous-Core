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
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIState_DA.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIState
| Functional Name: UGorgeousUIState_DA
| Parent Class: UGorgeousPrimaryDataAsset
| Class Suffix: _DA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Defines a specific UI state (e.g., Combat, Exploration).
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIState_DA : public UGorgeousPrimaryDataAsset
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Returns the primary asset type for UI states.
	virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("UI_State"); }
	// Returns the content paths scanned for UI states.
	virtual TArray<FString> GetDefaultScanPaths() const override { return { TEXT("UserInterfaces/States") }; }
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// Keeps existing input mapping contexts when this state is applied.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	bool bAdditiveToCurrentState;

	// Layers this state's theme over the active themes instead of replacing them.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	bool bAdditiveToCurrentTheme;


	// Optional animation to play when entering this state globally.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName TransitionInAnimation;

	// Overlay configuration for this state.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UGorgeousUIOverlayConfig_DA* OverlayConfig;

	// Explicit focus routing boundaries for this UI state (e.g. escaping auto-navigation).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State", meta = (DisplayAfter = "OverlayConfig"))
	TArray<FGorgeousFocusRoute_S> FocusRoutes;

	// Fallback configuration for what to focus if no breadcrumb or default exists.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State", meta = (DisplayAfter = "FocusRoutes"))
	FGorgeousFocusFallbackConfig_S FocusFallback;

	// Optional theme to apply when this state is active.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UGorgeousUITheme_DA* Theme;

	// Tag-to-Action bindings to activate with this state.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UGorgeousInputBinding_DA* InputBindings;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};