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

//<=============================--- Includes ---============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIOverlayConfig_DA.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Defines the interpolation used for a UI transition.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousUIInterpType_E : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut
};

/**
 * Configures a UI interpolation transition.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousUIInterpConfig_S
{
	GENERATED_BODY()

	// Duration of the transition in seconds.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interp")
	float Duration = 0.25f;

	// The easing function to use.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interp")
	EGorgeousUIInterpType_E InterpType = EGorgeousUIInterpType_E::EaseInOut;

	// Optional custom curve to drive the transition. If set, overrides InterpType.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interp")
	FRuntimeFloatCurve InterpCurve;
};

/**
 * Configures a specific widget's behavior in a UI state.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousUIStateConfig
{
	GENERATED_BODY()

	// The state this configuration applies to.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	class UGorgeousUIState_DA* State = nullptr;

	// Whether the widget should be visible in this state.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bIsVisible = true;

	// Whether to use the automated opacity interpolation. Set to false to handle transitions manually.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bEnableOpacityInterp = true;

	// Target opacity of the widget in this state (0.0 to 1.0).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableOpacityInterp"))
	float Opacity = 1.0f;

	// How to animate the opacity change.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (EditCondition = "bEnableOpacityInterp"))
	FGorgeousUIInterpConfig_S OpacityInterp;

	// Whether to use automated theme color interpolation for this widget in this state.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bEnableThemeInterp = true;

	// Optional animation name to play when switching to this state.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName AnimationName;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UI Overlay Config
| Functional Name: UGorgeousUIOverlayConfig_DA
| Parent Class: UPrimaryDataAsset
| Class Suffix: _DA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Configures how overlay child widgets behave across UI states, including
| visibility, opacity, theme interpolation, and state-specific animations.
<--------------------------------------------------------------------------->
<==========================================================================>
 */
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIOverlayConfig_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// Map of Widget Tags to their state-specific configurations.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
	TMap<FGameplayTag, FGorgeousUIStateConfig> WidgetConfigs;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};