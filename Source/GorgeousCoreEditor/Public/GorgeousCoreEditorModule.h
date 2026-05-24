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
#include "IGorgeousThingsModuleInterface.h"
//<-------------------------------------------------------------------------->

/**
 * This module defines the editor functionality of the Gorgeous Things plugin.
 *
 * @author Nils Bergemann
 */
class FGorgeousCoreEditorModule final : public IGorgeousThingsModuleInterface
{
	//<============================--- Overrides ---============================>
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
	virtual EGorgeousModuleFunctionality GetModuleFunctionality() const override { return EGorgeousModuleFunctionality::Editor; }
	
	/**
	 * Returns an array of names from other gorgeous plugins that this module depends on.
	 * This is used for the shop extension to download the full dependency chain for a gorgeous plugin.
	 * 
	 * @return All plugins that provide functionality that is used within this current module.
	 */
	virtual TArray<FName> GetDependentPlugins() const override { return TArray<FName>(); }
	
	/**
	 * Returns the minimum required core version for this module to work properly.
	 * This is used to ensure compatibility between different versions of gorgeous plugins.
	 * 
	 * @return The minimum required core version as an integer. Represented as Major * 100 + (Minor * 10)
	 */
	virtual int32 GetMinimumRequiredCoreVersion() const override { return 100; /* Version 1.0 */ }
	
	/**
	 * Returns whether this module provides core functionality.
	 * Core functionality modules are essential for the operation of the gorgeous plugin family.
	 * 
	 * @return true if this module provides core functionality, false otherwise.
	 */
	virtual bool ProvidesCoreFunctionality() const override { return true; }
	
#if WITH_EDITOR
	virtual void ValidateGorgeousModule(FDataValidationContext& InContext) override;
#endif
	//<------------------------------------------------------------------------->

	
	//<============================--- Variables ---============================>
private:
	
	// The delegate handles for the BeginPIE and EndPIE editor delegates, used to unregister the delegates on module shutdown.
	FDelegateHandle BeginPIEHandle;
	
	// The delegate handle for the EndPIE editor delegate, used to unregister the delegate on module shutdown.
	FDelegateHandle EndPIEHandle;

	//@TODO: Planned for version 1.1 and upwards
	//TSharedPtr<FRainbowPinFactory> Factory;
	//TSharedPtr<FGorgeousRainbowGraphPinFactory> RainbowPinFactory;
	//TSharedPtr<FGorgeousRainbowConnectionFactory> RainbowConnectionFactory;
	//<------------------------------------------------------------------------->
};