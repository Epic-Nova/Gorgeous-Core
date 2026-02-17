// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

#include "IGorgeousThingsModuleInterface.h"

/**
 * This module defines the editor utilities for the Gorgeous Things plugin.
 *
 * @author Nils Bergemann
 */
class FGorgeousCoreEditorUtilitiesModule final : public IGorgeousThingsModuleInterface
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
	 * Returns the name of the plugin that this module belongs to.
	 * 
	 * @return The name of the plugin as an FName.
	 */
	virtual FName GetBelongingPluginName() const override { return FName("GorgeousCore"); }
	
	/**
	 * Returns the functionality type of this gorgeous module.
	 * 
	 * @return The functionality type as an EGorgeousModuleFunctionality enum value.
	 */
	virtual EGorgeousModuleFunctionality GetModuleFunctionality() const override { return EGorgeousModuleFunctionality::EditorUtilities; }

	/**
	 * We return false here because we don't want the core to be participated in any hot reload/live coding actions.
	 * As long term debugging shows that reloading the gorgeous core module invokes the corruption of the current object variable registry instance, and finally a crash of the current unreal instance.
	 */
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}
	
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
	
private:

	
	void HandleGorgeousLogEntry(const FGorgeousLogEntry& Entry);
	void HandleBeginPIE(bool bIsSimulating);
	void HandleEndPIE(bool bIsSimulating);

private:
	FDelegateHandle LogEntryHandle;
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
	FName RegisteredLogListingName = NAME_None;
};