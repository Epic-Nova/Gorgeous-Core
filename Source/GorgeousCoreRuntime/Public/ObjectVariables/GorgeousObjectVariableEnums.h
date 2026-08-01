// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousObjectVariableEnums.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Enum that defines the variable count of the object variable.
 * @author Nils Bergemann
 */
UENUM(BlueprintType, DisplayName = "Object Variable Count", meta = (ShortTooltip = "The count of the object variable."))
enum class EObjectVariableContainerType_E : uint8
{
	ESingle = 0 UMETA(DisplayName = "Single", ToolTip = "A single object variable."),
	EArray = 1 UMETA(DisplayName = "Array", ToolTip = "An array of object variables."),
	EMap = 2 UMETA(DisplayName = "Map", ToolTip = "A map of object variables."),
	ESet = 3 UMETA(DisplayName = "Set", ToolTip = "A set of object variables."),
	EQueue = 4 UMETA(DisplayName = "Queue", ToolTip = "A queue of object variables."),
	EStack = 5 UMETA(DisplayName = "Stack", ToolTip = "A stack of object variables."),
	EDeque = 6 UMETA(DisplayName = "Deque", ToolTip = "A deque of object variables."),
	EMultiMap = 7 UMETA(DisplayName = "Multi Map", ToolTip = "A multi map of object variables. That means that the map can have multiple values for a single key."),
	EObjectVariableCount_MAX UMETA(Hidden)
};

/**
 * Enum that defines the queue mode for queue object variables.
 * @author Nils Bergemann
 */
UENUM(BlueprintType, DisplayName = "Object Variable Queue Mode", meta = (ShortTooltip = "The queue mode for queue object variables."))
enum class EObjectVariableQueueMode_E : uint8
{
	ESpsc = 0 UMETA(DisplayName = "SPSC", ToolTip = "Single-producer, single-consumer queue."),
	EMpsc = 1 UMETA(DisplayName = "MPSC", ToolTip = "Multiple-producers, single-consumer queue."),
	EObjectVariableQueueMode_MAX UMETA(Hidden)
};