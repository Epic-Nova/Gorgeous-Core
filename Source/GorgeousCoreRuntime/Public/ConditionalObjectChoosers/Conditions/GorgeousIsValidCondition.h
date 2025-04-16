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
#include "GorgeousIsValidCondition.generated.h"
//<-------------------------------------------------------------------------->
/**
 * A condition that checks if objects are valid (not null).
 *
 * Key features include:
 * - Object inputs A and B.
 * - Mode selection from EConditionalChooserMode_E (inherited from UGorgeousCondition).
 * - CheckCondition function to evaluate the condition.
 *
 * @author Nils Bergemann
 * @note This condition can be used in conditional object choosers to select objects based on object validity.
 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType)
class UGorgeousIsValidCondition : public UGorgeousCondition
{
	GENERATED_BODY()

public:

	/**
	 * The first object input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous IsValid Condition")
	UObject* A;

	/**
	 * The second object input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous IsValid Condition")
	UObject* B;

	/**
	 * Checks if the object inputs are valid based on the selected mode.
	 *
	 * @return 1 if the condition is true, 0 otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous IsValid Condition")
	virtual uint8 CheckCondition() override;
};