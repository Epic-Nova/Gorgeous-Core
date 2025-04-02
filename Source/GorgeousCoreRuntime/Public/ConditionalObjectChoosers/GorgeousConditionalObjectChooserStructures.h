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
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousConditionalObjectChooserStructures.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A wrapper struct for FGameplayTagContainer, enabling its use as a key in TMap and TSet.
 *
 * Key features include:
 * - Encapsulates an FGameplayTagContainer.
 * - Provides an explicit constructor for initialization.
 * - Implements operator== for equality comparison.
 * - Implements GetTypeHash for efficient hashing in associative containers.
 *
 * @author Nils Bergemann
 * @note This struct is necessary because FGameplayTagContainer does not inherently support comparison or hashing, which are required for use as keys in TMap and TSet.
 */
USTRUCT(Blueprintable)
struct FGameplayTagContainerWrapper_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Container;

	FGameplayTagContainerWrapper_S() : Container() {}
	
	explicit FGameplayTagContainerWrapper_S(const FGameplayTagContainer& InContainer) : Container(InContainer) {}

	bool operator==(const FGameplayTagContainerWrapper_S& Other) const
	{
		return Container == Other.Container;
	}

	friend uint32 GetTypeHash(const FGameplayTagContainerWrapper_S& Wrapper)
	{
		uint32 Hash = 0;
		for (const FGameplayTag& Tag : Wrapper.Container)
		{
			Hash = HashCombine(Hash, GetTypeHash(Tag));
		}
		return Hash;
	}
};