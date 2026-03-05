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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Modules/ModuleManager.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "IGorgeousThingsModuleInterface.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class FSlateStyleSet;
class IGorgeousInsightMatrixProvider;
//<-------------------------------------------------->

/**
 * Enum defining the different functionalities a gorgeous module can provide.
 */
UENUM(BlueprintType)
enum class EGorgeousModuleFunctionality : uint8
{
	Runtime,
	RuntimeUtilities,
	Editor,
	EditorUtilities,
	ThirdParty
};

/**
 * Enum defining how to handle failures during gorgeous module loading.
 */
UENUM(BlueprintType)
enum class EGorgeousModuleLoadFailureHandling : uint8
{
	ContinueOnFailure,
	ShutdownOnFailure
};

/**
 * This module defines the base functionality of the Gorgeous Things plugin.
 *
 * It's responsible for providing base interface functions for the gorgeous plugin family.
 * It is also responsible for managing the support from 4.26 up to 5.4. Everything under is marked as legacy and partly supported.
 *
 * @author Nils Bergemann
 */
class GORGEOUSCORERUNTIMEUTILITIES_API IGorgeousThingsModuleInterface : public IModuleInterface
{
	
	//<============================--- Overrides ---============================>
	
	/**
	 * The override function of the module interface that is called to load up this module.
	 */
	virtual void StartupModule() override;

	/**
	 * The override function of the module interface that is called to unload this module.
	 */
	virtual void ShutdownModule() override;
	//<------------------------------------------------------------------------->

	
	//<============================--- C++ Only ---=============================>
public:
	
	/**
	 * The public wrapper function of the module interface that is called to load up this module.
	 */
	virtual void GorgeousStartupModule() = 0;
	
	/**
	 * The public wrapper function of the module interface that is called to unload this module.
	 */
	virtual void GorgeousShutdownModule() = 0;
	
	/**
	 * Returns the name of the gorgeous plugin that this module belongs to.
	 * 
	 * @return The name of the gorgeous plugin that this module belongs to.
	 */
	virtual FName GetBelongingPluginName() const = 0;
	
	/**
	 * Returns the functionality type of this gorgeous module.
	 * 
	 * @return The functionality type of this gorgeous module.
	 */
	virtual EGorgeousModuleFunctionality GetModuleFunctionality() const = 0;

	/**
	 * Returns how to handle failures during gorgeous module loading.
	 * Defaults to ContinueOnFailure.
	 * 
	 * @return  How to handle failures during gorgeous module loading.
	 */
	virtual EGorgeousModuleLoadFailureHandling GetLoadFailureHandling() const;
	
	/**
	 * Returns an array of names from other gorgeous plugins that this module depends on.
	 * This is used for the shop extension to download the full dependency chain for a gorgeous plugin.
	 * 
	 * @return All plugins that provide functionality that is used within this current module.
	 */
	virtual TArray<FName> GetDependentPlugins() const = 0;
	
	/**
	 * Returns the minimum required core version for this module to work properly.
	 * This is used to ensure compatibility between different versions of gorgeous plugins.
	 * 
	 * @return The minimum required core version as an integer. Represented as Major * 100 + (Minor * 10)
	 */
	virtual int32 GetMinimumRequiredCoreVersion() const = 0;
	
	/**
	 * Returns the style set provided by this module for editor modules.
	 * This is used to define used slate brushes and other style elements for editor modules of the gorgeous plugin family.
	 * 
	 * @return The style set as a shared pointer to an FSlateStyleSet. Defaults to nullptr.
	 */
	virtual TSharedPtr<FSlateStyleSet> GetModuleStyleSet() const { return ModuleStyleSet; }
	
	/**
	 * Returns whether this module provides core functionality.
	 * Core functionality modules are essential for the operation of the gorgeous plugin family.
	 * 
	 * @return true if this module provides core functionality, false otherwise.
	 */
	virtual bool ProvidesCoreFunctionality() const;
	
protected:

	/** Returns the Insight Matrix provider instance for this module (if any). */
	IGorgeousInsightMatrixProvider* GetInsightProvider() const { return InsightProvider; }
	//<------------------------------------------------------------------------->

	
	//<============================--- Variables ---============================>
	
	// Optional style set for editor modules.
	TSharedPtr<FSlateStyleSet> ModuleStyleSet;
	
	// Optional Insight Matrix provider for runtime modules.
	IGorgeousInsightMatrixProvider* InsightProvider = nullptr;
	//<------------------------------------------------------------------------->
};