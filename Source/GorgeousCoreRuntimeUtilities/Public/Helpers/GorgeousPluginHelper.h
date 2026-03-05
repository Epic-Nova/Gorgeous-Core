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
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousSingleton.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousPluginHelper.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class IGorgeousThingsModuleInterface;
enum class EGorgeousModuleFunctionality : uint8;
//<-------------------------------------------------->

/**
 * Enum defining the load state of a gorgeous plugin.
 * 
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousPluginLoadState : uint8
{
	// All modules loaded successfully
	FullyLoaded,
	// Some modules failed to load - dependent plugins should warn about potential crashes
	PartiallyLoaded,
	// All modules failed to load - dependent plugins should not load
	NotLoaded,
	// Plugin state not yet determined
	Unknown
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Module Helper
| Functional Name: UGorgeousPluginHelper
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Helper singleton to manage gorgeous things module interfaces.
| Checks for minimum core version and dependent plugin requirements, 
| and allows for deferred registration of modules until requirements are met.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousPluginHelper : public UObject
{
	GENERATED_BODY()
	GORGEOUS_ATTACH_SINGLETON(UGorgeousPluginHelper);
	
	//<============================--- C++ Only ---=============================>
	/**
	 * Finds and returns the core gorgeous things module interface.
	 * 
	 * @return The core gorgeous things module interface, or nullptr if not found.
	 */
	IGorgeousThingsModuleInterface* GetCoreModuleInterface(const EGorgeousModuleFunctionality FunctionalityType) const;
	
	/**
	 * Registers a gorgeous things module interface with the module helper.
	 *
	 * @param ModuleInterface The module interface to register.
	 */
	bool RegisterModule(IGorgeousThingsModuleInterface* ModuleInterface);
	
	/**
	 * Unregisters a gorgeous things module interface from the module helper.
	 *
	 * @param ModuleInterface The module interface to unregister.
	 */
	void UnregisterModule(const IGorgeousThingsModuleInterface* ModuleInterface);
	
	/**
	 * Checks if a gorgeous things module interface is registered with the module helper.
	 *
	 * @param ModuleInterface The module interface to check.
	 * @return true if the module interface is registered, false otherwise.
	 */
	bool IsModuleRegistered(const IGorgeousThingsModuleInterface* ModuleInterface) const;
	
	/**
	 * Gets the list of known gorgeous plugins.
	 *
	 * @return Array of known gorgeous plugin names.
	 */
	TSet<FName> GetKnownGorgeousPlugins() const { return KnownGorgeousPlugins; }
	
	/**
	 * Checks if a gorgeous things module interface satisfies the minimum requirements.
	 *
	 * @param ModuleInterface The module interface to check.
	 * @return true if the module interface satisfies the minimum requirements, false otherwise.
	 */
	bool SatisfiesMinimumRequiredCoreVersion(const IGorgeousThingsModuleInterface* ModuleInterface) const;

	/**
	 * Defers the registration of a gorgeous things module interface until minimum requirements are met.
	 *
	 * @param ModuleInterface The module interface to defer.
	 */
	void DefferModuleRegistration(IGorgeousThingsModuleInterface* ModuleInterface);

	/**
	 * Gets the load state of a gorgeous plugin by name.
	 *
	 * @param PluginName The name of the plugin to check.
	 * @return The load state of the plugin.
	 */
	EGorgeousPluginLoadState GetPluginLoadState(const FName& PluginName) const;

	/**
	 * Records a module load failure for a plugin and updates the plugin's load state.
	 *
	 * @param ModuleInterface The module that failed to load.
	 * @param Reason The reason for the failure.
	 */
	void RecordModuleLoadFailure(const IGorgeousThingsModuleInterface* ModuleInterface, const FString& Reason);
	
	/**
	 * Checks if a plugin is installed (exists in the plugin manager).
	 *
	 * @param PluginName The name of the plugin to check.
	 * @return true if the plugin is installed, false otherwise.
	 */
	bool IsPluginInstalled(const FName& PluginName) const;
	
	/**
	 * Checks if a plugin is a Gorgeous plugin (part of the Gorgeous ecosystem).
	 *
	 * @param PluginName The name of the plugin to check.
	 * @return true if the plugin is a Gorgeous plugin, false otherwise.
	 */
	bool IsGorgeousPlugin(const FName& PluginName) const;
	
	/**
	 * Validates the entire dependency chain for a plugin.
	 * Checks that dependencies are installed and enabled.
	 * For Gorgeous plugins that have already loaded, also validates their dependencies recursively.
	 *
	 * @param PluginName The name of the plugin to validate.
	 * @param OutFailedDependencies Array to collect names of failed dependencies.
	 * @param OutFailureReasons Map of plugin names to their failure reasons.
	 * @return true if all dependencies are valid, false otherwise.
	 */
	bool ValidateDependencyChain(const FName& PluginName, TArray<FName>& OutFailedDependencies, TMap<FName, FString>& OutFailureReasons);
	
	/**
	 * Gets the declared dependencies of a Gorgeous plugin by checking its registered modules.
	 *
	 * @param PluginName The name of the plugin.
	 * @return Array of dependency plugin names.
	 */
	TArray<FName> GetGorgeousPluginDependencies(const FName& PluginName) const;

