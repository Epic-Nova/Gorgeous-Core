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
//<-------------------------------------------------------------------------->
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousGameInstance.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of UGameInstance used to handle game-specific initialization, settings, and configuration.
 * 
 * This class extends UGameInstance to provide additional functionality tailored for the GorgeousCore runtime. It includes the ability
 * to manage additional configuration data specific to the game instance through the `AdditionalGorgeousData` property.
 * 
 * The class provides overrides for the `Init()` and `PostEditChangeProperty()` functions, which allow custom initialization 
 * and handling of property changes, respectively.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	//<============================--- Overrides ---=============================>

	/** 
	 * Initializes the game instance.
	 * 
	 * This function is called when the game instance is initialized. It is intended to be overridden 
	 * to add custom initialization logic for the game instance, such as loading resources or setting up settings.
	 */
	virtual void Init() override;

#if WITH_EDITOR
	/** 
	 * Handles property changes for the game instance during the editor post-edit phase.
	 * 
	 * This function is triggered whenever a property of the game instance is changed in the editor. 
	 * The `PostEditChangeProperty` override ensures that changes to specific properties, such as `AdditionalGorgeousData`, 
	 * are handled, including updating the `UniqueIdentifier` of newly added data.
	 * 
	 * @param PropertyChangedEvent The event triggered by the property change.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif WITH_EDITOR
	
	//<-------------------------------------------------------------------------->
	
	/** 
	 * Additional settings/configuration data for the current game instance.
	 * 
	 * This property holds a map of additional settings and configuration data relevant to the current game instance.
	 * Examples of data stored here may include the cached save game, current game settings, or other game-specific data.
	 * Each entry in the map is associated with a unique name and is represented by an `UGorgeousObjectVariable`.
	 * 
	 * @note This data is editable within the editor and can be used to store settings that are dynamically accessed during runtime.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Gorgeous Game Instance")
	TMap<FName, UGorgeousObjectVariable*> AdditionalGorgeousData;
};