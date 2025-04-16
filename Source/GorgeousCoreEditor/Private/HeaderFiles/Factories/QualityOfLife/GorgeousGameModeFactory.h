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
#include "GorgeousCoreEditorUtilitiesMinimalShared.h"
#include "GorgeousCoreMinimalShared.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousGameModeFactory.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Factory class responsible for creating instances of AGorgeousGameMode.
 *
 * Key features include:
 * - Registration of AGorgeousGameMode as the target class.
 *
 * The configuration is encapsulated using FGorgeousFactoryInfo_S, which is passed to the base class.
 *
 * @author Nils Bergemann
 */
UCLASS()
class UGorgeousGameModeFactory : public UGorgeousFactory
{
	GENERATED_BODY()
	
public:

	/**
	 * Default constructor that sets up the factory with metadata describing its target class
	 * and behavior within the Unreal Editor asset system.
	 */
	UGorgeousGameModeFactory()
	{
		SetFactoryInformation(FGorgeousFactoryInfo_S(AGorgeousGameMode::StaticClass(), true, false, true, false));
	}
};