private:
	
	/**
	 * Checks if all modules for a plugin have been processed and shows a final notification if needed.
	 * @param PluginName The name of the plugin to check.
	 */
	void TryShowFinalNotification(const FName& PluginName);
	
	/**
	 * Checks if two plugins have a circular dependency through their deferred modules.
	 * Used to allow co-loading when plugins mutually depend on each other.
	 * 
	 * @param PluginA First plugin name.
	 * @param PluginB Second plugin name.
	 * @return true if the plugins have a circular deferred dependency.
	 */
	bool HasCircularDeferredDependency(const FName& PluginA, const FName& PluginB) const;
	
	/**
	 * Attempts to resolve circular deferred dependencies by allowing co-loading.
	 * This is called when a module detects that it's waiting for a plugin that is also waiting for it.
	 * 
	 * @param ModuleInterface The module attempting to register.
	 * @return true if the module can proceed with registration despite circular dependency.
	 */
	bool TryResolveCircularDependency(const IGorgeousThingsModuleInterface* ModuleInterface);
	
	/**
	 * Handles circular dependency detection: logs once per plugin pair and shows a toast notification
	 * if this combination hasn't been notified before (persisted to disk).
	 * 
	 * @param PluginA First plugin in the circular dependency.
	 * @param PluginB Second plugin in the circular dependency.
	 */
	void HandleCircularDependencyDetected(const FName& PluginA, const FName& PluginB);
	
	/**
	 * Gets a canonical key for a plugin pair (sorted alphabetically to ensure A-B == B-A).
	 * 
	 * @param PluginA First plugin name.
	 * @param PluginB Second plugin name.
	 * @return A canonical string key for the pair.
	 */
	static FString GetCircularDependencyPairKey(const FName& PluginA, const FName& PluginB);
	
	/**
	 * Loads the list of previously notified circular dependency pairs from disk.
	 */
	void LoadNotifiedCircularDependencyPairs();
	
	/**
	 * Saves the list of notified circular dependency pairs to disk.
	 */
	void SaveNotifiedCircularDependencyPairs();
	
	/**
	 * Gets the file path for storing notified circular dependency pairs.
	 */
	static FString GetCircularDependencyNotificationFilePath();
	//<------------------------------------------------------------------------->

	
	//<============================--- Variables ---============================>
	
	// Array of registered core gorgeous things module interfaces.
	TMap<IGorgeousThingsModuleInterface*, EGorgeousModuleFunctionality> RegisteredCoreModules;
	
	// Array of registered gorgeous things module interfaces.
	TMap<IGorgeousThingsModuleInterface*, EGorgeousModuleFunctionality> RegisteredModules;
	
	// Array of gorgeous things module interfaces that need to be deferred
	TArray<IGorgeousThingsModuleInterface*> ModulesToDefer;

	// Track expected module count per plugin (for determining partial vs full failure)
	TMap<FName, int32> ExpectedModulesPerPlugin;
	
	// Track successfully loaded modules per plugin
	TMap<FName, int32> LoadedModulesPerPlugin;
	
	// Track failed modules per plugin
	TMap<FName, int32> FailedModulesPerPlugin;
	
	// Track failed module types per plugin (for notification)
	TMap<FName, TArray<EGorgeousModuleFunctionality>> FailedModuleTypesPerPlugin;
	
	// Track failure reasons per plugin (for notification)
	TMap<FName, TSet<FString>> FailureReasonsPerPlugin;
	
	// Track plugins that have already shown their final notification
	TSet<FName> PluginsWithNotificationShown;
	
	// Cached load states per plugin
	TMap<FName, EGorgeousPluginLoadState> PluginLoadStates;
	
	// Known Gorgeous plugins (populated during registration)
	TSet<FName> KnownGorgeousPlugins;
	
	// Cache validation results per plugin
	TMap<FName, bool> ValidatedDependencies;
	
	// Cache validation failure reasons
	TMap<FName, FString> ValidationFailureReasons;
	
	// Detect circular dependencies during validation
	TSet<FName> PluginsCurrentlyValidating;
	
	// Track plugins that have modules currently being deferred (for circular dependency resolution)
	TSet<FName> PluginsWithDeferredModules;
	
	// Track deferred retry count per module to prevent infinite loops
	TMap<IGorgeousThingsModuleInterface*, int32> DeferredRetryCount;
	
	// Maximum number of deferred registration retries before giving up
	static constexpr int32 MaxDeferredRetries = 10;
	
	// Track circular dependency pairs that have been logged this session (to avoid duplicate logs)
	TSet<FString> LoggedCircularDependencyPairs;
	
	// Track circular dependency pairs that have shown toast notifications (loaded from/saved to disk)
	TSet<FString> NotifiedCircularDependencyPairs;
	
	// Flag to track if we've loaded the notified pairs from disk
	bool bLoadedNotifiedPairs = false;
	//<------------------------------------------------------------------------->
};