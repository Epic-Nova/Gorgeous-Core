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
#include "GorgeousCoreRuntimeModule.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#endif

#include "InsightMatrix/GorgeousCoreInsightMatrixProvider.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"
#include "GorgeousCoreUtilitiesMinimalShared.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
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
	const TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
	
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
		UE_LOG(LogGorgeousThings, Warning, TEXT("Could not find GorgeousCore plugin via PluginManager, using calculated path: %s"), *PluginBaseDir);
	}

	UGameplayTagsManager::Get().AddTagIniSearchPath(PluginBaseDir / TEXT("Config") / TEXT("Tags"));

	UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands();
	
	InsightProvider = new FGorgeousCoreInsightMatrixProvider();

	// Try immediate registration first; if subsystem not ready, defer to post-engine init
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->RegisterProvider(InsightProvider);
	}
	else
	{
		// Defer registration until engine subsystems are initialized
		FCoreDelegates::OnPostEngineInit.AddLambda([this]()
		{
			if (UGorgeousInsightMatrixSubsystem* Sub = UGorgeousInsightMatrixSubsystem::Get())
			{
				if (InsightProvider)
				{
					Sub->RegisterProvider(InsightProvider);
				}
			}
		});
	}

#if WITH_DEV_AUTOMATION_TESTS
	// Ensure scenarios defined inside this module register with the Insight Matrix once all statics are loaded.
	FGorgeousInsightScenarioRegistrar::ActivateAll();
#endif
}

void FGorgeousCoreRuntimeModule::GorgeousShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("FGorgeousCoreRuntimeModule has shut down!"));

	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		if (InsightProvider)
		{
			Subsystem->UnregisterProvider(InsightProvider);
		}
	}
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
