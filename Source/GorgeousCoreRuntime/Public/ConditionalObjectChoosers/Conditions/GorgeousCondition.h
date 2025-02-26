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
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooserEnums.h"
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "GorgeousCondition.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class UGorgeousCondition : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConditionalChooserMode_E Mode;
	
	virtual uint8 CheckCondition();
};
