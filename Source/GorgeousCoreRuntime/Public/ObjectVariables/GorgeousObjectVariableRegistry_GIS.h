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
//<-------------------------=== Engine Includes ===-------------------------->
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousRootObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousObjectVariableRegistry_GIS.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class ULevel;
class UWorld;
//<-------------------------------------------------->

/**
 * Game Instance Subsystem for managing the Gorgeous Object Variable Registry.
 *
 * Key features include:
 * - Initialization and deinitialization of the registry.
 * - Handling level removal events to clean up persistent variables.
 *
 * @note This subsystem provides a centralized place for managing object variables across the game instance.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariableRegistry_GIS : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Initializes the subsystem.
	 *
	 * @param Collection The collection of subsystems.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Deinitializes the subsystem.
	 */
	virtual void Deinitialize() override;

private:

	/**
	 * Handles the event when a level is removed.
	 *
	 * @param Level The level that was removed.
	 * @param World The world the level belonged to.
	 */
	void OnLevelRemoved(ULevel* Level, UWorld* World);
};