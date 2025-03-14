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
#include "UObject/Interface.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousObjectVariableInteraction_I.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Interface for interacting with object variables.
 *
 * Key features include:
 * - Function to get the unique identifier for an object variable.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to interact with object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousObjectVariableInteraction_I : public UInterface
{
	GENERATED_BODY()
};


/**
 * Native interface for interacting with object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousObjectVariableInteraction_I
{
	GENERATED_BODY()

public:

	/**
	 * Gets the unique identifier for an object variable.
	 *
	 * @return The unique identifier as an FGuid.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables")
	FGuid GetUniqueIdentifierForObjectVariable();
};
