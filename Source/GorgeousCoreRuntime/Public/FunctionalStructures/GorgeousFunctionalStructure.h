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
#include "GorgeousFunctionalStructure.generated.h"

USTRUCT(Blueprintable, DisplayName = "Gorgeous Functional Structure")
struct GORGEOUSCORERUNTIME_API FGorgeousFunctionalStructure_S
{
	GENERATED_BODY()

	FGorgeousFunctionalStructure_S()
	{
		Identifier = FGuid::NewGuid();
	}
	
	virtual ~FGorgeousFunctionalStructure_S() = default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid Identifier;

#if WITH_EDITOR
	
	virtual void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent) {};
	
	virtual void PreEditChangeProperty(TSharedRef<IPropertyHandle> PropertyHandle) {};
#endif //WITH_EDITOR
};