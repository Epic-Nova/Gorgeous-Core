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
#include "UObject/NoExportTypes.h"
#include "GorgeousSingleton.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousRecompilationHelper.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Helper class to manage system-wide recompilation needs.
 * Detects changes in installed Gorgeous systems and triggers the Gorgeous Installer if needed.
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousRecompilationHelper : public UObject
{
	GENERATED_BODY()
	GORGEOUS_ATTACH_SINGLETON(UGorgeousRecompilationHelper);

public:
public:
	/** Broadcasts when the recompilation requirement state changes */
	static FSimpleMulticastDelegate OnRecompilationRequirementChanged;

	/**
	 * Returns true if a recompilation has been determined as required.
	 */
	static bool IsRecompilationRequired();

	/**
	 * Checks if a recompilation is needed by comparing currently installed systems
	 * with recorded systems from the previous session.
	 *
	 * @return true if a recompilation is required.
	 */
	static bool CheckRecompilationRequirement();

	/**
	 * Triggers the Gorgeous Installer to perform a full system re-install and recompilation.
	 * This will close the engine, run the installer, and optionally reopen the engine.
	 */
	static void TriggerGorgeousRecompilation();

private:
	/** Cached state of whether recompilation is needed */
	static bool bIsRecompilationRequired;

	/** Internal helper to get currently active Gorgeous systems based on preprocessor definitions. */
	static TArray<FString> GetActiveGorgeousSystems();
};