// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GorgeousUIExtensions.generated.h"

class UCommonActivatableWidget;
class ULocalPlayer;
class APlayerController;

/**
 * High-level Blueprint extensions for the Gorgeous Core|Common UI Foundation system.
 * Inspired by Lyra's CommonUIExtensions but supercharged with Themes and Signals.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousUIExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the Gorgeous HUD for the primary player. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static class AGorgeousHUD* GetGorgeousHUD(const UObject* WorldContextObject);
	
public:
	/** 
	 * Returns the correct icon brush for an input action, automatically filtered by:
	 * 1. The current Input Device (Gamepad vs. M&K)
	 * 2. The current Global UI Theme
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static FSlateBrush GetGorgeousActionIcon(UObject* WorldContextObject, FGameplayTag ActionTag);

	/**
	 * Pushes a widget to a specific layer for a player.
	 * Returns the widget instance if immediately successful.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static UCommonActivatableWidget* PushGorgeousWidget(UObject* WorldContextObject, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass, ULocalPlayer* LocalPlayer = nullptr);

	/**
	 * Pushes a widget to a specific layer for a player asynchronously.
	 * Handles asset loading and input suspension automatically.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static void PushGorgeousWidgetAsync(UObject* WorldContextObject, FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass, bool bSuspendInput = true, ULocalPlayer* LocalPlayer = nullptr);

	/**
	 * Suspends input for a player and returns a token.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions")
	static FName SuspendGorgeousInput(APlayerController* PlayerController, FName Reason);

	/**
	 * Resumes input for a player using a previously returned token.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions")
	static void ResumeGorgeousInput(APlayerController* PlayerController, FName Token);

	/** Calculates an eased alpha based on the interpolation type. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Math")
	static float CalculateEasedAlpha(float InAlpha, EGorgeousUIInterpType_E InterpType);


	/**
	 * Sets the hold time threshold for an input trigger. Useful for dynamically adjusting input responsiveness based on game state or player preferences.
	 * 
	 * @param Trigger The input trigger to modify. Must be of type UInputTriggerHold or a subclass thereof.
	 * @param HoldTimeThreshold The new hold time threshold in seconds. Must be non-negative
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Extensions|Input", meta = (WorldContext = "WorldContextObject"))
	static void SetInputTriggerHoldTimeThreshold(class UInputTrigger* Trigger, const float& HoldTimeThreshold);
};
