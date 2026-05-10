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
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousCommonWidget : public UCommonUserWidget, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	UGorgeousCommonWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied"))
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme);

	/** Play an animation on this widget by name. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Animation")
	void PlayAnimationByName(const FName& AnimName);

	/** Binding Tag for Signal Bridge routing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Routing ID for this widget instance (e.g. Slot Index). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FName RoutingID;

	/** Enables the per-widget style allow list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	bool bUseStylePropertyAllowList = true;

	/** Properties that can be styled by themes or Signal Bridge payloads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	TSet<FName> StylePropertyAllowList;

	/** Optional Action Tag for this widget (e.g. UI.Action.Confirm). Used for dynamic icon swapping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag ActionTag;

	/** Updates the widget's icon based on the current theme and input method. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void UpdateActionIcon(const UGorgeousUITheme_DA* ThemeOverride = nullptr);

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
	virtual FName GetRoutingID() const override { return RoutingID; }
	virtual void SetRoutingID(FName InID) override { RoutingID = InID; }
	// End of IGorgeousUIWidget_I interface

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
