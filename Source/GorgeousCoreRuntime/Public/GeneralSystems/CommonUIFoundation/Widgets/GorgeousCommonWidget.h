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
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCommonWidget.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousUIProcessor;
class UGorgeousUIState_DA;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Common Widget
| Functional Name: UGorgeousCommonWidget
| Parent Class: UCommonUserWidget
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base widget for the Gorgeous UI system. Inherits from CommonUserWidget for
| Common UI compatibility.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(Abstract, BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/UGorgeousCommonWidget",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousCommonWidget : public UCommonUserWidget, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	
	// Initializes the widget and sets up its Signal Bridge interface boilerplate.
	UGorgeousCommonWidget(const FObjectInitializer& ObjectInitializer);

	//<============================--- Overrides ---============================>
	#pragma region Overrides

	//<----------------------=== Interface Overrides ===------------------------>
	
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()
	//<----------------------=== End Interface Overrides ===-------------------->
	
	
	// Returns the routing ID used by the Signal Bridge.
	virtual FName GetRoutingID() const override { return RoutingID; }
	
	// Sets the routing ID used by the Signal Bridge.
	virtual void SetRoutingID(FName InID) override { RoutingID = InID; }
	
protected:
	
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
	 * Updates the widget's icon based on the current theme and input method.
	 *
	 * @param ThemeOverride Optional theme override; uses the active theme when null.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void UpdateActionIcon(const UGorgeousUITheme_DA* ThemeOverride = nullptr);
	
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
	
	// Routing ID for this widget instance (e.g. Slot Index).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FName RoutingID;
	
	// Enables per-widget filtering of which properties themes and Signal Bridge payloads may style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	bool bUseStylePropertyAllowList = true;
	
	// The set of property names permitted to be styled when the allow list is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	TSet<FName> StylePropertyAllowList;
	
	// Optional Action Tag for this widget (e.g. UI.Action.Confirm), used for dynamic icon swapping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag ActionTag;
	
	// Map of UI States to animation names, played automatically when the system switches states.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	TMap<UGorgeousUIState_DA*, FName> StateAnimations;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};