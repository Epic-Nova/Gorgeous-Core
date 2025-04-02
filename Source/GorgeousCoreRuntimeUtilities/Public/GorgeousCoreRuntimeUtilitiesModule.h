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
#include "IGorgeousThingsModuleInterface.h"
//<-------------------------------------------------------------------------->

/**
 * This module defines the runtime utilities of the Gorgeous Things plugin.
 *
 * It's responsible for loading runtime components and referenced third-party libraries.
 *
 * @author Nils Bergemann
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

	/**
	 * We return false here because we don't want the core to be participated in any hot reload/live coding actions.
	 * As long term debugging shows that reloading the gorgeous core module invokes the corruption of the current object variable registry instance, and finally a crash of the current unreal instance.
	 */
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}
	
	/**
	 * Returns an array of names from other gorgeous plugins that this plugin depends on.
	 * This is used for the shop extension to download the full dependency chain of a gorgeous plugin.
	 * 
	 * @return All plugins that provide functionality that is used within this current module.
	 */
	virtual TArray<FName> GetDependentPlugins() override
	{
		return TArray<FName>();
	}
};