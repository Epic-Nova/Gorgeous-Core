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
#include "Conditions/GorgeousCondition.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GorgeousConditionalObjectChooser.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UGorgeousConditionalObjectChooser : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Gorgeous Conditional Object Chooser")
	UGorgeousObjectVariable* DecideCondition() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	UGorgeousCondition* ConditionCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta = (ExposeOnSpawn))
	TArray<UGorgeousObjectVariable*> Conditions;
};
