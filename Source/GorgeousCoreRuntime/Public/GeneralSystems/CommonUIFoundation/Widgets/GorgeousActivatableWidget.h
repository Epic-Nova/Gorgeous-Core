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
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousActivatableWidget.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousUITheme_DA;
class UGorgeousUIState_DA;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Activatable Widget
| Functional Name: UGorgeousActivatableWidget
| Parent Class: UCommonActivatableWidget
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base activatable widget for all Gorgeous menus and overlays. Automatically
| participates in the theme/processor system.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(Abstract, BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/UGorgeousActivatableWidget",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousActivatableWidget : public UCommonActivatableWidget, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	
	// Initializes the widget and sets up its Signal Bridge interface boilerplate.
	UGorgeousActivatableWidget();

	//<============================--- Overrides ---============================>
	#pragma region Overrides

	//<----------------------=== Interface Overrides ===------------------------>
	
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()
	//<----------------------=== End Interface Overrides ===-------------------->
	
protected:
	
	// Called when the widget is activated onto a layer stack.
	virtual void NativeOnActivated() override;
	
	// Called when the widget is deactivated from a layer stack.
	virtual void NativeOnDeactivated() override;
	
	// Called when the widget is constructed into the viewport.
	virtual void NativeConstruct() override;
	
	// Called when the widget is removed from the viewport.
	virtual void NativeDestruct() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Notifies the widget that a theme was applied so it can refresh its styled properties.
	 *
	 * @param Theme The theme data asset that was applied to this widget.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied"))
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme);
	
	/**
	 * Plays an animation on this widget by name.
	 *
	 * @param AnimName The name of the animation to play.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Animation")
	void PlayAnimationByName(const FName& AnimName);
	
	/**
	 * Called when the global UI state changes.
	 *
	 * @param NewState The newly activated UI state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI")
	void OnUIStateChanged(UGorgeousUIState_DA* NewState);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// The gameplay tag used to route this widget's binding through the Signal Bridge.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;
	
	// Enables per-widget filtering of which properties themes and Signal Bridge payloads may style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	bool bUseStylePropertyAllowList = true;
	
	// The set of property names permitted to be styled when the allow list is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	TSet<FName> StylePropertyAllowList;
	
	// When set, the UI system switches to this state when the widget is activated.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	TObjectPtr<class UGorgeousUIState_DA> AutoState;
	
	// When true, the system returns to the previous state when the widget is deactivated.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	bool bRevertStateOnDeactivation = false;
	
	// Map of UI States to animation names, played automatically when the system switches states.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	TMap<class UGorgeousUIState_DA*, FName> StateAnimations;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:

	// Tracks the UI state that was active before this widget took over.
	UPROPERTY()
	TObjectPtr<class UGorgeousUIState_DA> PreviousState;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};