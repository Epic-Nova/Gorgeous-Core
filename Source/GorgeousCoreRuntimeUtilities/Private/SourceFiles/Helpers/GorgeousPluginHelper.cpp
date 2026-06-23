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
#include "Helpers/GorgeousPluginHelper.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/SecureHash.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "IGorgeousThingsModuleInterface.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/GorgeousLoggingHelper.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Interfaces/IPluginManager.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousPluginHelper Implementation
//=============================================================================

// Helper function to convert version number to readable format (e.g., 90 -> "0.9")
static FString VersionToString(const int32 Version)
{
	const int32 Major = Version / 100;
	const int32 Minor = (Version % 100) / 10;
	return FString::Printf(TEXT("%d.%d"), Major, Minor);
}

// Helper function to get short module type name (e.g., "Runtime" instead of "EGorgeousModuleFunctionality::Runtime")
static FString GetModuleTypeString(EGorgeousModuleFunctionality ModuleType)
{
	return StaticEnum<EGorgeousModuleFunctionality>()->GetNameStringByValue(static_cast<int64>(ModuleType));
}

IGorgeousThingsModuleInterface* UGorgeousPluginHelper::GetCoreModuleInterface(
	const EGorgeousModuleFunctionality FunctionalityType) const
{
	// Search registered core modules for the requested functionality type
	for (const auto& Element : RegisteredCoreModules)
	{
		if (Element.Value == FunctionalityType)
		{
			return Element.Key;
		}
	}
	return nullptr;
}

