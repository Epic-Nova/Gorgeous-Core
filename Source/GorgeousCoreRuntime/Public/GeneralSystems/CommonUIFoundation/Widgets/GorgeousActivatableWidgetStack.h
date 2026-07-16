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
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousActivatableWidgetStack.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Activatable Widget Stack
| Functional Name: UGorgeousActivatableWidgetStack
| Parent Class: UCommonActivatableWidgetStack
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A CommonActivatableWidgetStack that auto-registers itself as a layout layer.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Gorgeous Activatable Widget Stack",
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/UGorgeousActivatableWidgetStack",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousActivatableWidgetStack : public UCommonActivatableWidgetStack
{
	GENERATED_BODY()

	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// The layer tag this stack registers as. Must be under the UI.Layer parent tag.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- Overrides ---============================>
	#pragma region Overrides
protected:

	// Re-initializes styling bindings when the underlying widget is rebuilt.
	virtual void OnWidgetRebuilt() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
};