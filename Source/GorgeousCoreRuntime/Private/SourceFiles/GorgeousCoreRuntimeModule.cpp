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
#include "GorgeousGuardRequirements.h"
#include "GeneralSystems/GorgeousCompiledSystemGuards.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#endif

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

bool GorgeousCompiledSystemGuards::IsAvailable(FName Guard)
{
#define GORGEOUS_CHECK_GUARD(Name, Available) \
	if (Guard == FName(TEXT(#Name))) return Available;
	GORGEOUS_FOR_EACH_COMPILED_GUARD(GORGEOUS_CHECK_GUARD)
#undef GORGEOUS_CHECK_GUARD
	return false;
}

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/Macros/GorgeousGameplayTagHelperMacros.h"
#include "GeneralSystems/GorgeousCheatCommandAliases.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
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
	GT_REGISTER_TAG_SOURCE("GorgeousCore");
	
#if WITH_EDITOR
	// Shutdown the Gorgeous Startup Hook Module, because it has already done its job
	// StartupHook remains loaded until normal module shutdown (no dangling callbacks).
#endif
	
	GORGEOUS_58_SWITCH(FCoreDelegates::OnPostEngineInit, FCoreDelegates::GetOnPostEngineInit()).AddStatic([]()
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		UGeneralSystemConfiguration_PDA::ActivateAllSystemConfigurations();
	});


	UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands();
	FGorgeousCheatCommandAliases::RegisterConsoleCommands();

	// Register a world-cleanup hook that removes all registry entries belonging to a dying world
	// from the immortal Root OV registries. This MUST fire before UEditorEngine::CheckForWorldGCLeaks
	// to break the strong reference chain (Root → TObjectPtr → OV → Outer → World) that would
	// otherwise cause a "World Memory Leaks: 2 leaked objects" fatal error at editor startup and PIE teardown.
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddLambda([](UWorld* World, bool bSessionEnded, bool /*bCleanupResources*/)
	{
		UGorgeousRootObjectVariable::PurgeWorldOwnedRegistryEntries(World, bSessionEnded);
	});
	
	// Optional developer providers are owned by GorgeousDevelopmentSuiteRuntime.

#if WITH_DEV_AUTOMATION_TESTS
	// Ensure scenarios defined inside this module register with the Insight Matrix once all statics are loaded.
	FGorgeousInsightScenarioRegistrar::ActivateAll();
#endif
}

void FGorgeousCoreRuntimeModule::GorgeousShutdownModule()
{
	GT_W_LOG("GT.Core.Lifecycle", TEXT("FGorgeousCoreRuntimeModule has shut down!"));

	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
	WorldCleanupHandle.Reset();
	// IModularFeature is a non-virtual empty base; recover the allocation before deleting.


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
	return 100; // Version 1.0
}

IMPLEMENT_MODULE(FGorgeousCoreRuntimeModule, GorgeousCoreRuntime)