bool UGorgeousPluginHelper::RegisterModule(IGorgeousThingsModuleInterface* ModuleInterface)
{
	if (IsModuleRegistered(ModuleInterface))
		return false;

	const FName PluginName = ModuleInterface->GetBelongingPluginName();
	
	// Track this as a known Gorgeous plugin
	KnownGorgeousPlugins.Add(PluginName);
	
	// Track expected modules per plugin
	ExpectedModulesPerPlugin.FindOrAdd(PluginName)++;

	// Core modules bypass all checks - they must register first to bootstrap the system
	if (ModuleInterface->ProvidesCoreFunctionality())
	{
		RegisteredCoreModules.Add(ModuleInterface, ModuleInterface->GetModuleFunctionality());
		LoadedModulesPerPlugin.FindOrAdd(PluginName)++;
		
		GT_I_LOG("GT.Core.PluginHelper", TEXT("[%s] Successfully registered core module type: %s"),
		         *PluginName.ToString(),
		         *GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
		
		return true;
	}

	// Non-core modules require version compatibility and dependency validation
	if (!SatisfiesMinimumRequiredCoreVersion(ModuleInterface))
	{
		// Null-safe access - CoreRuntime may not exist yet during early startup
		const IGorgeousThingsModuleInterface* CoreRuntime = GetCoreModuleInterface(EGorgeousModuleFunctionality::Runtime);
		const int32 CoreVersion = CoreRuntime ? CoreRuntime->GetMinimumRequiredCoreVersion() : 0;
		
		const FString Reason = FString::Printf(TEXT("Incompatible core version. Required: %s, Got: %s"),
			*VersionToString(ModuleInterface->GetMinimumRequiredCoreVersion()), *VersionToString(CoreVersion));
		
		RecordModuleLoadFailure(ModuleInterface, Reason);

		switch (ModuleInterface->GetLoadFailureHandling())
		{
		case EGorgeousModuleLoadFailureHandling::ContinueOnFailure:
			GT_I_LOG("GT.Core.PluginHelper", TEXT("[%s] Continuing on failure due to failure handling policy of module type: %s"),
				*PluginName.ToString(),
				*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
			break;
		case EGorgeousModuleLoadFailureHandling::ShutdownOnFailure:
			GT_F_LOG("GT.Core.PluginHelper", TEXT("[%s] Shutting down due to failure handling policy of module type: %s"),
				*PluginName.ToString(),
				*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
			break;
		}
		
		return false;
	}
	
	// Validate the entire dependency chain before allowing registration
	TArray<FName> FailedDependencies;
	TMap<FName, FString> FailureReasons;
	
	for (const FName& DependentPlugin : ModuleInterface->GetDependentPlugins())
	{
		if (!ValidateDependencyChain(DependentPlugin, FailedDependencies, FailureReasons))
		{
			// Dependency chain validation failed - collect all failures before failing
			GT_E_LOG("GT.Core.PluginHelper", TEXT("[%s] Dependency chain validation failed for plugin '%s' required by module type: %s"),
				*PluginName.ToString(),
				*DependentPlugin.ToString(),
				*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
		}
	}
	
	// If any dependencies failed validation, fail this module
	if (FailedDependencies.Num() > 0)
	{
		FString CombinedReasons;
		for (const FName& FailedDep : FailedDependencies)
		{
			const FString* Reason = FailureReasons.Find(FailedDep);
			if (!CombinedReasons.IsEmpty()) CombinedReasons += TEXT("; ");
			CombinedReasons += FString::Printf(TEXT("%s: %s"), 
				*FailedDep.ToString(), 
				Reason ? **Reason : TEXT("Unknown reason"));
		}
		
		RecordModuleLoadFailure(ModuleInterface, FString::Printf(TEXT("Dependency chain failed: %s"), *CombinedReasons));
		return false;
	}
	
	// Check dependent plugin load states and handle ordering
	// Dependencies must be FullyLoaded or PartiallyLoaded before we proceed
	for (const FName& DependentPlugin : ModuleInterface->GetDependentPlugins())
	{
		const EGorgeousPluginLoadState DependencyState = GetPluginLoadState(DependentPlugin);
		
		if (DependencyState == EGorgeousPluginLoadState::NotLoaded)
		{
			// Dependency completely failed - don't load this module
			const FString Reason = FString::Printf(TEXT("Dependency '%s' failed to load completely"), *DependentPlugin.ToString());
			RecordModuleLoadFailure(ModuleInterface, Reason);
			return false;
		}
		
		if (DependencyState == EGorgeousPluginLoadState::Unknown)
		{
			// Check if this is a circular dependency that can be resolved
			if (TryResolveCircularDependency(ModuleInterface))
			{
				// Circular dependency resolved - we can proceed despite Unknown state
				GT_I_LOG("GT.Core.PluginHelper", TEXT("[%s] Circular dependency with '%s' resolved - proceeding with module type %s"),
					*PluginName.ToString(),
					*DependentPlugin.ToString(),
					*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
				continue;
			}
			
			// Dependency hasn't finished loading yet - defer this module to ensure correct load order
			GT_I_LOG("GT.Core.PluginHelper", TEXT("[%s] Deferring module type %s - waiting for dependency %s to finish loading"),
				*PluginName.ToString(),
				*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()),
				*DependentPlugin.ToString());
			
			// Decrement expected count since we'll re-add when DefferModuleRegistration increments it
			ExpectedModulesPerPlugin.FindOrAdd(PluginName)--;
			DefferModuleRegistration(ModuleInterface);
			return false;
		}
		
		if (DependencyState == EGorgeousPluginLoadState::PartiallyLoaded)
		{
			// Dependency partially loaded - warn but continue
			GT_W_LOG("GT.Core.PluginHelper", TEXT("[%s] Module type %s depends on partially loaded plugin %s - crashes may occur!"),
				*PluginName.ToString(),
				*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()),
				*DependentPlugin.ToString());
		}
		
		// FullyLoaded - good to go, continue
	}
	
	// All checks passed - register the non-core module
	RegisteredModules.Add(ModuleInterface, ModuleInterface->GetModuleFunctionality());
	LoadedModulesPerPlugin.FindOrAdd(PluginName)++;
	
	GT_I_LOG("GT.Core.PluginHelper", TEXT("[%s] Successfully registered module type: %s"),
	         *PluginName.ToString(),
	         *GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
	
	// Check if all modules are processed and show final notification if needed
	TryShowFinalNotification(PluginName);
	
	return true;
}

void UGorgeousPluginHelper::UnregisterModule(const IGorgeousThingsModuleInterface* ModuleInterface)
{
	if (!IsModuleRegistered(ModuleInterface))
		return;
	
	GT_I_LOG("GT.Core.PluginHelper", TEXT("[%s] Unregistering module type: %s"),
	         *ModuleInterface->GetBelongingPluginName().ToString(),
	         *GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
	
	// Remove from appropriate registry based on module type
	(ModuleInterface->ProvidesCoreFunctionality() ? RegisteredCoreModules : RegisteredModules).Remove(ModuleInterface);
}

bool UGorgeousPluginHelper::IsModuleRegistered(const IGorgeousThingsModuleInterface* ModuleInterface) const
{
	if (!ModuleInterface)
		return false;
	
	const bool bIsInCoreModules = RegisteredCoreModules.Contains(ModuleInterface);
	const bool bIsInModules = RegisteredModules.Contains(ModuleInterface);
	
	return bIsInCoreModules || bIsInModules;
}

bool UGorgeousPluginHelper::SatisfiesMinimumRequiredCoreVersion(const IGorgeousThingsModuleInterface* ModuleInterface) const
{
	const IGorgeousThingsModuleInterface* CoreRuntime = GetCoreModuleInterface(EGorgeousModuleFunctionality::Runtime);
	if (!CoreRuntime)
	{
		// Bootstrap phase: CoreRuntime not yet available, only core modules can proceed
		return ModuleInterface->ProvidesCoreFunctionality();
	}
	const int32 BaseVersion = CoreRuntime->GetMinimumRequiredCoreVersion();
	return ModuleInterface->GetMinimumRequiredCoreVersion() <= BaseVersion;
}

void UGorgeousPluginHelper::DefferModuleRegistration(IGorgeousThingsModuleInterface* ModuleInterface)
{
	if (IsModuleRegistered(ModuleInterface))
		return;
	
	if (ModulesToDefer.Contains(ModuleInterface))
		return;
	
	ModulesToDefer.Add(ModuleInterface);
	
	// Track that this plugin has deferred modules (for circular dependency resolution)
	const FName PluginName = ModuleInterface->GetBelongingPluginName();
	PluginsWithDeferredModules.Add(PluginName);
	
	// Initialize retry count
	DeferredRetryCount.Add(ModuleInterface, 0);
	
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, ModuleInterface](float DeltaTime)
	{
		// Check retry count
		int32& RetryCount = DeferredRetryCount.FindOrAdd(ModuleInterface);
		RetryCount++;
		
		if (RetryCount > MaxDeferredRetries)
		{
			GT_E_LOG("GT.Core.PluginHelper", TEXT("[%s] Maximum deferred retries (%d) exceeded for module type %s - giving up"),
				*ModuleInterface->GetBelongingPluginName().ToString(),
				MaxDeferredRetries,
				*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()));
			
			ModulesToDefer.Remove(ModuleInterface);
			DeferredRetryCount.Remove(ModuleInterface);
			
			// Check if this was the last deferred module for this plugin
			const FName ModulePluginName = ModuleInterface->GetBelongingPluginName();
			bool bHasOtherDeferredModules = false;
			for (const auto* DeferredModule : ModulesToDefer)
			{
				if (DeferredModule->GetBelongingPluginName() == ModulePluginName)
				{
					bHasOtherDeferredModules = true;
					break;
				}
			}
			if (!bHasOtherDeferredModules)
			{
				PluginsWithDeferredModules.Remove(ModulePluginName);
			}
			
			// Record the failure
			FString DependencyList;
			for (const FName& Dep : ModuleInterface->GetDependentPlugins())
			{
				if (!DependencyList.IsEmpty()) DependencyList += TEXT(", ");
				DependencyList += Dep.ToString();
			}
			const FString Reason = FString::Printf(TEXT("Deferred registration timed out waiting for dependencies: %s"), *DependencyList);
			RecordModuleLoadFailure(ModuleInterface, Reason);
			return false; // Stop the ticker
		}
		
		// Check if any dependency has terminally failed - if so, stop trying
		for (const FName& DependentPlugin : ModuleInterface->GetDependentPlugins())
		{
			if (const EGorgeousPluginLoadState DependencyState = GetPluginLoadState(DependentPlugin); 
				DependencyState == EGorgeousPluginLoadState::NotLoaded)
			{
				GT_W_LOG("GT.Core.PluginHelper", TEXT("[%s] Stopping deferred registration of module type %s - dependency %s has failed completely"),
					*ModuleInterface->GetBelongingPluginName().ToString(),
					*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()),
					*DependentPlugin.ToString());
				ModulesToDefer.Remove(ModuleInterface);
				DeferredRetryCount.Remove(ModuleInterface);
				
				// Update deferred plugins tracking
				const FName ModulePluginName = ModuleInterface->GetBelongingPluginName();
				bool bHasOtherDeferredModules = false;
				for (const auto* DeferredModule : ModulesToDefer)
				{
					if (DeferredModule->GetBelongingPluginName() == ModulePluginName)
					{
						bHasOtherDeferredModules = true;
						break;
					}
				}
				if (!bHasOtherDeferredModules)
				{
					PluginsWithDeferredModules.Remove(ModulePluginName);
				}
				
				// Record the failure
				const FString Reason = FString::Printf(TEXT("Dependency '%s' failed to load completely"), *DependentPlugin.ToString());
				RecordModuleLoadFailure(ModuleInterface, Reason);
				return false; // Stop the ticker
			}
		}
		
		GT_I_LOG("GT.Core.PluginHelper", TEXT("[%s] Attempting deferred registration of module type: %s (attempt %d/%d)"),
			*ModuleInterface->GetBelongingPluginName().ToString(),
			*GetModuleTypeString(ModuleInterface->GetModuleFunctionality()),
			RetryCount,
			MaxDeferredRetries);
		
		// Try to register - RegisterModule handles all validation
		if (RegisterModule(ModuleInterface))
		{
			ModulesToDefer.Remove(ModuleInterface);
			DeferredRetryCount.Remove(ModuleInterface);
			
			// Update deferred plugins tracking
			const FName ModulePluginName = ModuleInterface->GetBelongingPluginName();
			bool bHasOtherDeferredModules = false;
			for (const auto* DeferredModule : ModulesToDefer)
			{
				if (DeferredModule->GetBelongingPluginName() == ModulePluginName)
				{
					bHasOtherDeferredModules = true;
					break;
				}
			}
			if (!bHasOtherDeferredModules)
			{
				PluginsWithDeferredModules.Remove(ModulePluginName);
			}
			
			return false; // Success, stop ticker
		}
		
		// Check if we're still deferred (RegisterModule may have removed us on failure)
		if (!ModulesToDefer.Contains(ModuleInterface))
		{
			DeferredRetryCount.Remove(ModuleInterface);
			return false; // Was removed due to failure, stop ticker
		}
		
		return true; // Keep trying
	}), 1.0f);
}

