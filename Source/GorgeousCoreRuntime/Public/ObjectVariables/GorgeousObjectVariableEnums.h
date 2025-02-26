// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousObjectVariableEnums.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Enum that defines the variable count of the object variable.
 *
 * @brief Enum that defines the variable count of the object variable.
 * @author Nils Bergemann
 * @since Beginning
 */
UENUM(BlueprintType, DisplayName = "Object Variable Count", meta = (ShortTooltip = "The count of the object variable."))
enum class EObjectVariableCount_E : uint8
{
	ESingle = 0 UMETA(DisplayName = "Single", ToolTip = "A single object variable."),
	EArray = 1 UMETA(DisplayName = "Array", ToolTip = "An array of object variables."),
	EMap = 2 UMETA(DisplayName = "Map", ToolTip = "A map of object variables."),
	ESet = 3 UMETA(DisplayName = "Set", ToolTip = "A set of object variables."),
	EQueue = 4 UMETA(DisplayName = "Queue", ToolTip = "A queue of object variables."),
	EStack = 5 UMETA(DisplayName = "Stack", ToolTip = "A stack of object variables."),
	EDeque = 6 UMETA(DisplayName = "Deque", ToolTip = "A deque of object variables."),
	EMultiMap = 7 UMETA(DisplayName = "Multi Map", ToolTip = "A multi map of object variables. That means that the map can have multiple values for a single key."),
	EObjectVariableCount_Max UMETA(Hidden)
};