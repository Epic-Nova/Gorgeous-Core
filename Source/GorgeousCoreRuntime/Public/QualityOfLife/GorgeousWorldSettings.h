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
#include "GameFramework/WorldSettings.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousWorldSettings.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of AWorldSettings used to manage world-specific settings and configuration.
 * 
 * This class extends AWorldSettings to provide extra functionality for managing world-specific data and configurations. 
 * The `AGorgeousWorldSettings` class enables the management of additional world-related settings such as NPC spawn points, 
 * item spawns, and other important world data through the `AdditionalGorgeousData` map. Custom behaviors are also provided 
 * for `BeginPlay()` and `PostEditChangeProperty()` functions to handle world initialization and property changes.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:

	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the world settings begin play.
	 * 
	 * This function is called when the world settings are initialized. It is ideal for setting up world-specific data, 
	 * such as NPC spawn locations, item spawns, and other world-related properties.
	 */
	virtual void BeginPlay() override;

#if WITH_EDITOR
	/** 
	 * Handles property changes during the editor post-edit phase.
	 * 
	 * This function is called when properties are changed in the editor. It can be used to handle any special logic 
	 * when properties like `AdditionalGorgeousData` are modified during development.
	 * 
	 * @param PropertyChangedEvent The event triggered by the property change.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif WITH_EDITOR
	
	//<-------------------------------------------------------------------------->

	/** 
	 * Additional settings/configuration data for the current world.
	 * 
	 * This property holds additional world-related data such as spawn points, locations, and other configurations 
	 * for the game world. It is stored as a map, allowing dynamic access to key-value pairs.
	 * 
	 * @note The data is editable in the editor and can be modified at runtime to adjust world settings or configurations 
	 * on the fly.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Gorgeous World Settings")
	TMap<FName, UGorgeousObjectVariable*> AdditionalGorgeousData; 
};