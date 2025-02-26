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

#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "GorgeousCoreRuntimeGlobals.generated.h"

/**
 * Class extended by all other classes that are part of the Gorgeous Things ecosystem.
 *
 * Used to provide a static way to extend the functionality of the Gorgeous Things ecosystem.
 * Although the accessing of these functionalities is mostly done trough wrapper classes that are widely accessible in the engine (blueprint).
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeous : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInterface : public UGorgeous
{
	GENERATED_BODY()


protected:

	explicit UGorgeousInterface() {}
};