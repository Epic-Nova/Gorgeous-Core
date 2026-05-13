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
#include "GorgeousGameplayTagCondition.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A condition that evaluates gameplay tag values based on their appearance mode.
 * 
 * Uses the specified class reference and property name to retrieve a gameplay tag container, then evaluates the condition based on the selected fight mode and mapping.
 * 
 * @author Nils Bergemann
 * @note This condition can be used in conditional object choosers to select objects based on gameplay tag logic.
 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType, HideCategories = "Gorgeous Condition", 
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Overview", 
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Conditions/GameplayTagCondition", 
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ConditionalObjectChoosers/Examples/"
		))
class UGorgeousGameplayTagCondition final : public UGorgeousCondition
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
public:
	
	/**
	 * Evaluates a custom specified rule for selecting the condition.
	 * 
	 * @return A valid index for a condition.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintPure, Category = "Gorgeous Gameplay Tag Condtion")
	uint8 EvaluateCustomRule();
	
	/**
	 * Finds the condition mapping for the given gameplay tag container based on the specified mapping.
	 *
	 * @param Container The gameplay tag container to check against the mapping.
	 * @param OutValue The value associated with the first matching tag container in the mapping, if found.
	 * @return true if a matching condition index was found, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Gameplay Tag Condtion", meta = (CompactNodeTitle = "Find in Condition Mapping"))
	bool FindConditionMappingForTagContainer(const FGameplayTagContainer& Container, int32& OutValue) const;
	
	/**
	 * Evaluates the gameplay tag condition based on the selected mode.
	 *
	 * @return index of the condition.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Gameplay Tag Condtion")
	virtual uint8 CheckCondition_Implementation() override;
	//<------------------------------------------------------------------------->

	
	//<====================--- UAT/UBT Exposed Variables ---====================>

	// The class where the gameplay tag container is stored in.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	UObject* GameplayTagContainerClassReference;
	
	// THe name of the UProperty of the gameplay tag container.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	FName GameplayTagContainerUPropertyName;
	
	/**
	 * The conditional mapping, when Key is present in the container then it's Value is returned for the Condition array.
	 * Only used when the fight mode is not set to RULE, otherwise the custom rule is evaluated to determine the condition index.
	 * In some rare cases you might want to use the mapping even with a custom rule, therefore we don't add a EditCondition on the mapping to hide it when the fight mode is set to RULE.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	TMap<FGameplayTagContainerWrapper_S, int32> GameplayTagConditionMapping;

	// The ruleset for when more than just one gameplay tags are present in the container.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Gameplay Tag Condition")
	EConditionalGameplayTagChooserFightMode_E GameplayTagChooserFightMode;
	//<------------------------------------------------------------------------->

	
	//<============================--- C++ Only ---=============================>
private:
	
	/**
	 * Returns the gameplay tag container from the specified class and property name.
	 * 
	 * @return The gameplay tag container from the specified class and property name.
	 */
	FGameplayTagContainer GetGameplayTagContainer() const;
	//<------------------------------------------------------------------------->
};