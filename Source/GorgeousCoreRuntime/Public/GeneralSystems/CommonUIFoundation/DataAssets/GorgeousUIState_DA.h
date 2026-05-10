// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
#include "GorgeousUIState_DA.generated.h"

/**
 * Defines a specific UI state (e.g., Combat, Exploration).
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousUIState_DA : public UGorgeousPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** UGorgeousPrimaryDataAsset Interface */
	virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("UI_State"); }
	virtual TArray<FString> GetDefaultScanPaths() const override { return { TEXT("UserInterfaces/States") }; }
	/** Unique tag identifying this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State", meta = (Categories = "UI.State"))
	FGameplayTag StateTag;

	/** Friendly name for the state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	FText StateName;

	/** Optional animation to play when entering this state globally. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName TransitionInAnimation;

	/** Overlay configuration for this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UGorgeousUIOverlayConfig_DA* OverlayConfig;

	/** Optional theme to apply when this state is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UGorgeousUITheme_DA* Theme;

	/** 
	 * Input Mapping Contexts to push when this state is active.
	 * Automatically managed by the Subsystem.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<FGorgeousInputMappingConfig_S> InputMappingContexts;

	/**
	 * Tag-to-Action bindings to activate with this state.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UGorgeousInputBinding_DA* InputBindings;
};
