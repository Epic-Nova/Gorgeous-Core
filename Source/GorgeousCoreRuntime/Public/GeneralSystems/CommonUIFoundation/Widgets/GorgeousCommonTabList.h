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
#include "CommonTabListWidgetBase.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCommonTabList.generated.h"
//<-------------------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Common Tab List
| Functional Name: UGorgeousCommonTabList
| Parent Class: UCommonTabListWidgetBase
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| AAA Tab List with Gorgeous Foundation support. Automatically themes its tab
| buttons and supports signal-driven tab switching.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/UGorgeousCommonTabList",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousCommonTabList : public UCommonTabListWidgetBase, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

	// Initializes the widget and sets up its Signal Bridge interface boilerplate.
	UGorgeousCommonTabList(const FObjectInitializer& ObjectInitializer);

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
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
protected:

	/**
	 * Signal-driven tab selection handler.
	 * Payload should contain "TabIndex" (int) or "TabID" (FName).
	 *
	 * @param SignalTag The signal tag that triggered the selection.
	 * @param Payload The instanced payload carrying the tab selection data.
	 */
	UFUNCTION()
	void OnTabSelectSignalReceived(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};