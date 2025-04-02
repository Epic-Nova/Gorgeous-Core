// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#include "GorgeousCoreRuntimeModule.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariableCmdletHandler.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreRuntimeModule Implementation
//=============================================================================

void FGorgeousCoreRuntimeModule::StartupModule()
{
	//@TODO: Use the Gorgeous Helper functions and do this in every gorgeous plugin.
	const TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
	check(ThisPlugin.IsValid());

	UGameplayTagsManager::Get().AddTagIniSearchPath(ThisPlugin->GetBaseDir() / TEXT("Config") / TEXT("Tags"));

	UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands();
}

void FGorgeousCoreRuntimeModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("FGorgeousCoreRuntimeModule has shut down!"));
}

TArray<FName> FGorgeousCoreRuntimeModule::GetDependentPlugins()
{
	return TArray<FName>();
}

IMPLEMENT_MODULE(FGorgeousCoreRuntimeModule, GorgeousCoreRuntime)
