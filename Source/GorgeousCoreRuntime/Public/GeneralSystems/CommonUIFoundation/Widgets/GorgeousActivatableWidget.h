// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GorgeousActivatableWidget.generated.h"

class UGorgeousUITheme_DA;
class UGorgeousUIState_DA;

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

	UGorgeousActivatableWidget(const FObjectInitializer& ObjectInitializer);

	/** Binding Tag for Signal Bridge routing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Enables the per-widget style allow list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	bool bUseStylePropertyAllowList = true;

	/** Properties that can be styled by themes or Signal Bridge payloads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	TSet<FName> StylePropertyAllowList;

	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied"))
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme);

	/** 
	 * If set, the UI system will automatically switch to this state when this widget is activated.
	 * Ideal for Inventory, Menus, or Dialogs that should drive the global UI state.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	TObjectPtr<class UGorgeousUIState_DA> AutoState;
	
	/** If true, the system will attempt to return to the previous state when this widget is deactivated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	bool bRevertStateOnDeactivation = false;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	/** Track the state we were in before this widget took over. */
	UPROPERTY()
	TObjectPtr<class UGorgeousUIState_DA> PreviousState;

public:	/** Play an animation on this widget by name. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Animation")
	void PlayAnimationByName(const FName& AnimName);

	/**
	 * Map of UI States to animation names.
	 * Played automatically when the system switches states.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	TMap<class UGorgeousUIState_DA*, FName> StateAnimations;

	/** Called when the global UI state changes. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI")
	void OnUIStateChanged(UGorgeousUIState_DA* NewState);

	// IGorgeousUIWidget_I interface
	// End of IGorgeousUIWidget_I interface

protected:


protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
