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

#include "UObject/Interface.h"
#include "GorgeousObjectVariableInteraction_I.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousObjectVariableInteraction_I : public UInterface
{
	GENERATED_BODY()
};


class GORGEOUSCORERUNTIME_API IGorgeousObjectVariableInteraction_I
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables")
	FGuid GetUniqueIdentifierForObjectVariable(UObject* ObjectVariable);
};
