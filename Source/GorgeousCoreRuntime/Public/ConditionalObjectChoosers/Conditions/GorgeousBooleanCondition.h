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
#include "GorgeousCondition.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousBooleanCondition.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A condition that evaluates boolean values based on the set mode.
 *
 * Uses Param A & Param B and applies the selected mode to evaluate if the chooser should return byte 0 for false or byte 1 for true.
 * 
 * @author Nils Bergemann
 * @note This condition can be used in conditional object choosers to select objects based on boolean logic.
 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Overview", 
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/BooleanCondition", 
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Examples/"
		))
class UGorgeousBooleanCondition : public UGorgeousCondition
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
public:
	
	/**
	 * Evaluates the boolean condition based on the selected mode.
	 *
	 * @return 1 if the condition is true, 0 otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Boolean Condition")
	virtual uint8 CheckCondition_Implementation() override;
	//<------------------------------------------------------------------------->

	
	//<====================--- UAT/UBT Exposed Variables ---====================>
	
	// The first boolean input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Boolean Condition")
	bool A;

	// The second boolean input.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Boolean Condition")
	bool B;
	//<------------------------------------------------------------------------->
};