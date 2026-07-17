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
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooserTypes.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCondition.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Condition
| Functional Name: UGorgeousCondition
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base class for conditions used in conditional object choosers.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable, BlueprintType, EditInlineNew,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/GorgeousCondition",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/Examples/"
		)
)
class UGorgeousCondition : public UObject
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
#pragma region Blueprint Functions
public:

	/**
	 * Checks the blueprint overwritten condition and returns a result.
	 *
	 * @return The result of the condition check.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Condition")
	uint8 CheckCondition();
	//<------------------------------------------------------------------------->
#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
#pragma region UAT/UBT Exposed Variables

	/**
	 * The mode of the condition.
	 * Only used for algebraic conditions, for gameplay tag conditions the mode is determined by the fight mode.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Condition")
	EConditionalChooserMode_E Mode;
	//<------------------------------------------------------------------------->
#pragma endregion UAT/UBT Exposed Variables
};