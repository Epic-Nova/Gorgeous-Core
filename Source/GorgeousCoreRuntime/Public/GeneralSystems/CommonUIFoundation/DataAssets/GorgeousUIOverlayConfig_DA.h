// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CommonUIOverlayConfig_DA.generated.h"

class UGorgeousUIState_DA;

/**
 * Configuration for a specific widget's behavior in a given state.
 */
USTRUCT(BlueprintType)
struct FGorgeousUIStateConfig
{
	GENERATED_BODY()

	/** The state this configuration applies to. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	UGorgeousUIState_DA* State = nullptr;

	/** Whether the widget should be visible in this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bIsVisible = true;

	/** Opacity of the widget in this state (0.0 to 1.0). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Opacity = 1.0f;

	/** Optional animation name to play when switching to this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName AnimationName; //@TODO: Not wired up
};

/**
 * Configuration for an overlay, defining how its child elements behave in various states.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UCommonUIOverlayConfig_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Map of Widget Tags to their state-specific configurations. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay")
	TMap<FGameplayTag, FGorgeousUIStateConfig> WidgetConfigs;
};
