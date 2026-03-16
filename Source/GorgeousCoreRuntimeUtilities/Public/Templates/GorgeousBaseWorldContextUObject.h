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
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousBaseWorldContextUObject.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UWorld;
//<-------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Base World Context UObject
| Functional Name: UGorgeousBaseWorldContextUObject
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| An abstract base class for UObjects that require access to the world context.
| This class overrides the GetWorld() method to provide the appropriate UWorld
| instance based on a cached owner object. It is designed to be extended by
| other classes that need world context functionality.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(Abstract, Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousBaseWorldContextUObject : public UObject
{
	GENERATED_BODY()

public:
	
	/**
	 * Sets the cached owner of this object. This is used as an absolute failsafe to ensure the consistent availability of the world to the current active instance.
	 * 
	 * @param NewFallbackOwner the new chaced owner that has the capability to a valid GetWorld() call.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Base World Context UObject")
	void SetFallbackOwner(UObject* NewFallbackOwner) const { FallbackOwner = MakeWeakObjectPtr(NewFallbackOwner); }

protected:
	
	/**
	 * Gets the world context for this object.
	 *
	 * @return The UWorld associated with this object.
	 */
	virtual UWorld* GetWorld() const override;

	// The cached owner object used to retrieve the world context.
	mutable TWeakObjectPtr<UObject> FallbackOwner;
};