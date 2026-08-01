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

	TArray<FGameplayTag> FoundTags;
	for (const auto& Pair : GameplayTagConditionMapping)
	{
		if (GetGameplayTagContainer().HasAny(Pair.Key.Container))
		{
			FoundTags.Append(Pair.Key.Container.GetGameplayTagArray());
		}
	}

	if (FoundTags.IsEmpty())
	{
		return 0;
	}

	switch (GameplayTagChooserFightMode)
	{
	case EConditionalGameplayTagChooserFightMode_E::FIRST:
		{
			for (const auto& Pair : GameplayTagConditionMapping)
			{
				if (Pair.Key.Container.HasTag(FoundTags[0]))
				{
					return Pair.Value;
				}
			}
			return 0;
		}
	case EConditionalGameplayTagChooserFightMode_E::LAST:
		{
			for (auto It = GameplayTagConditionMapping.CreateIterator(); It; ++It)
			{
				if (It.Key().Container.HasTag(FoundTags.Last()))
				{
					return It.Value();
				}
			}
			return 0;
		}
	case EConditionalGameplayTagChooserFightMode_E::RANDOM:
		{
			const int32 RandomIndex = FMath::RandRange(0, FoundTags.Num() - 1);
			for (const auto& Pair : GameplayTagConditionMapping)
			{
				if (Pair.Key.Container.HasTag(FoundTags[RandomIndex]))
				{
					return Pair.Value;
				}
			}
			return 0;
		}
	default:
		return 0;
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