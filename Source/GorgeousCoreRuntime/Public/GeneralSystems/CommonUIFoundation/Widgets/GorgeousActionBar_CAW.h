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
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousActionBar_CAW.generated.h"
//<-------------------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Action Bar CAW
| Functional Name: UGorgeousActionBar_CAW
| Parent Class: UGorgeousActivatableWidget
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base widget for the HUD Action Bar stripe. Displays available input
| actions, their icons, and names.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(Abstract, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/UGorgeousActionBar_CAW",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Widgets/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousActionBar_CAW : public UGorgeousActivatableWidget
{
	GENERATED_BODY()

	// Initializes the widget and sets up its Signal Bridge interface boilerplate.
	UGorgeousActionBar_CAW(const FObjectInitializer& ObjectInitializer);

	//<============================--- Overrides ---============================>
	#pragma region Overrides
protected:

	// Resolves the desired UI input configuration for this action bar.
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Updates the action bar with new entries.
	 * Blueprint should implement this to rebuild the UI (e.g. clear box and add children).
	 *
	 * @param Entries The action bar entries used to rebuild the UI.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous|UI")
	void UpdateActionBar(const TArray<FGorgeousActionBarEntry_S>& Entries);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};