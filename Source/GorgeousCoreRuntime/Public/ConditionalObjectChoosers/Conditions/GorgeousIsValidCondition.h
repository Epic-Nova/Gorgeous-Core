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
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousIsValidCondition.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A condition that checks if objects are valid (not null).
 * 
 * Uses Param A & Param B and applies the selected mode to evaluate if the chooser should return byte 0 for false or byte 1 for true.
 * 
 * @author Nils Bergemann
 * @note This condition can be used in conditional object choosers to select objects based on object validity.
 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType, 
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Overview", 
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/IsValidCondition", 
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Examples/"
		))
class UGorgeousIsValidCondition : public UGorgeousCondition
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
public:
	
	/**
	 * Checks if the object inputs are valid based on the selected mode.
	 *
	 * @return 1 if the condition is true, 0 otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous IsValid Condition")
	virtual uint8 CheckCondition_Implementation() override;
	//<------------------------------------------------------------------------->

	
	//<====================--- UAT/UBT Exposed Variables ---====================>
	
	// The first object input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous IsValid Condition")
	UObject* A;

	// The second object input. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous IsValid Condition")
	UObject* B;
	//<------------------------------------------------------------------------->
};