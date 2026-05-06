// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/CommonUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonWidget.generated.h"

class UGorgeousUIProcessor;
class UCommonUIState_DA;

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

	/** Play an animation on this widget by name. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Animation")
	void PlayAnimationByName(const FName& AnimName);
	/** 
	 * Tag used to identify this widget for Signal Bridge updates.
	 * If empty, this widget will not receive dynamic updates via processors.
	 */
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
	TMap<UCommonUIState_DA*, FName> StateAnimations;

	/** Called when the UI state is switched. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI")
	void OnStateSwitched(UCommonUIState_DA* NewState);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Map of colors currently being interpolated. */
	TMap<FName, FLinearColor> CurrentThemeColors;
	TMap<FName, FLinearColor> TargetThemeColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Juicy")
	float ThemeInterpSpeed = 5.0f;

	/** Whether we are currently interpolating theme colors. */
	bool bIsInterpTheme = false;



	virtual void NativeConstruct();
	virtual void NativeDestruct();
};
