/*==========================================================================>
|			    Gorgeous Events - Events functionality provider				|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousBaseWorldContextUObject.generated.h"
//<-------------------------------------------------------------------------->

/*============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous BaseWorld Context UObject
| Functional Name: -
| Parent Class: /Script/Engine.UObject
| Class Suffix: -
| Author: Sven Maibaum & Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides a UObject with a default handle to the world.
<--------------------------------------------------------------------------->
<==========================================================================*/
UCLASS(Abstract, Blueprintable)
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousBaseWorldContextUObject : public UObject
{
	GENERATED_BODY()
	
	//<============================--- Overrides ---============================>
protected:
	
	virtual UWorld* GetWorld() const override;
	
	//<------------------------------------------------------------------------->

	
	//<============================--- Variables ---============================>
	
	UPROPERTY()
	mutable TObjectPtr<UObject> CachedOwner;
	
	//<------------------------------------------------------------------------->
};