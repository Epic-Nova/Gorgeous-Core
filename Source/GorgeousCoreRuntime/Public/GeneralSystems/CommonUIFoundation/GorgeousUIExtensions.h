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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIExtensions.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UCommonActivatableWidget;
class ULocalPlayer;
class APlayerController;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIExtensions
| Functional Name: UGorgeousUIExtensions
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| High-level Blueprint extensions for the Gorgeous Core|Common UI Foundation
| system. Inspired by Lyra's CommonUIExtensions but supercharged with Themes
| and Signals.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousUIExtensions",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Resolves the Gorgeous HUD for the primary player.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @return The primary player's Gorgeous HUD, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static class AGorgeousHUD* GetGorgeousHUD(const UObject* WorldContextObject);
	/**
	 * Returns the correct icon brush for an input action, automatically filtered by:
	 * 1. The current Input Device (Gamepad vs. M&K)
	 * 2. The current Global UI Theme
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @param ActionTag The input action for which to resolve an icon.
	 * @return The brush matching the action, active input device, and theme.
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static FSlateBrush GetGorgeousActionIcon(UObject* WorldContextObject, FGameplayTag ActionTag);

	/**
	 * Pushes a widget to a specific layer for a player.
	 * Returns the widget instance if immediately successful.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @param LayerTag The layer that receives the widget.
	 * @param WidgetClass The widget class to create.
	 * @param LocalPlayer The optional player that owns the widget.
	 * @return The pushed widget, or null when it cannot be created immediately.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static UCommonActivatableWidget* PushGorgeousWidget(UObject* WorldContextObject, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass, ULocalPlayer* LocalPlayer = nullptr);

	/**
	 * Pushes a widget to a specific layer for a player asynchronously.
	 * Handles asset loading and input suspension automatically.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @param LayerTag The layer that receives the widget.
	 * @param WidgetClass The widget class to load and create.
	 * @param bSuspendInput Whether input remains suspended while loading.
	 * @param LocalPlayer The optional player that owns the widget.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (WorldContext = "WorldContextObject"))
	static void PushGorgeousWidgetAsync(UObject* WorldContextObject, FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass, bool bSuspendInput = true, ULocalPlayer* LocalPlayer = nullptr);

	/**
	 * Suspends input for a player and returns a token.
	 *
	 * @param PlayerController The controller whose input is suspended.
	 * @param Reason The reason recorded for the suspension.
	 * @return The token required to resume the suspended input.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions")
	static FName SuspendGorgeousInput(APlayerController* PlayerController, FName Reason);

	/**
	 * Resumes input for a player using a previously returned token.
	 *
	 * @param PlayerController The controller whose input is resumed.
	 * @param Token The token returned when input was suspended.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions")
	static void ResumeGorgeousInput(APlayerController* PlayerController, FName Token);

	/**
	 * Calculates an eased alpha using the requested interpolation type.
	 *
	 * @param InAlpha The linear alpha to transform.
	 * @param InterpType The interpolation method to apply.
	 * @return The transformed alpha.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Math")
	static float CalculateEasedAlpha(float InAlpha, EGorgeousUIInterpType_E InterpType);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};