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

#include "GorgeousObjectVariableEnums.h"
#include "GorgeousObjectVariableStrctures.generated.h"

class UGorgeousObjectVariable;

USTRUCT(Blueprintable)
struct GORGEOUSCORERUNTIME_API FCastableObjectVariableWithDefaultValue
{
	GENERATED_BODY()

public:
	virtual ~FCastableObjectVariableWithDefaultValue() = default;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid ObjectVariableID = FGuid::NewGuid();
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EObjectVariableTypes_E ObjectVariableType;

	virtual TSubclassOf<UGorgeousObjectVariable> GetCorrespondingObjectVariableClass() const
	{
		return nullptr;
	}

	virtual EObjectVariableCount_E GetObjectVariableCount() const
	{
		return EObjectVariableCount_E::EObjectVariableCount_Max;
	}

#if WITH_EDITOR
    virtual void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent)
	{

	}
#endif
};