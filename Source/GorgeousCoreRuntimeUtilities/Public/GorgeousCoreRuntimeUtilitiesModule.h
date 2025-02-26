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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "IGorgeousThingsModuleInterface.h"
//<-------------------------------------------------------------------------->

/**
 * This module defines the runtime utilities of the Gorgeous Things plugin.
 *
 * It's responsible for loading runtime components and referenced third-party libraries.
 *
 * @author Nils Bergemann
 * @since Beginning
 * @version v1.0
 */
class FGorgeousCoreRuntimeUtilitiesModule final : public IGorgeousThingsModuleInterface
{
public:

	/**
	 * The override function of the module interface that is called to load up this module.
	 */
	virtual void StartupModule() override;

	/**
	 * The override function of the module interface that is called to unload this module.
	 */
	virtual void ShutdownModule() override;

};