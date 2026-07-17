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
#include "Conditions/GorgeousCondition.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousConditionalObjectChooser.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Conditional Object Chooser
| Functional Name: UGorgeousConditionalObjectChooser
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A conditional chooser that selects an object variable based on a
| condition.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable, BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Choosers",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousConditionalObjectChooser : public UObject
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
#pragma region Overrides

	// Sanitizes serialized data after load to drop placeholders created by circular blueprint dependencies.
	virtual void PostLoad() override;

	// Sanitizes during load so placeholders are nulled before dependency repair runs.
	virtual void Serialize(FArchive& Ar) override;
	//<------------------------------------------------------------------------->
#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
#pragma region Blueprint Functions
public:

	/**
	 * Selects an object variable based on the condition check.
	 *
	 * @return The selected Object Variable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Conditional Object Chooser")
	UGorgeousObjectVariable* DecideCondition() const;

	/**
	 * Removes invalid or placeholder references from the condition check and conditions array.
	 * Safe to call from editor utility or construction scripts to repair corrupted assets.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Gorgeous Conditional Object Chooser")
	void CleanupInvalidEntries();
//<------------------------------------------------------------------------->
#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
#pragma region UAT/UBT Exposed Variables

	// The condition check that should be performed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Conditional Object Chooser", Setter, meta = (AllowAbstract = "true"))
	TObjectPtr<UGorgeousCondition> ConditionCheck = nullptr;

	// The object variables that can be selected based on the condition check.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Conditional Object Chooser", meta = (AllowAbstract = "true"))
	TArray<TObjectPtr<UGorgeousObjectVariable>> Conditions;
	//<------------------------------------------------------------------------->
#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
#pragma region C++ Only
private:

	/**
	 * Sets the condition check variable and ensures that the assigned object is valid and of the correct type.
	 *
	 * @param NewConditionCheck The new condition check to set.
	 */
	UFUNCTION(BlueprintCallable)
	void SetConditionCheck(UGorgeousCondition* NewConditionCheck);
	//<------------------------------------------------------------------------->
#pragma endregion C++ Only
};
