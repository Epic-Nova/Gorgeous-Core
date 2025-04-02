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
//<-------------------------=== Engine Includes ===-------------------------->
#include "Modules/ModuleInterface.h"
//<-------------------------------------------------------------------------->

/**
 * This module defines the base functionality of the Gorgeous Things plugin.
 *
 * It's responsible for providing base interface functions for the gorgeous plugin family.
 * It is also responsible for managing the support from 4.26 up to 5.4. Everything under is marked as legacy and partly supported.
 *
 * @author Nils Bergemann
 * @since Beginning
 * @version v1.1
 */
class IGorgeousThingsModuleInterface : public IModuleInterface
{
public:

	/**
	 * The override function of the module interface that is called to load up this module.
	 */
	virtual void StartupModule() override = 0;

	/**
	 * The override function of the module interface that is called to unload this module.
	 */
	virtual void ShutdownModule() override = 0;

	/**
	 * Returns an array of names from other gorgeous plugins that this plugin depends on.
	 * This is used for the shop extension to download the full dependency chain of a gorgeous plugin.
	 * 
	 * @return All plugins that provide functionality that is used within this current module.
	 */
	virtual TArray<FName> GetDependentPlugins() = 0;
};