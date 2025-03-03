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
#include "GameplayTagContainer.h"
#include "GorgeousGameplayTagCondition.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A condition that evaluates gameplay tag values based on their appearance mode.
 *
 * Key features include:
 * - Gameplay Tag Container reference
 * - CheckCondition function to evaluate the condition.
 * - EvaluateCustomRule function for when a custom ruleset on the gameplay tag conditioning should be performed.
 *
 * @note This condition can be used in conditional object choosers to select objects based on gameplay tag logic.
 */
UCLASS(Blueprintable, BlueprintType, HideCategories = "Condition")
class UGorgeousGameplayTagCondition : public UGorgeousCondition
{
	GENERATED_BODY()

public:
	
	/**
	 * The class where the gameplay tag container is stored in.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	UObject* GameplayTagContainerClassReference;


	/**
	 * THe name of the uproperty of the gameplay tag container.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	FName GameplayTagContainerUPropertyName;
	
	/**
	 * The conditional mapping, when Key is present in the container then it's Value is returned for the Condition arrray.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	TMap<FGameplayTagContainerWrapper_S, int32> GameplayTagConditionMapping;

	/**
	 * The ruleset for when more than just one gameplay tags are present in the container.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	EConditionalGameplayTagChooserFightMode_E GameplayTagChooserFightMode;


	UFUNCTION(BlueprintPure, Category = "Gorgeous Gameplay Tag Condtion")
	virtual uint8 EvaluateCustomRule();
	
	/**
	 * Evaluates the gameplay tag condition based on the selected mode.
	 *
	 * @return index of the condition.
	 */
	virtual uint8 CheckCondition() override;

private:

	FGameplayTagContainer GetGameplayTagContainer() const;
};