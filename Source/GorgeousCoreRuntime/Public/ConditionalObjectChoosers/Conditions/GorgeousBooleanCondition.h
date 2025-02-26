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
#include "GorgeousBooleanCondition.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UGorgeousBooleanCondition : public UGorgeousCondition
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool A;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool B;

	virtual bool CheckCondition() override;
};
