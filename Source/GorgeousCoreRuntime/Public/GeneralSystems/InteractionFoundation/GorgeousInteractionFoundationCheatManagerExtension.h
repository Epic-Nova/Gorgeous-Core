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
//<--------------------------=== Engine Includes ===------------------------->
#include "GameFramework/CheatManager.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInteractionFoundationCheatManagerExtension.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Interaction Foundation Cheat Manager Extension
| Functional Name: UGorgeousInteractionFoundationCheatManagerExtension
| Parent Class: UCheatManagerExtension
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides developer console commands for inspecting, clearing and testing
| local Interaction Foundation focus and interaction behavior.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/InteractionFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/InteractionFoundation/GorgeousInteractionFoundationCheatManagerExtension",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/InteractionFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousInteractionFoundationCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	// Reports the local controller's focus-tracking and interaction counters.
	UFUNCTION(Exec)
	void Cheat_InspectInteractionFocus();

	// Clears the local controller's current interaction focus when one is tracked.
	UFUNCTION(Exec)
	void Cheat_ClearInteractionFocus();

	/**
	 * Traces from the local player's view and requests focus on a matching target.
	 *
	 * @param InteractionTag The interaction tag that the traced target must support.
	 * @param TraceDistance The maximum view-space distance to trace.
	 */
	UFUNCTION(Exec)
	void Cheat_TraceInteractionFocus(FString InteractionTag, float TraceDistance = 1000.0f);

	/**
	 * Traces from the local player's view and executes a matching interaction.
	 *
	 * @param InteractionTag The interaction tag that the traced target must support.
	 * @param TraceDistance The maximum view-space distance to trace.
	 */
	UFUNCTION(Exec)
	void Cheat_TraceInteraction(FString InteractionTag, float TraceDistance = 1000.0f);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};