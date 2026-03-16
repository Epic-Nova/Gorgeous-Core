// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooserEnums.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousCondition.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Base class for conditions used in conditional object choosers.
 *
 * Key features include:
 * - Mode selection from EConditionalChooserMode_E.
 * - CheckCondition function to evaluate the condition.
 *
 * @author Nils Bergemann
 * @note This class serves as a base for specific condition implementations.
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class UGorgeousCondition : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * The mode of the condition.
	 * Only used for algebraic conditions, for gameplay tag conditions the mode is determined by the fight mode.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Condition")
	EConditionalChooserMode_E Mode;

	/**
	 * Checks the blueprint overwritten condition and returns a result.
	 *
	 * @return The result of the condition check.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Condition")
	uint8 CheckCondition();
};