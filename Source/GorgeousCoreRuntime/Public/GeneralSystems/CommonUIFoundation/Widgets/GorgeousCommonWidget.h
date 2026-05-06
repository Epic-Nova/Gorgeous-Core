// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonWidget.generated.h"

class UGorgeousUIProcessor;
class UGorgeousUIState_DA;

/**
 * Base widget for the Gorgeous UI system.
 * Inherits from CommonUserWidget for Common UI compatibility.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousCommonWidget : public UCommonUserWidget, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied"))
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme);

	/** Play an animation on this widget by name. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Animation")
	void PlayAnimationByName(const FName& AnimName);

	/** Binding Tag for Signal Bridge routing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Optional Action Tag for this widget (e.g. UI.Action.Confirm). Used for dynamic icon swapping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag ActionTag;

	/** Updates the widget's icon based on the current theme and input method. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void UpdateActionIcon();

	/**
	 * Map of UI States to animation names.
	 * Played automatically when the system switches states.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	TMap<UGorgeousUIState_DA*, FName> StateAnimations;

	/** Called when the global UI state changes. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI")
	void OnUIStateChanged(UGorgeousUIState_DA* NewState);

	// IGorgeousUIWidget_I interface
	// End of IGorgeousUIWidget_I interface

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
};