EGorgeousPluginLoadState UGorgeousPluginHelper::GetPluginLoadState(const FName& PluginName) const
{
	// Check cached state first
	if (const EGorgeousPluginLoadState* CachedState = PluginLoadStates.Find(PluginName))
	{
		return *CachedState;
	}
	
	const int32 Expected = ExpectedModulesPerPlugin.FindRef(PluginName);
	const int32 Loaded = LoadedModulesPerPlugin.FindRef(PluginName);
	const int32 Failed = FailedModulesPerPlugin.FindRef(PluginName);
	
	// Plugin hasn't been encountered yet
	if (Expected == 0)
	{
		return EGorgeousPluginLoadState::Unknown;
	}
	
	// All modules failed
	if (Failed == Expected)
	{
		return EGorgeousPluginLoadState::NotLoaded;
	}
	
	// Some modules failed
	if (Failed > 0)
	{
		return EGorgeousPluginLoadState::PartiallyLoaded;
	}
	
	// All modules loaded successfully
	if (Loaded == Expected)
	{
		return EGorgeousPluginLoadState::FullyLoaded;
	}
	
	// Still loading
	return EGorgeousPluginLoadState::Unknown;
}

void UGorgeousPluginHelper::RecordModuleLoadFailure(const IGorgeousThingsModuleInterface* ModuleInterface, const FString& Reason)
{
	const FName PluginName = ModuleInterface->GetBelongingPluginName();
	const EGorgeousModuleFunctionality ModuleType = ModuleInterface->GetModuleFunctionality();
	
	// Abort binary checksum generation since a module failed to load
	bAbortBinaryChecksumSave = true;

	// Track the failure
	FailedModulesPerPlugin.FindOrAdd(PluginName)++;
	FailedModuleTypesPerPlugin.FindOrAdd(PluginName).AddUnique(ModuleType);
	FailureReasonsPerPlugin.FindOrAdd(PluginName).Add(Reason);
	
	GT_E_LOG("GT.Core.PluginHelper", TEXT("[%s] Failed to register module type: %s. Reason: %s"),
		*PluginName.ToString(), *GetModuleTypeString(ModuleType), *Reason);
	
	const int32 Expected = ExpectedModulesPerPlugin.FindRef(PluginName);

	if (const int32 Failed = FailedModulesPerPlugin.FindRef(PluginName); 
		Failed == Expected)
	{
		PluginLoadStates.Add(PluginName, EGorgeousPluginLoadState::NotLoaded);
	}
	else if (Failed > 0)
	{
		PluginLoadStates.Add(PluginName, EGorgeousPluginLoadState::PartiallyLoaded);
	}
	
	// Check if all modules are processed and show final notification
	TryShowFinalNotification(PluginName);
}

void UGorgeousPluginHelper::TryShowFinalNotification(const FName& PluginName)
{
	// Skip if already shown
	if (PluginsWithNotificationShown.Contains(PluginName))
	{
		return;
	}
	
	const int32 Expected = ExpectedModulesPerPlugin.FindRef(PluginName);
	const int32 Loaded = LoadedModulesPerPlugin.FindRef(PluginName);
	const int32 Failed = FailedModulesPerPlugin.FindRef(PluginName);
	
	// Wait until all modules are processed
	if (Loaded + Failed < Expected)
	{
		return;
	}
	
	// All modules processed - update final state
	if (Failed == Expected)
	{
		PluginLoadStates.Add(PluginName, EGorgeousPluginLoadState::NotLoaded);
	}
	else if (Failed > 0)
	{
		PluginLoadStates.Add(PluginName, EGorgeousPluginLoadState::PartiallyLoaded);
	}
	else
	{
		PluginLoadStates.Add(PluginName, EGorgeousPluginLoadState::FullyLoaded);
	}
	
	// Only show notification if there were failures
	if (Failed == 0)
	{
		return;
	}
	
	PluginsWithNotificationShown.Add(PluginName);
	
	// Build failed module types list
	const TArray<EGorgeousModuleFunctionality>& FailedTypes = FailedModuleTypesPerPlugin.FindRef(PluginName);
	FString FailedModulesStr;
	for (int32 i = 0; i < FailedTypes.Num(); ++i)
	{
		if (i > 0) FailedModulesStr += TEXT(", ");
		FailedModulesStr += GetModuleTypeString(FailedTypes[i]);
	}
	
	// Build unique failure reasons list
	const TSet<FString>& Reasons = FailureReasonsPerPlugin.FindRef(PluginName);
	FString ReasonsStr;
	for (const FString& Reason : Reasons)
	{
		if (!ReasonsStr.IsEmpty()) ReasonsStr += TEXT("\n");
		ReasonsStr += TEXT("- ") + Reason;
	}
	
	const bool bIsError = (Failed == Expected);
	GorgeousLogging::ShowToastNotification(
		FString::Printf(TEXT("Gorgeous Plugin %s: %s"), bIsError ? TEXT("Failed") : TEXT("Warning"), *PluginName.ToString()),
		FString::Printf(TEXT("%s modules failed to load: %s\n%s\nReasons:\n%s"),
			bIsError ? TEXT("All") : TEXT("Some"),
			*FailedModulesStr,
			bIsError ? TEXT("Dependent plugins will not function.") : TEXT("Some features may not work."),
			*ReasonsStr),
		bIsError ? 3 : 1
	);
}

