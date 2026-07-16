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
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "CommonButtonBase.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCommonButton.generated.h"
//<-------------------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Common Button
| Functional Name: UGorgeousCommonButton
| Parent Class: UCommonButtonBase
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A Gorgeous button that automatically skins itself and plays themed audio.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(Abstract, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/UGorgeousCommonButton",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousCommonButton : public UCommonButtonBase, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

	// Initializes the widget and sets up its Signal Bridge interface boilerplate.
	UGorgeousCommonButton(const FObjectInitializer& ObjectInitializer);

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	//<----------------------=== Interface Overrides ===------------------------>
	
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()
	//<----------------------=== End Interface Overrides ===-------------------->
	
protected:
	
	// Called when the widget is constructed into the viewport.
	virtual void NativeConstruct() override;
	
	// Called when the widget is removed from the viewport.
	virtual void NativeDestruct() override;
	
	// Called when the pointer enters the widget bounds.
	virtual void NativeOnHovered() override;
	
	// Called when the pointer leaves the widget bounds.
	virtual void NativeOnUnhovered() override;
	
	// Called when the widget is clicked.
	virtual void NativeOnClicked() override;
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
	
	// Sound tag played on the hover event.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI|Audio")
	FGameplayTag HoverSoundTag;
	
	// Sound tag played on the click event.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI|Audio")
	FGameplayTag ClickSoundTag;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
protected:

	// Plays a themed sound by the supplied tag.
	void PlayThemedSound(FGameplayTag SoundTag);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};