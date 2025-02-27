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
#include "GorgeousBaseWorldContextUObject.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UWorld;
//<-------------------------------------------------->

/**
 * Provides a UObject with a default handle to the world.
 *
 * Key features include:
 * - Abstract base class for UObjects requiring world context.
 * - Overrides GetWorld() to provide world context.
 * - Caches owner object for efficient world retrieval.
 */
UCLASS(Abstract, Blueprintable)
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousBaseWorldContextUObject : public UObject
{
	GENERATED_BODY()

protected:

	/**
	 * Gets the world context for this object.
	 *
	 * @return The UWorld associated with this object.
	 */
	virtual UWorld* GetWorld() const override;

	/**
	 * Cached owner object used to determine the world context.
	 */
	UPROPERTY()
	mutable TObjectPtr<UObject> CachedOwner;
};