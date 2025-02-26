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
#include "FunctionalStructures/GorgeousFunctionalStructure.h"

#include "GorgeousFunctionalStructures_GIS.generated.h"

UCLASS(BlueprintType, DisplayName = "Gorgeous Functional Structures")
class UGorgeousFunctionalStructures_GIS : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RegisterFunctionalStructure(UPARAM(ref) FGorgeousFunctionalStructure_S& Structure);

	UFUNCTION(BlueprintCallable)
	void UnregisterFunctionalStructure(UPARAM(ref) FGorgeousFunctionalStructure_S& Structure);
};