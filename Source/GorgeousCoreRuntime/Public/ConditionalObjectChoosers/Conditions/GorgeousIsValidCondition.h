// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#pragma once
#include "GorgeousCondition.h"
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooserEnums.h"

#include "GorgeousIsValidCondition.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UGorgeousIsValidCondition : public UGorgeousCondition
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* A;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* B;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Conditional Object Chooser")
	virtual bool CheckCondition() override;
	
};
