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
#include "GorgeousCoreRuntimeModule.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#endif

#include "InsightMatrix/GorgeousCoreInsightMatrixProvider.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "Interfaces/IPluginManager.h"
#include "CoreMinimal.h"

// Set to 1 to enable experimental memory leak fixes for PIE teardown and replication crashes.
#ifndef GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
#define GORGEOUS_EXPERIMENTAL_MEMORY_FIXES 0
#endif

#include "GameplayTagsManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"
#include "GorgeousCoreUtilitiesMinimalShared.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "GeneralSystems/GorgeousCheatCommandAliases.h"
#include "ObjectVariables/GorgeousObjectVariableCmdletHandler.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "Engine/World.h"
//<-------------------------------------------------------------------------->
//=============================================================================
// FGorgeousCoreRuntimeModule Implementation
//=============================================================================



void FGorgeousCoreRuntimeModule::GorgeousStartupModule()
{


	//@TODO: Use the Gorgeous Helper functions and do this in every gorgeous plugin.
	/*const TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
	
	// During module loading, the plugin may not yet be fully registered in the Plugin Manager.
	// If FindPlugin fails, we calculate the path relative to this module's location.
	FString PluginBaseDir;
	if (ThisPlugin.IsValid())
	{
		PluginBaseDir = ThisPlugin->GetBaseDir();
	}
	else
	{
		// Fallback: Calculate plugin directory from module DLL location
		// The module DLL is in <PluginDir>/Binaries/<Platform>/, so go up 3 levels
		const FString ModulePath = FModuleManager::Get().GetModuleFilename(TEXT("GorgeousCoreRuntime"));
		PluginBaseDir = FPaths::GetPath(FPaths::GetPath(FPaths::GetPath(ModulePath)));
		GT_W_LOG("GT.Core.Lifecycle", TEXT("Could not find GorgeousCore plugin via PluginManager, using calculated path: %s"), *PluginBaseDir);
	}

	UGameplayTagsManager::Get().AddTagIniSearchPath(PluginBaseDir / TEXT("Config") / TEXT("Tags"));*/
	
	GT_REGISTER_TAG_SOURCE("GorgeousCore")

	UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands();
	FGorgeousCheatCommandAliases::RegisterConsoleCommands();

	// Register a world-cleanup hook that removes all registry entries belonging to a dying world
	// from the immortal Root OV registries. This MUST fire before UEditorEngine::CheckForWorldGCLeaks
	// to break the strong reference chain (Root → TObjectPtr → OV → Outer → World) that would
	// otherwise cause a "World Memory Leaks: 2 leaked objects" fatal error at editor startup and PIE teardown.
#if GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
	FWorldDelegates::OnWorldCleanup.AddLambda([](UWorld* World, bool bSessionEnded, bool /*bCleanupResources*/)
	{
		UGorgeousRootObjectVariable::PurgeWorldOwnedRegistryEntries(World, bSessionEnded);
	});
#endif
	
	InsightProvider = new FGorgeousCoreInsightMatrixProvider();

#if WITH_DEV_AUTOMATION_TESTS
	// Ensure scenarios defined inside this module register with the Insight Matrix once all statics are loaded.
	FGorgeousInsightScenarioRegistrar::ActivateAll();
#endif
}

void FGorgeousCoreRuntimeModule::GorgeousShutdownModule()
{
	GT_W_LOG("GT.Core.Lifecycle", TEXT("FGorgeousCoreRuntimeModule has shut down!"));

	delete InsightProvider;
	InsightProvider = nullptr;

#if WITH_DEV_AUTOMATION_TESTS
	FGorgeousInsightScenarioRegistrar::DeactivateAll();
#endif
}

TArray<FName> FGorgeousCoreRuntimeModule::GetDependentPlugins() const
{
	return TArray<FName>();
}

int32 FGorgeousCoreRuntimeModule::GetMinimumRequiredCoreVersion() const
{
	//Actually not needed as the Core does not perform checks against itself.
	return 100; // Version 0.9
}

IMPLEMENT_MODULE(FGorgeousCoreRuntimeModule, GorgeousCoreRuntime)
