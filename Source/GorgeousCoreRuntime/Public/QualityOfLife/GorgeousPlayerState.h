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
#include "GameFramework/PlayerState.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousPlayerState.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of APlayerState used to manage player state and additional data.
 * 
 * This class extends APlayerState to provide extra functionality for managing player state and custom player metadata. 
 * The `AGorgeousPlayerState` class allows for the management of additional data through the `AdditionalGorgeousData` map, 
 * which can hold metadata or other relevant information about the player. The class also provides custom behavior for 
 * `BeginPlay()` and `PostEditChangeProperty()` functions, enabling initialization and modification of the player state 
 * in various scenarios, such as at the start of the game or when properties are edited in the Unreal Editor.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	
	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the player state begins play.
	 * 
	 * This function is called when the player state is initialized. It is ideal for setting up the player's state, 
	 * managing metadata, or initializing any relevant data related to the player.
	 */
	virtual void BeginPlay() override;

	/** 
	 * Handles property changes during the editor post-edit phase.
	 * 
	 * This function is called when properties are changed in the editor. It can be used to handle any special logic 
	 * when properties like `AdditionalGorgeousData` are modified during development.
	 * 
	 * @param PropertyChangedEvent The event triggered by the property change.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	//<-------------------------------------------------------------------------->

	/** 
	 * Additional data for the current class.
	 * 
	 * This property holds additional data relevant to the player state, such as player metadata, stats, or custom information.
	 * It is stored as a map, allowing for dynamic access to key-value pairs.
	 * 
	 * @note The data is editable in the editor and can be modified at runtime as needed, providing flexibility for handling 
	 * player-specific attributes or settings.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Gorgeous Player State")
	TMap<FName, UGorgeousObjectVariable*> AdditionalGorgeousData; 
};