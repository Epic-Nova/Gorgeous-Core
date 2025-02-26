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
 * Enum that defines the variable type of the object variable.
 * 
 * @brief Enum that defines the variable type of the object variable.
 * @author Nils Bergemann
 * @since Beginning
 * @deprecated Will be removed and replaced by the details customisation that that automatically geathers registered object variables
 */
UENUM(BlueprintType, DisplayName = "Object Variable Type", meta = (ShortTooltip = "The type of the object variable."))
enum class EObjectVariableTypes_E : uint8
{
	EObjectVariable = 0 UMETA(DisplayName = "Object Variable", ToolTip = "An Object Variable that can be used to store any kind of data."),
	EObject = 1 UMETA(DisplayName = "Object", ToolTip = "An reference to an object."),
	EClass = 2 UMETA(DisplayName = "Class", ToolTip = "An reference to a class."),
	ESoftObject = 3 UMETA(DisplayName = "Soft Object", ToolTip = "An reference to an object that is soft."),
	ESoftClass = 4 UMETA(DisplayName = "Soft Class", ToolTip = "An reference to a class that is soft."),
	EBoolean = 5 UMETA(DisplayName = "Boolean", ToolTip = "A boolean value."),
	EByte = 6 UMETA(DisplayName = "Byte", ToolTip = "A byte value."),
	EFloat = 7 UMETA(DisplayName = "Float", ToolTip = "A float value."),
	EInteger64 = 8 UMETA(DisplayName = "Integer64", ToolTip = "An integer 64 value."),
	EInteger = 9 UMETA(DisplayName = "Integer", ToolTip = "An integer value."),
	EName = 10 UMETA(DisplayName = "Name", ToolTip = "A name value."),
	ERotator = 11 UMETA(DisplayName = "Rotator", ToolTip = "A rotator value."),
	EString = 12 UMETA(DisplayName = "String", ToolTip = "A string value."),
	EText = 13 UMETA(DisplayName = "Text", ToolTip = "A text value."),
	ETransform = 14 UMETA(DisplayName = "Transform", ToolTip = "A transform value."),
	EVector = 15 UMETA(DisplayName = "Vector", ToolTip = "A vector value."),
	EObjectVariableTypes_Max UMETA(Hidden)
};

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