bool UGorgeousPluginHelper::IsPluginInstalled(const FName& PluginName) const
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName.ToString());
	return Plugin.IsValid();
}

bool UGorgeousPluginHelper::IsGorgeousPlugin(const FName& PluginName) const
{
	// Check if we've already registered modules from this plugin
	if (KnownGorgeousPlugins.Contains(PluginName))
	{
		return true;
	}
	
	// Check if any registered module belongs to this plugin
	for (const auto& Pair : RegisteredModules)
	{
		if (Pair.Key->GetBelongingPluginName() == PluginName)
		{
			return true;
		}
	}
	
	for (const auto& Pair : RegisteredCoreModules)
	{
		if (Pair.Key->GetBelongingPluginName() == PluginName)
		{
			return true;
		}
	}
	
	// Check plugin descriptor for Gorgeous-style module naming
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName.ToString()); 
		Plugin.IsValid())
	{
		// Heuristic: Gorgeous plugins typically have "Gorgeous" in their name
		if (PluginName.ToString().Contains(TEXT("Gorgeous")))
		{
			return true;
		}
	}
	
	return false;
}

bool UGorgeousPluginHelper::ValidateDependencyChain(const FName& PluginName, TArray<FName>& OutFailedDependencies, TMap<FName, FString>& OutFailureReasons)
{
	// Check cached validation result
	if (const bool* CachedResult = ValidatedDependencies.Find(PluginName))
	{
		if (!*CachedResult)
		{
			OutFailedDependencies.Add(PluginName);
			if (const FString* CachedReason = ValidationFailureReasons.Find(PluginName))
			{
				OutFailureReasons.Add(PluginName, *CachedReason);
			}
		}
		return *CachedResult;
	}
	
	// Detect circular dependencies
	if (PluginsCurrentlyValidating.Contains(PluginName))
	{
		const FString Reason = TEXT("Circular dependency detected");
		OutFailedDependencies.Add(PluginName);
		OutFailureReasons.Add(PluginName, Reason);
		ValidationFailureReasons.Add(PluginName, Reason);
		ValidatedDependencies.Add(PluginName, false);
		GT_E_LOG("GT.Core.PluginHelper", TEXT("Circular dependency detected for plugin: %s"), *PluginName.ToString());
		return false;
	}
	
	if (!IsPluginInstalled(PluginName))
	{
		const FString Reason = TEXT("Plugin not installed");
		OutFailedDependencies.Add(PluginName);
		OutFailureReasons.Add(PluginName, Reason);
		ValidationFailureReasons.Add(PluginName, Reason);
		ValidatedDependencies.Add(PluginName, false);
		GT_E_LOG("GT.Core.PluginHelper", TEXT("Dependency '%s' is not installed"), *PluginName.ToString());
		return false;
	}
	
	// Mark as currently validating (for circular dependency detection)
	PluginsCurrentlyValidating.Add(PluginName);
	
	bool bAllValid = true;
	
	// If it's a Gorgeous plugin that has already loaded, validate its dependency chain recursively
	if (IsGorgeousPlugin(PluginName))
	{
		// Get dependencies from registered modules (only available if plugin has already loaded)

		// If plugin hasn't loaded yet (Unknown state), we can't know its dependencies.
		// The deferred registration system will handle this - when the dependency loads,
		// the waiting module's ticker will re-check and proceed.
		// For now, we just validate that it's installed and enabled.
		
		// Recursively validate each known dependency
		for (TArray<FName> PluginDependencies = GetGorgeousPluginDependencies(PluginName); const FName& Dependency : PluginDependencies)
		{
			if (!ValidateDependencyChain(Dependency, OutFailedDependencies, OutFailureReasons))
			{
				bAllValid = false;
				// Continue checking other dependencies to collect all failures
			}
		}
	}
	else
	{
		// Non-Gorgeous plugin: just verify it's installed and enabled
		if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName.ToString());
			!Plugin.IsValid() || !Plugin->IsEnabled())
		{
			const FString Reason = TEXT("Plugin not enabled");
			OutFailedDependencies.Add(PluginName);
			OutFailureReasons.Add(PluginName, Reason);
			ValidationFailureReasons.Add(PluginName, Reason);
			ValidatedDependencies.Add(PluginName, false);
			PluginsCurrentlyValidating.Remove(PluginName);
			return false;
		}
	}
	
	// Remove from currently validating
	PluginsCurrentlyValidating.Remove(PluginName);
	
	// Cache the result
	ValidatedDependencies.Add(PluginName, bAllValid);
	if (!bAllValid)
	{
		ValidationFailureReasons.Add(PluginName, TEXT("Dependency chain failed"));
	}
	
	return bAllValid;
}

TArray<FName> UGorgeousPluginHelper::GetGorgeousPluginDependencies(const FName& PluginName) const
{
	TSet<FName> UniqueDependencies;
	
	// Check registered modules for this plugin
	for (const auto& Pair : RegisteredModules)
	{
		if (Pair.Key->GetBelongingPluginName() == PluginName)
		{
			for (const FName& Dep : Pair.Key->GetDependentPlugins())
			{
				UniqueDependencies.Add(Dep);
			}
		}
	}
	
	for (const auto& Pair : RegisteredCoreModules)
	{
		if (Pair.Key->GetBelongingPluginName() == PluginName)
		{
			for (const FName& Dep : Pair.Key->GetDependentPlugins())
			{
				UniqueDependencies.Add(Dep);
			}
		}
	}
	
	return UniqueDependencies.Array();
}
TArray<IGorgeousThingsModuleInterface*> UGorgeousPluginHelper::GetAllRegisteredModules() const
{
	TArray<IGorgeousThingsModuleInterface*> Result;
	Result.Reserve(RegisteredCoreModules.Num() + RegisteredModules.Num());

	for (const auto& Pair : RegisteredCoreModules)
	{
		Result.Add(Pair.Key);
	}

	for (const auto& Pair : RegisteredModules)
	{
		Result.Add(Pair.Key);
	}

	return Result;
}

