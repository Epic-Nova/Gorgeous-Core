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

#include "Modules/ModuleInterface.h"

/**
 * This module defines the base functionality of the LogGorgeousThings plugin.
 *
 * It's responsible for loading runtime components and referenced third party libraries.
 * it is also responsible for managing the support from 4.26 up to 5.4. Everything under is marked as legacy and partly supported.
 *
 * @author Nils Bergemann
 * @since Beginning
 * @version v0.1
 * @todo Finish this Interface
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
};
