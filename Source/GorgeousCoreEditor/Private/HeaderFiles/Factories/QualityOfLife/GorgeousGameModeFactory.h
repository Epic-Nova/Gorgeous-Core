// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreMinimalShared.h"
#include "GorgeousCoreEditorUtilitiesMinimalShared.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousGameModeFactory.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Factory class responsible for creating instances of AGorgeousGameMode.
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
		SetFactoryInformation(FGorgeousFactoryInfo_S(AGorgeousGameMode::StaticClass(), true, false, true, false,
			UGorgeousGameModeBlueprint::StaticClass()));
	}
};