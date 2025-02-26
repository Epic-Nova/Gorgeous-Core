/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/
#pragma once
#include "GameplayTagContainer.h"
#include "GorgeousCondition.h"

#include "GorgeousGameplayTagCondition.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UGorgeousGameplayTagCondition : public UGorgeousCondition
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag A;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag B;
	
};