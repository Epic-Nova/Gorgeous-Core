// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "QualityOfLife/GorgeousGameInstance.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousCoreRuntimeGlobals.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Class extended by all other classes that are part of the Gorgeous Things ecosystem.
 *
 * Used to provide a static way to extend the functionality of the Gorgeous Things ecosystem.
 * Although the accessing of these functionalities is mostly done through wrapper classes that are widely accessible in the engine (blueprint).
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeous : public UObject
{
	GENERATED_BODY()
};

/**
 * Interface class extended by all other interface classes that are part of the Gorgeous Things ecosystem.
 *
 * Used to provide a static way to extend the functionality of the Gorgeous Things ecosystem for interfaces.
 *
 * @note This class is intended to be used as a base for interface implementations within the Gorgeous Things ecosystem.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInterface : public UGorgeous
{
	GENERATED_BODY()

protected:

	/**
	 * Protected constructor to prevent direct instantiation.
	 */
	explicit UGorgeousInterface() {}
};

/**
 * Globals class for the Gorgeous Core Runtime module.
 *
 * Exposes static functions and properties that are used throughout the module.
 */
UCLASS(BlueprintType, DisplayName = "Gorgeous Core Globals")
class UGorgeousCoreRuntimeGlobals : public UObject
{
	GENERATED_BODY()

public:
	/**
	 *	Returns the reference to the Gorgeous Game Instance,
	 *	Requires to set UGorgeousGameInstance or a child of it to be set in the Project Settings to work.
	 * 
	 * @return The current instance of the Gorgeous Game Instance.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousGameInstance* GetGorgeousGameInstance(const UObject* WorldContextObject);

	/**
	 *	Returns the reference to the Gorgeous Game Mode,
	 *	Requires to set AGorgeousGameMode or a child of it to be set in the Project Settings to work.
	 * 
	 * @return The current instance of the Gorgeous Game Mode.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals", meta = (WorldContext = "WorldContextObject"))
	static AGorgeousGameMode* GetGorgeousGameMode(const UObject* WorldContextObject);

	/**
	 *	Returns the reference to the Gorgeous Game State,
	 *	Requires to set AGorgeousGameState or a child of it to be set in the Game Mode to work.
	 * 
	 * @return The current instance of the Gorgeous Game State.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals", meta = (WorldContext = "WorldContextObject"))
	static AGorgeousGameState* GetGorgeousGameState(const UObject* WorldContextObject);

	/**
	 *	Returns the reference to the Gorgeous Player Controller,
	 *	Requires to set AGorgeousPlayerController or a child of it to be set in the Game Mode to work.
	 * 
	 * @return The current instance of the Gorgeous Player Controller.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals", meta = (WorldContext = "WorldContextObject"))
	static AGorgeousPlayerController* GetGorgeousPlayerController(const UObject* WorldContextObject, const int32 PlayerIndex);

	/**
	 *	Returns the reference to the Gorgeous Player State,
	 *	Requires to set AGorgeousPlayerController or a child of it to be set in the Game Mode to work.
	 * 
	 * @return The current instance of the Gorgeous Player State.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals", meta = (WorldContext = "WorldContextObject"))
	static AGorgeousPlayerState* GetGorgeousPlayerState(const UObject* WorldContextObject, const int32 PlayerStateIndex);

	/**
	 *	Returns the reference to the Gorgeous World Settings,
	 *	Requires to set AGorgeousWorldSettings or a child of it to be set in the Project Settings to work.
	 * 
	 * @return The current instance of the Gorgeous World Settings.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals", meta = (WorldContext = "WorldContextObject"))
	static AGorgeousWorldSettings* GetGorgeousWorldSettings(const UObject* WorldContextObject);
};

