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
	
	/**
	 * Weather the current state should be cleared or if this state should be additive to the current one
	 * 
	 * If true, all previously applied InputMappingContexts will not be cleared & this new state serves as an addition to the current one.
	 * If false, clears all current states and applies this one.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	bool bAdditiveToCurrentState;
	
	/**
	 * Weather the current theme should be cleared or if the theme being applied should be additive to the current one.
	 * 
	 * If true, the new theme will be applied on top of the current one instead of replacing it.
	 * If false, clear all current themes and applies this one.
	 * This allows you to layer themes on top of each other (e.g. a "Danger" theme on top of the base "Combat" theme).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	bool bAdditiveToCurrentTheme;
	

	/** Optional animation to play when entering this state globally. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName TransitionInAnimation;

	/** Overlay configuration for this state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UGorgeousUIOverlayConfig_DA* OverlayConfig;

	/** Explicit focus routing boundaries for this UI state (e.g. escaping auto-navigation). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State", meta = (DisplayAfter = "OverlayConfig"))
	TArray<FGorgeousFocusRoute_S> FocusRoutes;

	/** Fallback configuration for what to focus if no breadcrumb or default exists. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State", meta = (DisplayAfter = "FocusRoutes"))
	FGorgeousFocusFallbackConfig_S FocusFallback;

	/** Optional theme to apply when this state is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI State")
	class UGorgeousUITheme_DA* Theme;

	/**
	 * Tag-to-Action bindings to activate with this state.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UGorgeousInputBinding_DA* InputBindings;
};
