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
#include "GameFramework/GameStateBase.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousGameState.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of AGameStateBase used to manage game state-specific logic and settings.
 * 
 * This class extends AGameStateBase to provide additional functionality tailored for the GorgeousCore runtime.
 * It is used to manage game state-specific data and logic, such as match data and other relevant information 
 * during gameplay. The class provides overrides for the `BeginPlay()` and `PostEditChangeProperty()` functions,
 * enabling custom behavior during the start of the game state and when properties are modified in the editor.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the game state begins play.
	 * 
	 * This function is called when the game state starts. It is intended to be overridden to implement custom
	 * logic for initializing the game state, such as setting up match conditions or managing state variables.
	 */
	virtual void BeginPlay() override;

#if WITH_EDITOR
	/** 
	 * Handles property changes for the game state during the editor post-edit phase.
	 * 
	 * This function is triggered whenever a property of the game state is changed in the editor. 
	 * The `PostEditChangeProperty` override ensures that changes to properties like `AdditionalGorgeousData` are handled appropriately.
	 * 
	 * @param PropertyChangedEvent The event triggered by the property change.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif WITH_EDITOR
	
	//<-------------------------------------------------------------------------->

	/** 
	 * Additional data for the current class.
	 * 
	 * This property holds a map of additional data specific to the game state. Examples of data include match data, 
	 * round information, or any other state-related variables that need to be dynamically accessed during gameplay.
	 * 
	 * @note This data is editable in the editor and can be used to store and manage game state settings or other dynamic information.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Gorgeous Game State")
	TMap<FName, UGorgeousObjectVariable*> AdditionalGorgeousData; 
};
