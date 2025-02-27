// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousCoreRuntimeGlobals.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Class extended by all other classes that are part of the Gorgeous Things ecosystem.
 *
 * Used to provide a static way to extend the functionality of the Gorgeous Things ecosystem.
 * Although the accessing of these functionalities is mostly done through wrapper classes that are widely accessible in the engine (blueprint).
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeous : public UObject
{
	GENERATED_BODY()
};

/**
 * Interface class extended by all other interface classes that are part of the Gorgeous Things ecosystem.
 *
 * Used to provide a static way to extend the functionality of the Gorgeous Things ecosystem for interfaces.
 *
 * @note This class is intended to be used as a base for interface implementations within the Gorgeous Things ecosystem.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInterface : public UGorgeous
{
	GENERATED_BODY()

protected:

	/**
	 * Protected constructor to prevent direct instantiation.
	 */
	explicit UGorgeousInterface() {}
};