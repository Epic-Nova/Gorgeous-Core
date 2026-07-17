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
#include "Components/RichTextBlock.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCommonRichTextBlock.generated.h"
//<-------------------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Common Rich Text Block
| Functional Name: UGorgeousCommonRichTextBlock
| Parent Class: URichTextBlock
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Overridden RichTextBlock with Signal Bridge support.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/UGorgeousCommonRichTextBlock",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousCommonRichTextBlock : public URichTextBlock, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

	// Initializes the widget and sets up its Signal Bridge interface boilerplate.
	UGorgeousCommonRichTextBlock(const FObjectInitializer& ObjectInitializer);

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	//<----------------------=== Interface Overrides ===------------------------>
	
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()
	//<----------------------=== End Interface Overrides ===-------------------->
	
	// Reapplies styled properties after editor or runtime property changes.
	virtual void SynchronizeProperties() override;
	
	// Frees any cached slate resources held by the widget.
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	
protected:
	
	// Re-initializes styling bindings when the underlying widget is rebuilt.
	virtual void OnWidgetRebuilt() override;
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
};