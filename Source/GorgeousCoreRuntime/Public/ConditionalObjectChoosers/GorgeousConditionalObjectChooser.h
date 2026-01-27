// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Conditions/GorgeousCondition.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
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
 * @author Nils Bergemann
 * @note This class allows for dynamic object selection based on specified conditions.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousConditionalObjectChooser : public UObject
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
	 * Removes invalid or placeholder references from the condition check and conditions array.
	 * Safe to call from editor utility or construction scripts to repair corrupted assets.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Gorgeous Conditional Object Chooser")
	void CleanupInvalidEntries();

	/**
	 * The condition check object.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Conditional Object Chooser", Setter = SetConditionCheck, meta = (AllowAbstract = "true"))
	TObjectPtr<UGorgeousCondition> ConditionCheck = nullptr;

	/**
	 * The array of object variables to choose from.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Conditional Object Chooser", meta = (AllowAbstract = "true"))
	TArray<TObjectPtr<UGorgeousObjectVariable>> Conditions;

private:

	// Sanitizes serialized data after load to drop placeholders created by circular blueprint dependencies.
	virtual void PostLoad() override;

	// Sanitizes during load so placeholders are nulled before dependency repair runs.
	virtual void Serialize(FArchive& Ar) override;

	/**
	 * Sets the condition check variable and ensures that the variable is not this object.
	 * @param NewConditionCheck The new condition check to set.
	 */
	void SetConditionCheck(UGorgeousCondition* NewConditionCheck);
	
};