bool UGorgeousPluginHelper::HasPersistentDataFile() const
{
	return FPaths::FileExists(GetGorgeousPersistentDataFilePath());
}

TArray<FGorgeousOfflineSystemCacheEntry> UGorgeousPluginHelper::GetOfflineSystemCache() const
{
	const_cast<UGorgeousPluginHelper*>(this)->LoadPersistentData();

	TArray<FGorgeousOfflineSystemCacheEntry> CacheEntries;
	if (PersistentDataObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* CacheArray;
		if (PersistentDataObject->TryGetArrayField(TEXT("OfflineSystemCache"), CacheArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *CacheArray)
			{
				const TSharedPtr<FJsonObject> Obj = Value->AsObject();
				if (Obj.IsValid())
				{
					FGorgeousOfflineSystemCacheEntry Entry;
					Entry.SystemId = Obj->GetStringField(TEXT("SystemId"));
					Entry.TargetPluginName = Obj->HasField(TEXT("TargetPluginName")) ? Obj->GetStringField(TEXT("TargetPluginName")) : TEXT("GorgeousCore");
					Entry.DisplayName = Obj->GetStringField(TEXT("DisplayName"));
					Entry.Description = Obj->GetStringField(TEXT("Description"));
					Entry.Version = Obj->GetStringField(TEXT("Version"));
					Entry.DownloadUrl = Obj->GetStringField(TEXT("DownloadUrl"));
					Entry.bIsCoreSystem = Obj->GetBoolField(TEXT("bIsCoreSystem"));

					const TArray<TSharedPtr<FJsonValue>>* SourcePathsArray;
					if (Obj->TryGetArrayField(TEXT("SourcePaths"), SourcePathsArray))
					{
						for (const auto& PathVal : *SourcePathsArray)
							Entry.SourcePaths.Add(PathVal->AsString());
					}

					const TArray<TSharedPtr<FJsonValue>>* ContentPathsArray;
					if (Obj->TryGetArrayField(TEXT("ContentPaths"), ContentPathsArray))
					{
						for (const auto& PathVal : *ContentPathsArray)
							Entry.ContentPaths.Add(PathVal->AsString());
					}

					CacheEntries.Add(Entry);
				}
			}
		}
	}
	return CacheEntries;
}

TArray<FGorgeousPluginUpdateCacheEntry> UGorgeousPluginHelper::GetPluginUpdateCache() const
{
	const_cast<UGorgeousPluginHelper*>(this)->LoadPersistentData();

	TArray<FGorgeousPluginUpdateCacheEntry> CacheEntries;
	if (PersistentDataObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* CacheArray;
		if (PersistentDataObject->TryGetArrayField(TEXT("PluginUpdateCache"), CacheArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *CacheArray)
			{
				const TSharedPtr<FJsonObject> Obj = Value->AsObject();
				if (Obj.IsValid())
				{
					FGorgeousPluginUpdateCacheEntry Entry;
					Entry.PluginName = Obj->GetStringField(TEXT("PluginName"));
					Entry.AvailableVersion = Obj->GetStringField(TEXT("AvailableVersion"));
					Entry.MinimumCoreVersion = Obj->GetStringField(TEXT("MinimumCoreVersion"));
					Entry.ChangelogUrl = Obj->GetStringField(TEXT("ChangelogUrl"));
					Entry.DownloadToken = Obj->GetStringField(TEXT("DownloadToken"));
					CacheEntries.Add(Entry);
				}
			}
		}
	}
	return CacheEntries;
}

void UGorgeousPluginHelper::SetOfflineSystemCache(const TArray<FGorgeousOfflineSystemCacheEntry>& NewCache)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FGorgeousOfflineSystemCacheEntry& Entry : NewCache)
	{
		TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
		JsonObj->SetStringField(TEXT("SystemId"), Entry.SystemId);
		JsonObj->SetStringField(TEXT("TargetPluginName"), Entry.TargetPluginName);
		JsonObj->SetStringField(TEXT("DisplayName"), Entry.DisplayName);
		JsonObj->SetStringField(TEXT("Description"), Entry.Description);
		JsonObj->SetStringField(TEXT("Version"), Entry.Version);
		JsonObj->SetStringField(TEXT("DownloadUrl"), Entry.DownloadUrl);
		JsonObj->SetStringField(TEXT("ChangelogUrl"), Entry.ChangelogUrl);
		JsonObj->SetBoolField(TEXT("bIsCoreSystem"), Entry.bIsCoreSystem);

		TArray<TSharedPtr<FJsonValue>> SourcePathsArr;
		for (const FString& Path : Entry.SourcePaths) { SourcePathsArr.Add(MakeShared<FJsonValueString>(Path)); }
		JsonObj->SetArrayField(TEXT("SourcePaths"), SourcePathsArr);

		TArray<TSharedPtr<FJsonValue>> ContentPathsArr;
		for (const FString& Path : Entry.ContentPaths) { ContentPathsArr.Add(MakeShared<FJsonValueString>(Path)); }
		JsonObj->SetArrayField(TEXT("ContentPaths"), ContentPathsArr);

		JsonArray.Add(MakeShared<FJsonValueObject>(JsonObj));
	}

	PersistentDataObject->SetArrayField(TEXT("OfflineSystemCache"), JsonArray);
	SavePersistentData();
}

void UGorgeousPluginHelper::SetPluginUpdateCache(const TArray<FGorgeousPluginUpdateCacheEntry>& NewCache)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FGorgeousPluginUpdateCacheEntry& Entry : NewCache)
	{
		TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
		JsonObj->SetStringField(TEXT("PluginName"), Entry.PluginName);
		JsonObj->SetStringField(TEXT("AvailableVersion"), Entry.AvailableVersion);
		JsonObj->SetStringField(TEXT("MinimumCoreVersion"), Entry.MinimumCoreVersion);
		JsonObj->SetStringField(TEXT("ChangelogUrl"), Entry.ChangelogUrl);
		JsonObj->SetStringField(TEXT("DownloadToken"), Entry.DownloadToken);
		JsonArray.Add(MakeShared<FJsonValueObject>(JsonObj));
	}

	PersistentDataObject->SetArrayField(TEXT("PluginUpdateCache"), JsonArray);
	SavePersistentData();
}

