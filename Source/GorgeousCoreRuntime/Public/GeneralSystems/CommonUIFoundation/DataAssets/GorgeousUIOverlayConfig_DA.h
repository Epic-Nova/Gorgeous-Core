// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Curves/CurveFloat.h"
#include "GorgeousUIOverlayConfig_DA.generated.h"

/** Types of interpolation for UI transitions. */
UENUM(BlueprintType)
enum class EGorgeousUIInterpType_E : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut
};

/** Configuration for a UI interpolation transition. */
USTRUCT(BlueprintType)
struct FGorgeousUIInterpConfig_S
{
	GENERATED_BODY()

	/** Duration of the transition in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interp")
	float Duration = 0.25f;

	/** The easing function to use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interp")
	EGorgeousUIInterpType_E InterpType = EGorgeousUIInterpType_E::EaseInOut;

	/** Optional custom curve to drive the transition. If set, overrides InterpType. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interp")
	FRuntimeFloatCurve InterpCurve;
};

/**
 * Configuration for a specific widget's behavior in a given state.
 */
USTRUCT(BlueprintType)
struct FGorgeousUIStateConfig
{
	GENERATED_BODY()

	/** The state this configuration applies to. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	class UGorgeousUIState_DA* State = nullptr;

	/** Whether the widget should be visible in this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bIsVisible = true;

	/** Whether to use the automated opacity interpolation. Set to false to handle transitions manually. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bEnableOpacityInterp = true;

	/** Target opacity of the widget in this state (0.0 to 1.0). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableOpacityInterp"))
	float Opacity = 1.0f;

	/** How to animate the opacity change. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (EditCondition = "bEnableOpacityInterp"))
	FGorgeousUIInterpConfig_S OpacityInterp;

	/** Whether to use automated theme color interpolation for this widget in this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bEnableThemeInterp = true;

	/** Optional animation name to play when switching to this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName AnimationName;
};

/**
 * Configuration for an overlay, defining how its child elements behave in various states.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousUIOverlayConfig_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Map of Widget Tags to their state-specific configurations. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
	TMap<FGameplayTag, FGorgeousUIStateConfig> WidgetConfigs;
};
