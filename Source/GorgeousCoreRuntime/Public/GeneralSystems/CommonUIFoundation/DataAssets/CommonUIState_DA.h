// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "CommonUIState_DA.generated.h"

/**
 * Defines a specific UI state (e.g., Combat, Exploration).
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UCommonUIState_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique tag identifying this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	FGameplayTag StateTag;

	/** Friendly name for the state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	FText StateName;

	/** Optional animation to play when entering this state globally. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName TransitionInAnimation;

	/** Overlay configuration for this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UCommonUIOverlayConfig_DA* OverlayConfig;

	/** Optional theme to apply when this state is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UGorgeousUITheme_DA* Theme;

	/** 
	 * Input Mapping Contexts to push when this state is active.
	 * Automatically managed by the Subsystem.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<FGorgeousInputMappingConfig_S> InputMappingContexts;
};