bool UGorgeousPluginHelper::HasPluginUpdateAvailable(const FName& PluginName) const
{
	TArray<FGorgeousPluginUpdateCacheEntry> Updates = GetPluginUpdateCache();
	for (const FGorgeousPluginUpdateCacheEntry& Update : Updates)
	{
		if (Update.PluginName == PluginName.ToString())
		{
			return true;
		}
	}
	return false;
}

FString UGorgeousPluginHelper::GetPluginUpdateChangelogUrl(const FName& PluginName) const
{
	TArray<FGorgeousPluginUpdateCacheEntry> Updates = GetPluginUpdateCache();
	for (const FGorgeousPluginUpdateCacheEntry& Update : Updates)
	{
		if (Update.PluginName == PluginName.ToString())
		{
			return Update.ChangelogUrl;
		}
	}
	return FString();
}

FString UGorgeousPluginHelper::GetPluginUpdateDownloadToken(const FName& PluginName) const
{
	TArray<FGorgeousPluginUpdateCacheEntry> Updates = GetPluginUpdateCache();
	for (const FGorgeousPluginUpdateCacheEntry& Update : Updates)
	{
		if (Update.PluginName == PluginName.ToString())
		{
			return Update.DownloadToken;
		}
	}
	return FString();
}

bool UGorgeousPluginHelper::HasRunInitialValidation() const
{
	const_cast<UGorgeousPluginHelper*>(this)->LoadPersistentData();
	
	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
		if (SystemValidation.IsValid() && SystemValidation->HasField(TEXT("HasRunInitialValidation")))
		{
			return SystemValidation->GetBoolField(TEXT("HasRunInitialValidation"));
		}
	}
	
	return false;
}

void UGorgeousPluginHelper::SetHasRunInitialValidation(bool bHasRun)
{
	LoadPersistentData();
	
	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}
	
	TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}
	
	SystemValidation->SetBoolField(TEXT("HasRunInitialValidation"), bHasRun);
	
	SavePersistentData();
}

int32 UGorgeousPluginHelper::GetSystemValidationCount() const
{
	const_cast<UGorgeousPluginHelper*>(this)->LoadPersistentData();
	
	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
		if (SystemValidation.IsValid())
		{
			return SystemValidation->GetIntegerField(TEXT("ValidationCount"));
		}
	}
	
	return 0;
}

void UGorgeousPluginHelper::IncrementSystemValidationCount()
{
	LoadPersistentData();
	
	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}
	
	TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}
	
	const int32 CurrentCount = SystemValidation->GetIntegerField(TEXT("ValidationCount"));
	SystemValidation->SetNumberField(TEXT("ValidationCount"), CurrentCount + 1);
	
	SavePersistentData();
}

void UGorgeousPluginHelper::SetSystemValidationCount(int32 NewCount)
{
	LoadPersistentData();
	
	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}
	
	TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}
	
	SystemValidation->SetNumberField(TEXT("ValidationCount"), NewCount);
	
	SavePersistentData();
}

int32 UGorgeousPluginHelper::GetSystemValidationInterval() const
{
	const_cast<UGorgeousPluginHelper*>(this)->LoadPersistentData();
	
	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
		if (SystemValidation.IsValid() && SystemValidation->HasField(TEXT("ValidationInterval")))
		{
			return SystemValidation->GetIntegerField(TEXT("ValidationInterval"));
		}
	}
	
	return 10; // Default
}

void UGorgeousPluginHelper::SetSystemValidationInterval(int32 NewInterval)
{
	LoadPersistentData();
	
	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}
	
	TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}
	
	SystemValidation->SetNumberField(TEXT("ValidationInterval"), NewInterval);
	
	SavePersistentData();
}

bool UGorgeousPluginHelper::GetForceDevMode() const
{
	const_cast<UGorgeousPluginHelper*>(this)->LoadPersistentData();
	
	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> DevSettings = PersistentDataObject->GetObjectField(TEXT("DeveloperSettings"));
		if (DevSettings.IsValid() && DevSettings->HasField(TEXT("ForceDevMode")))
		{
			return DevSettings->GetBoolField(TEXT("ForceDevMode"));
		}
	}
	
	return false; // Default
}

void UGorgeousPluginHelper::SetForceDevMode(bool bForce)
{
	LoadPersistentData();
	
	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}
	
	TSharedPtr<FJsonObject> DevSettings = PersistentDataObject->GetObjectField(TEXT("DeveloperSettings"));
	if (!DevSettings.IsValid())
	{
		DevSettings = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("DeveloperSettings"), DevSettings);
	}
	
	DevSettings->SetBoolField(TEXT("ForceDevMode"), bForce);
	
	SavePersistentData();
}

bool UGorgeousPluginHelper::HasCircularDeferredDependency(const FName& PluginA, const FName& PluginB) const
{
	// Check if both plugins have deferred modules
	if (!PluginsWithDeferredModules.Contains(PluginA) || !PluginsWithDeferredModules.Contains(PluginB))
	{
		return false;
	}
	
	// Check if PluginA has deferred modules waiting for PluginB
	bool bAWaitsForB = false;
	for (const auto* DeferredModule : ModulesToDefer)
	{
		if (DeferredModule->GetBelongingPluginName() == PluginA)
		{
			for (const FName& Dep : DeferredModule->GetDependentPlugins())
			{
				if (Dep == PluginB)
				{
					bAWaitsForB = true;
					break;
				}
			}
			if (bAWaitsForB) break;
		}
	}
	
	if (!bAWaitsForB)
	{
		return false;
	}
	
	// Check if PluginB has deferred modules waiting for PluginA
	for (const auto* DeferredModule : ModulesToDefer)
	{
		if (DeferredModule->GetBelongingPluginName() == PluginB)
		{
			for (const FName& Dep : DeferredModule->GetDependentPlugins())
			{
				if (Dep == PluginA)
				{
					return true;
				}
			}
		}
	}
	
	return false;
}

