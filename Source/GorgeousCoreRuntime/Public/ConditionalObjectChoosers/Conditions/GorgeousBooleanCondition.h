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
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCondition.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousBooleanCondition.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Boolean Condition
| Functional Name: UGorgeousBooleanCondition
| Parent Class: UGorgeousCondition
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A condition that evaluates boolean values based on the set mode. Uses
| Param A & Param B and applies the selected mode to evaluate if the chooser
| should return byte 0 for false or byte 1 for true.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(MinimalAPI, Blueprintable, BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/GorgeousBooleanCondition",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/Examples/"
		)
)
class UGorgeousBooleanCondition : public UGorgeousCondition
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
#pragma region Blueprint Functions
public:

	/**
	 * Evaluates the boolean condition based on the selected mode.
	 *
	 * @return 1 if the condition is true, 0 otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Boolean Condition")
	virtual uint8 CheckCondition_Implementation() override;
	//<------------------------------------------------------------------------->
#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
#pragma region UAT/UBT Exposed Variables

	// The first boolean input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Boolean Condition")
	bool A;

	// The second boolean input.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Boolean Condition")
	bool B;
	//<------------------------------------------------------------------------->
#pragma endregion UAT/UBT Exposed Variables
};