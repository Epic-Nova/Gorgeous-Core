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
#include "ConditionalObjectChoosers/Conditions/GorgeousGameplayTagCondition.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooserTypes.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousGameplayTagCondition Implementation
//=============================================================================

uint8 UGorgeousGameplayTagCondition::CheckCondition_Implementation()
{
	if (GameplayTagChooserFightMode == EConditionalGameplayTagChooserFightMode_E::RULE)
	{
		return EvaluateCustomRule();
	}
	
	if (GetGameplayTagContainer().IsEmpty())
	{
		return 0;
	}

	const FGameplayTagContainer SourceTags = GetGameplayTagContainer();
	TArray<uint8> MatchingValues;
	for (const auto& Pair : GameplayTagConditionMapping)
	{
		if (SourceTags.HasAny(Pair.Key.Container) && Pair.Value >= 0 && Pair.Value <= MAX_uint8)
		{
			MatchingValues.Add(static_cast<uint8>(Pair.Value));
		}
	}
	if (MatchingValues.IsEmpty()) return 0;
	switch (GameplayTagChooserFightMode)
	{
	case EConditionalGameplayTagChooserFightMode_E::FIRST: return MatchingValues[0];
	case EConditionalGameplayTagChooserFightMode_E::LAST: return MatchingValues.Last();
	case EConditionalGameplayTagChooserFightMode_E::RANDOM: return MatchingValues[FMath::RandHelper(MatchingValues.Num())];
	default: return 0;
	}
}

bool UGorgeousGameplayTagCondition::FindConditionMappingForTagContainer(const FGameplayTagContainer& Container,
	int32& OutValue) const
{
	FGameplayTagContainerWrapper_S AssembledContainer;
	AssembledContainer.Container = Container;
	
	if (const int32* FoundValue = GameplayTagConditionMapping.Find(AssembledContainer))
	{
		OutValue = *FoundValue;
		return true;
	}
	OutValue = -1;
	return false;
}

FGameplayTagContainer UGorgeousGameplayTagCondition::GetGameplayTagContainer() const
{
	FGameplayTagContainer Container;

	if (GameplayTagContainerClassReference && GameplayTagContainerUPropertyName != NAME_None)
	{
		if (FProperty* Property = GameplayTagContainerClassReference->GetClass()->FindPropertyByName(GameplayTagContainerUPropertyName))
		{
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property); StructProperty && StructProperty->Struct == FGameplayTagContainer::StaticStruct())
			{
				if (void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(GameplayTagContainerClassReference))
				{
					Container = *static_cast<FGameplayTagContainer*>(ValuePtr);
				}
			}
		}
	}

	return Container;
}