bool UGorgeousPluginHelper::TryResolveCircularDependency(const IGorgeousThingsModuleInterface* ModuleInterface)
{
	const FName PluginName = ModuleInterface->GetBelongingPluginName();
	
	// Check each dependency to see if we have a circular deferred dependency
	for (const FName& DependentPlugin : ModuleInterface->GetDependentPlugins())
	{
		// If the dependency's state is Unknown and both plugins have deferred modules waiting for each other
		if (const EGorgeousPluginLoadState DependencyState = GetPluginLoadState(DependentPlugin); 
			DependencyState != EGorgeousPluginLoadState::Unknown)
		{
			continue; // This dependency has a determined state, no need to resolve
		}
		
		// Check if this is a mutual waiting scenario
		if (HasCircularDeferredDependency(PluginName, DependentPlugin))
		{
			// Handle the circular dependency (log once per pair, show toast if needed)
			HandleCircularDependencyDetected(PluginName, DependentPlugin);
			
			// Allow this module to proceed - the circular dependency will resolve itself
			// as both plugins load their modules simultaneously
			return true;
		}
		
		// Check if we've been retrying long enough that it's likely a circular dependency
		// even if the other plugin hasn't registered deferred modules yet
		if (const int32* RetryCount = DeferredRetryCount.Find(ModuleInterface); RetryCount && *RetryCount >= 3)
		{
			// After 3 retries, if the dependency is still Unknown and is a known Gorgeous plugin
			// with expected modules, it's likely a circular dependency or a stuck state
			if (KnownGorgeousPlugins.Contains(DependentPlugin) || PluginsWithDeferredModules.Contains(DependentPlugin))
			{
				// Handle as a possible circular dependency
				HandleCircularDependencyDetected(PluginName, DependentPlugin);
				return true;
			}
		}
	}
	
	return false;
}

FString UGorgeousPluginHelper::GetCircularDependencyPairKey(const FName& PluginA, const FName& PluginB)
{
	// Sort alphabetically to ensure A-B == B-A
	const FString NameA = PluginA.ToString();
	const FString NameB = PluginB.ToString();
	
	if (NameA < NameB)
	{
		return FString::Printf(TEXT("%s|%s"), *NameA, *NameB);
	}
	return FString::Printf(TEXT("%s|%s"), *NameB, *NameA);
}

FString UGorgeousPluginHelper::GetGorgeousPersistentDataFilePath()
{
	return FPaths::ProjectSavedDir() / TEXT("GorgeousThings") / TEXT("GorgeousPersistentData.json");
}

void UGorgeousPluginHelper::LoadPersistentData()
{
	if (bLoadedPersistentData)
	{
		return;
	}
	
	bLoadedPersistentData = true;
	
	const FString FilePath = GetGorgeousPersistentDataFilePath();
	
	if (!FPaths::FileExists(FilePath))
	{
		PersistentDataObject = MakeShared<FJsonObject>();
		
		// Default SystemValidation values
		TSharedPtr<FJsonObject> SystemValidation = MakeShared<FJsonObject>();
		SystemValidation->SetNumberField(TEXT("ValidationCount"), 0);
		SystemValidation->SetNumberField(TEXT("ValidationInterval"), 10);
		SystemValidation->SetBoolField(TEXT("HasRunInitialValidation"), false);
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
		
		// Default empty circular dependencies
		PersistentDataObject->SetArrayField(TEXT("CircularDependencyNotifications"), TArray<TSharedPtr<FJsonValue>>());
		
		SavePersistentData();
		return;
	}

	FString FileContents;
	if (FFileHelper::LoadFileToString(FileContents, *FilePath))
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);
		if (FJsonSerializer::Deserialize(Reader, PersistentDataObject) && PersistentDataObject.IsValid())
		{
			// Load circular dependencies from JSON
			const TArray<TSharedPtr<FJsonValue>>* PairsArray;
			if (PersistentDataObject->TryGetArrayField(TEXT("CircularDependencyNotifications"), PairsArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *PairsArray)
				{
					NotifiedCircularDependencyPairs.Add(Value->AsString());
				}
			}
			
			GT_I_LOG("GT.Core.PluginHelper", TEXT("Loaded persistent Gorgeous data from disk (%d circular dependency records)"), NotifiedCircularDependencyPairs.Num());
		}
	}
}

bool UGorgeousPluginHelper::bAbortBinaryChecksumSave = false;

void UGorgeousPluginHelper::GenerateAndSaveBinaryChecksum()
{
	if (bAbortBinaryChecksumSave)
	{
		GT_W_LOG("GT.Core.PluginHelper", TEXT("Binary checksum generation aborted due to previous validation errors."));
		return;
	}

	TSharedPtr<IPlugin> CorePlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
	if (!CorePlugin.IsValid())
	{
		return;
	}

	const FString BinariesDir = FPaths::Combine(
		CorePlugin->GetBaseDir(),
		TEXT("Binaries"),
		FPlatformProcess::GetBinariesSubdirectory()
	);

	if (!FPaths::DirectoryExists(BinariesDir))
	{
		return;
	}

	TArray<FString> FoundFiles;
	// Only hash the actual compiled module binaries (.so / .dll).
	// Deliberately exclude: gorgeous-installer, .desktop, .sym, .debug,
	// .modules and any other auxiliary files so the checksum is stable
	// across installer updates, debug info changes, etc.
#if PLATFORM_WINDOWS
	IFileManager::Get().FindFilesRecursive(FoundFiles, *BinariesDir, TEXT("*.dll"), true, false);
#else
	IFileManager::Get().FindFilesRecursive(FoundFiles, *BinariesDir, TEXT("*.so"), true, false);
#endif
	
	// Sort to ensure deterministic hashing order
	FoundFiles.Sort();

	FString CombinedHashes;
	for (const FString& FilePath : FoundFiles)
	{
		FMD5Hash FileHash = FMD5Hash::HashFile(*FilePath);
		CombinedHashes += LexToString(FileHash);
	}

	FMD5 FinalMD5;
	FinalMD5.Update((const uint8*)TCHAR_TO_UTF8(*CombinedHashes), CombinedHashes.Len());
	FMD5Hash FinalHash;
	FinalHash.Set(FinalMD5);
	FString ChecksumString = LexToString(FinalHash);

	GT_I_LOG("GT.Core.PluginHelper", TEXT("Computed GorgeousCore binary checksum: %s"), *ChecksumString);

	UGorgeousPluginHelper* Inst = UGorgeousPluginHelper::GetSingleton();
	if (!Inst) return;

	Inst->LoadPersistentData();

	if (!Inst->PersistentDataObject.IsValid())
	{
		Inst->PersistentDataObject = MakeShared<FJsonObject>();
	}

	const TSharedPtr<FJsonObject>* ChecksumsObjPtr = nullptr;
	TSharedPtr<FJsonObject> ChecksumsObj;
	
	if (Inst->PersistentDataObject->TryGetObjectField(TEXT("VerifiedBinaryChecksums"), ChecksumsObjPtr) && ChecksumsObjPtr && ChecksumsObjPtr->IsValid())
	{
		ChecksumsObj = *ChecksumsObjPtr;
	}
	else
	{
		ChecksumsObj = MakeShared<FJsonObject>();
		Inst->PersistentDataObject->SetObjectField(TEXT("VerifiedBinaryChecksums"), ChecksumsObj);
	}

	ChecksumsObj->SetStringField(TEXT("GorgeousCore"), ChecksumString);

	Inst->SavePersistentData();
	GT_I_LOG("GT.Core.PluginHelper", TEXT("Saved verified binary checksum to persistent data to bypass installer on next boot."));
}

