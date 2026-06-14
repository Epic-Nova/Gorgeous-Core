// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GorgeousStatFoundationCheatManagerExtension.generated.h"

/**
 * Cheat Manager Extension for the Gorgeous Stat System.
 * Provides console commands to manipulate stats during development.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousStatFoundationCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	/** 
	 * Sets a stat value for the player's current character.
	 * Usage: Stat.Set [StatTag] [Value]
	 */
	UFUNCTION(Exec)
	void Cheat_SetStat(FString StatTag, float Value);

	/** 
	 * Modifies a stat value for the player's current character by a delta.
	 * Usage: Stat.Modify [StatTag] [Delta]
	 */
	UFUNCTION(Exec)
	void Cheat_ModifyStat(FString StatTag, float Delta);

	/** 
	 * Lists all current stats and their values for the player's character.
	 * Usage: Stat.List
	 */
	UFUNCTION(Exec)
	void Cheat_ListStats();

	/** 
	 * Adds a controller class to the whitelist for a specific stat.
	 * Usage: Stat.Allow [StatTag] [ControllerClassName]
	 */
	UFUNCTION(Exec)
	void Cheat_AddAllowedController(FString StatTag, FString ControllerClassName);
};
