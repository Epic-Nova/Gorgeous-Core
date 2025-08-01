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
 UCLASS(Abstract, Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousBaseWorldContextUObject : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Sets the cached owner of this object. This is used as an absolute failsafe to ensure the consistent availability of the world to the current active instance.
	 * 
	 * @param NewCachedOwner the new chaced owner that has the capability to a valid GetWorld() call.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Base World Context UObject")
	void SetCachedOwner(const TObjectPtr<UObject>& NewCachedOwner) const { CachedOwner = NewCachedOwner; }
	
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