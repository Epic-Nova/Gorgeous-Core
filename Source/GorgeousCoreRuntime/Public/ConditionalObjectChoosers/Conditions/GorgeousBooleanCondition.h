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
#include "GorgeousCondition.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousBooleanCondition.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A condition that evaluates boolean values based on a specified mode.
 *
 * Key features include:
 * - Boolean inputs A and B.
 * - Mode selection from EConditionalChooserMode_E.
 * - CheckCondition function to evaluate the condition.
 *
 * @note This condition can be used in conditional object choosers to select objects based on boolean logic.
 */
UCLASS(Blueprintable, BlueprintType)
class UGorgeousBooleanCondition : public UGorgeousCondition
{
	GENERATED_BODY()

public:

	/**
	 * The first boolean input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Boolean Condition")
	bool A;

	/**
	 * The second boolean input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Boolean Condition")
	bool B;

	/**
	 * Evaluates the boolean condition based on the selected mode.
	 *
	 * @return 1 if the condition is true, 0 otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Boolean Condition")
	virtual uint8 CheckCondition() override;
};