// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GorgeousRecompilationHelper.generated.h"

/**
 * Helper class to manage system-wide recompilation needs.
 * Detects changes in installed Gorgeous systems and triggers the Gorgeous Installer if needed.
 */
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousRecompilationHelper : public UObject
{
	GENERATED_BODY()

public:
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
	/** Internal helper to get currently active Gorgeous systems based on preprocessor definitions. */
	static TArray<FString> GetActiveGorgeousSystems();
};
