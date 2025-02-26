// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousObjectVariableCmdletHandler.generated.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
//<-------------------------------------------------------------------------->

/**
 * Handles console commands related to Gorgeous Object Variables.
 *
 * Key features include:
 * - Registration of console commands.
 * - Listing of Gorgeous Variables.
 *
 * @note This class provides a way to interact with Gorgeous Object Variables through the console.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariableCmdletHandler : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Registers console commands related to Gorgeous Object Variables.
	 */
	static void RegisterConsoleCommands();

	/**
	 * Lists Gorgeous Variables based on the provided arguments.
	 *
	 * @param Args The arguments provided to the console command.
	 */
	static void ListGorgeousVariables(const TArray<FString>& Args);
};