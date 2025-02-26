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
//--------------=== Third Party & Miscellaneous Includes ===----------------->
//<-------------------------------------------------------------------------->

/**
 * This module defines the core runtime of the Gorgeous Things plugin.
 *
 * It's the most important module of the plugin, as it's responsible for loading the core runtime components and referenced third-party libraries.
 *
 * @author Nils Bergemann
 * @since Beginning
 * @version v1.0
 */
class FGorgeousCoreRuntimeModule final : public IGorgeousThingsModuleInterface
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