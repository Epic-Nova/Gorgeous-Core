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
 * A wrapper struct for FGameplayTagContainer to support hashing and equality checks.
 *
 * This struct is primarily intended to enable the use of Gameplay Tag Containers as keys in
 * TSet or TMap by providing an explicit `GetTypeHash` implementation and equality operator.
 * It can also be exposed to Blueprints if needed via the `Blueprintable` specifier.
 *
 * @note While FGameplayTagContainer already supports comparison, this wrapper adds better support
 *       for hash-based containers.
 *
 * @author Nils Bergemann
 */
USTRUCT(Blueprintable)
struct FGameplayTagContainerWrapper_S
{
	GENERATED_BODY()

	/**
	 * The underlying Gameplay Tag Container.
	 */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Container;

	/**
	 * Default constructor. Initializes an empty container.
	 */
	FGameplayTagContainerWrapper_S() : Container() {}

	/**
	 * Constructs the wrapper using an existing Gameplay Tag Container.
	 *
	 * @param InContainer The tag container to wrap.
	 */
	explicit FGameplayTagContainerWrapper_S(const FGameplayTagContainer& InContainer) : Container(InContainer) {}

	/**
	 * Compares two wrappers for equality based on their tag containers.
	 *
	 * @param Other The wrapper to compare against.
	 * @return true if both containers contain the same tags.
	 */
	bool operator==(const FGameplayTagContainerWrapper_S& Other) const
	{
		return Container == Other.Container;
	}

	/**
	 * Computes a hash value for the wrapper, combining the hashes of each tag inside.
	 *
	 * @param Wrapper The wrapper to hash.
	 * @return A 32-bit hash value.
	 */
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