void UGorgeousPluginHelper::InvalidatePersistedChecksum()
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	const TSharedPtr<FJsonObject>* ChecksumsObjPtr = nullptr;
	TSharedPtr<FJsonObject> ChecksumsObj;
	
	if (PersistentDataObject->TryGetObjectField(TEXT("VerifiedBinaryChecksums"), ChecksumsObjPtr) && ChecksumsObjPtr && ChecksumsObjPtr->IsValid())
	{
		ChecksumsObj = *ChecksumsObjPtr;
	}
	else
	{
		ChecksumsObj = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("VerifiedBinaryChecksums"), ChecksumsObj);
	}

	ChecksumsObj->SetStringField(TEXT("GorgeousCore"), TEXT(""));

	const TArray<TSharedPtr<FJsonValue>>* CacheArrayPtr = nullptr;
	if (PersistentDataObject->TryGetArrayField(TEXT("PluginUpdateCache"), CacheArrayPtr) && CacheArrayPtr)
	{
		PersistentDataObject->SetArrayField(TEXT("PluginUpdateCache"), TArray<TSharedPtr<FJsonValue>>());
	}

	SavePersistentData();
	GT_I_LOG("GT.Core.PluginHelper", TEXT("Invalidated persisted binary checksum and plugin update cache."));
}

void UGorgeousPluginHelper::SavePersistentData()
{
	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}
	
	// Ensure circular dependencies are synced
	TArray<TSharedPtr<FJsonValue>> PairsArray;
	for (const FString& PairKey : NotifiedCircularDependencyPairs)
	{
		PairsArray.Add(MakeShared<FJsonValueString>(PairKey));
	}
	PersistentDataObject->SetArrayField(TEXT("CircularDependencyNotifications"), PairsArray);

	const FString FilePath = GetGorgeousPersistentDataFilePath();
	const FString Directory = FPaths::GetPath(FilePath);
	IFileManager::Get().MakeDirectory(*Directory, true);
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (FJsonSerializer::Serialize(PersistentDataObject.ToSharedRef(), Writer))
	{
		if (FFileHelper::SaveStringToFile(OutputString, *FilePath))
		{
			GT_I_LOG("GT.Core.PluginHelper", TEXT("Saved persistent Gorgeous data to disk"));
		}
		else
		{
			GT_W_LOG("GT.Core.PluginHelper", TEXT("Failed to save persistent Gorgeous data to: %s"), *FilePath);
		}
	}
}

void UGorgeousPluginHelper::HandleCircularDependencyDetected(const FName& PluginA, const FName& PluginB)
{
	const FString PairKey = GetCircularDependencyPairKey(PluginA, PluginB);
	
	// Only log once per plugin pair per session
	if (!LoggedCircularDependencyPairs.Contains(PairKey))
	{
		LoggedCircularDependencyPairs.Add(PairKey);
		
		GT_W_LOG("GT.Core.PluginHelper", TEXT("Detected circular dependency between '%s' and '%s' - both plugins depend on each other. Allowing co-loading."),
			*PluginA.ToString(),
			*PluginB.ToString());
	}
	
	// Load previously notified pairs from disk (lazy load)
	LoadPersistentData();
	
	// Only show toast notification if this pair hasn't been notified before
	if (!NotifiedCircularDependencyPairs.Contains(PairKey))
	{
		NotifiedCircularDependencyPairs.Add(PairKey);
		
		// Save to disk so we don't show again
		SavePersistentData();
		
		// Show toast notification
		GorgeousLogging::ShowToastNotification(
			FString::Printf(TEXT("Circular Dependency Detected")),
			FString::Printf(TEXT("Plugins '%s' and '%s' have a circular dependency.\n\n"
				"Circular dependencies can lead to unpredictable behavior if not handled carefully. "
				"Consider restructuring your plugin dependencies to avoid this pattern.\n\n"
				"The system will attempt co-loading, but unexpected issues may occur.\n\n"
				"This notification will not appear again for this plugin combination."),
				*PluginA.ToString(),
				*PluginB.ToString()),
			false // Warning, not error
		);
	}
}

TArray<FString> UGorgeousPluginHelper::GetRecordedInstalledSystems() const
{
	const_cast<UGorgeousPluginHelper*>(this)->LoadPersistentData();

	TArray<FString> Systems;
	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
		if (SystemValidation.IsValid())
		{
			if (SystemValidation->HasField(TEXT("InstalledSystems")))
			{
				TArray<TSharedPtr<FJsonValue>> InstalledSystemsArray = SystemValidation->GetArrayField(TEXT("InstalledSystems"));
				for (const auto& Value : InstalledSystemsArray)
				{
					Systems.Add(Value->AsString());
				}
			}
		}
	}

	return Systems;
}

void UGorgeousPluginHelper::RecordInstalledSystems(const TArray<FString>& Systems)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TSharedPtr<FJsonObject> SystemValidation = PersistentDataObject->GetObjectField(TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}

	TArray<TSharedPtr<FJsonValue>> InstalledSystemsArray;
	for (const FString& System : Systems)
	{
		InstalledSystemsArray.Add(MakeShared<FJsonValueString>(System));
	}

	SystemValidation->SetArrayField(TEXT("InstalledSystems"), InstalledSystemsArray);

	SavePersistentData();
}