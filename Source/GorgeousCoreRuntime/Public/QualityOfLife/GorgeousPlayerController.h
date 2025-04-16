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
#include "GameFramework/PlayerController.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousPlayerController.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of APlayerController used to manage player input and data.
 * 
 * This class extends APlayerController to provide additional functionality for managing player-related data 
 * and input. The `AGorgeousPlayerController` class allows the handling of extra data, such as input configurations 
 * or custom player attributes, through the `AdditionalGorgeousData` map. It also provides custom behavior for the 
 * `BeginPlay()` and `PostEditChangeProperty()` functions, which are used to initialize and modify the player controller 
 * in different scenarios, such as gameplay start or when properties are edited in the Unreal Editor.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the player controller begins play.
	 * 
	 * This function is called when the player controller starts. It can be overridden to initialize any player-specific
	 * functionality or data, such as setting up player input or other gameplay-related elements.
	 */
	virtual void BeginPlay() override;
	
	/** 
	 * Handles property changes during the editor post-edit phase.
	 * 
	 * This function is called when properties are changed in the editor. It can be used to handle specific logic when
	 * properties like `AdditionalGorgeousData` are modified during development.
	 * 
	 * @param PropertyChangedEvent The event that triggered the property change.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	//<-------------------------------------------------------------------------->

	/** 
	 * Additional data for the current class.
	 * 
	 * This property holds additional data for the player controller, such as input configurations or player attributes.
	 * It allows for dynamic management of player-specific settings during gameplay and can be accessed or modified as needed.
	 * 
	 * @note This data is editable in the editor, allowing for customization of the player controller’s attributes.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Gorgeous Player Controller")
	TMap<FName, UGorgeousObjectVariable*> AdditionalGorgeousData; 
};