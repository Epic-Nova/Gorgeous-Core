// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GorgeousActivatableWidget.generated.h"

class UGorgeousUITheme_DA;
class UCommonUIState_DA;

/**
 * Base activatable widget for all Gorgeous menus and overlays.
 * Automatically participates in the theme/processor system.
 *
 * Use this as the base for any widget that lives on a layer stack
 * (pause menus, inventory screens, HUD overlays, etc.).
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousActivatableWidget : public UCommonActivatableWidget, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	/** Tag used to identify this widget for signal-driven updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Interp speed for theme color transitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Juicy")
	float ThemeInterpSpeed = 5.0f;

	/**
	 * Called when a new theme is applied.
	 * Implement in Blueprints to visually respond to theme changes.
	 */

	/**
	 * Called when the active UI state changes (Combat, Menu, etc.).
	 * Map states to animation names to trigger automatic transitions.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI")
	void OnStateSwitched(UCommonUIState_DA* NewState);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
