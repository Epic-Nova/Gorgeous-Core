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
#include "GameFramework/CheatManager.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousStatFoundationCheatManagerExtension.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Stat Foundation Cheat Manager Extension
| Functional Name: UGorgeousStatFoundationCheatManagerExtension
| Parent Class: UCheatManagerExtension
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Cheat Manager Extension for the Gorgeous Stat System. Provides console
| commands to manipulate stats during development.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/GorgeousStatFoundationCheatManagerExtension",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousStatFoundationCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Sets a stat value for the player's current character.
	 *
	 * @param StatTag The statistic tag to set.
	 * @param Value The value to assign.
	 */
	UFUNCTION(Exec)
	void Cheat_SetStat(FString StatTag, float Value);

	/**
	 * Modifies a stat value for the player's current character by a delta.
	 *
	 * @param StatTag The statistic tag to modify.
	 * @param Delta The amount to add to the statistic.
	 */
	UFUNCTION(Exec)
	void Cheat_ModifyStat(FString StatTag, float Delta);

	// Lists all current statistics and values for the player's character.
	UFUNCTION(Exec)
	void Cheat_ListStats();

	/**
	 * Adds a controller class to the whitelist for a specific stat.
	 *
	 * @param StatTag The statistic tag whose whitelist is updated.
	 * @param ControllerClassName The controller class name to allow.
	 */
	UFUNCTION(Exec)
	void Cheat_AddAllowedController(FString StatTag, FString ControllerClassName);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};