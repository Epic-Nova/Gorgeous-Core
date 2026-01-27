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
#include "Misc/CoreDelegates.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariableCmdletHandler.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreRuntimeModule Implementation
//=============================================================================

void FGorgeousCoreRuntimeModule::GorgeousStartupModule()
{
	//@TODO: Use the Gorgeous Helper functions and do this in every gorgeous plugin.
	const TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
	check(ThisPlugin.IsValid());

	UGameplayTagsManager::Get().AddTagIniSearchPath(ThisPlugin->GetBaseDir() / TEXT("Config") / TEXT("Tags"));

	UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands();

	InsightProvider = MakeUnique<FGorgeousCoreInsightMatrixProvider>();
	SetInsightProvider(InsightProvider.Get());

	// Try immediate registration first; if subsystem not ready, defer to post-engine init
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->RegisterProvider(InsightProvider.Get());
	}
	else
	{
		// Defer registration until engine subsystems are initialized
		FCoreDelegates::OnPostEngineInit.AddLambda([this]()
		{
			if (InsightProvider && UGorgeousInsightMatrixSubsystem::Get())
			{
				UGorgeousInsightMatrixSubsystem::Get()->RegisterProvider(InsightProvider.Get());
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
			Subsystem->UnregisterProvider(InsightProvider.Get());
		}
	}
	SetInsightProvider(nullptr);
	InsightProvider.Reset();

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
	return 90; // Version 0.9
}

IMPLEMENT_MODULE(FGorgeousCoreRuntimeModule, GorgeousCoreRuntime)
