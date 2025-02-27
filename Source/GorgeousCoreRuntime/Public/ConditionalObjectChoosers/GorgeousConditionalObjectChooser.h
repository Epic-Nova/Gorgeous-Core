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
//<-------------------------=== Module Includes ===-------------------------->
#include "Conditions/GorgeousCondition.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousConditionalObjectChooser.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A conditional object chooser that selects an object variable based on a condition.
 *
 * Key features include:
 * - ConditionCheck: A GorgeousCondition object to determine the selection.
 * - Conditions: An array of GorgeousObjectVariables to choose from.
 * - DecideCondition function to select the appropriate object variable.
 *
 * @note This class allows for dynamic object selection based on specified conditions.
 */
UCLASS(Blueprintable, BlueprintType)
class UGorgeousConditionalObjectChooser : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Selects an object variable based on the condition check.
	 *
	 * @return The selected GorgeousObjectVariable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Conditional Object Chooser")
	UGorgeousObjectVariable* DecideCondition() const;

	/**
	 * The condition check object.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Conditional Object Chooser")
	UGorgeousCondition* ConditionCheck;

	/**
	 * The array of object variables to choose from.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta = (ExposeOnSpawn), Category = "Conditional Object Chooser")
	TArray<UGorgeousObjectVariable*> Conditions;
};