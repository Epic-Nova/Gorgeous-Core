// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

#include "IGorgeousThingsModuleInterface.h"
#include "Templates/UniquePtr.h"

class FGorgeousCoreInsightMatrixProvider;

/**
 * This module defines the core runtime of the Gorgeous Things plugin.
 *
 * It's the most important module of the plugin, as it's responsible for loading the core runtime components and referenced third-party libraries.
 *
 * @author Nils Bergemann
 */
class FGorgeousCoreRuntimeModule final : public IGorgeousThingsModuleInterface
{
public:

	/**
	 * The override function of the module interface that is called to load up this module.
	 */
	virtual void GorgeousStartupModule() override;

	/**
	 * The override function of the module interface that is called to unload this module.
	 */
	virtual void GorgeousShutdownModule() override;
	
	/**
	 * Returns the functionality type of this gorgeous module.
	 * 
	 * @return The functionality type as an EGorgeousModuleFunctionality enum value.
	 */
	virtual FName GetBelongingPluginName() const override { return FName("GorgeousCore"); }
	
	/**
	 * Returns the functionality type of this gorgeous module.
	 * 
	 * @return The functionality type as an EGorgeousModuleFunctionality enum value.
	 */
	virtual EGorgeousModuleFunctionality GetModuleFunctionality() const override { return EGorgeousModuleFunctionality::Runtime; }

	/**
	 * Returns an array of names from other gorgeous plugins that this module depends on.
	 * This is used for the shop extension to download the full dependency chain for a gorgeous plugin.
	 * 
	 * @return All plugins that provide functionality that is used within this current module.
	 */
	virtual TArray<FName> GetDependentPlugins() const override;
	
	/**
	 * Returns the minimum required core version for this module to work properly.
	 * This is used to ensure compatibility between different versions of gorgeous plugins.
	 * 
	 * @return The minimum required core version as an integer. Represented as Major * 100 + (Minor * 10)
	 */
	virtual int32 GetMinimumRequiredCoreVersion() const override;
	
	/**
	 * Returns whether this module provides core functionality.
	 * Core functionality modules are essential for the operation of the gorgeous plugin family.
	 * 
	 * @return true if this module provides core functionality, false otherwise.
	 */
	virtual bool ProvidesCoreFunctionality() const override { return true; }
};