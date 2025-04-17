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
#include "GameFramework/GameModeBase.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousGameMode.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of AGameModeBase used to manage game mode-specific logic and settings.
 * 
 * This class extends AGameModeBase to provide additional functionality tailored for the GorgeousCore runtime.
 * It is used to manage game mode-specific data and logic, such as player data and other relevant information.
 * The class provides overrides for the `BeginPlay()` and `PostEditChangeProperty()` functions, allowing custom behavior 
 * during the start of the game and when properties are changed in the editor, respectively.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the game mode begins play.
	 * 
	 * This function is invoked when the game mode starts. It is intended to be overridden to implement custom
	 * logic for game mode initialization, such as managing players or configuring game settings.
	 */
	virtual void BeginPlay() override;

#if WITH_EDITOR
	/** 
	 * Handles property changes for the game mode during the editor post-edit phase.
	 * 
	 * This function is triggered whenever a property of the game mode is changed in the editor. 
	 * The `PostEditChangeProperty` override ensures that changes to properties like `AdditionalGorgeousData` are handled properly.
	 * 
	 * @param PropertyChangedEvent The event triggered by the property change.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif WITH_EDITOR
	
	//<-------------------------------------------------------------------------->

	/** 
	 * Additional data for the current class.
	 * 
	 * This property holds a map of additional data specific to the game mode. This could include player data, 
	 * game settings, or other game-specific information that needs to be dynamically accessed during gameplay.
	 * 
	 * @note This data is editable in the editor and can be used to store and manage settings or variables 
	 * that are relevant to the game's logic.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Gorgeous Game Mode")
	TMap<FName, UGorgeousObjectVariable*> AdditionalGorgeousData; 
};