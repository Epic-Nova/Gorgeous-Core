// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#include "GorgeousCoreRuntimeModule.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "ObjectVariables/GorgeousObjectVariableCmdletHandler.h"

void FGorgeousCoreRuntimeModule::StartupModule()
{
	//@TODO: Use the Gorgeous Helper functions and do this in every gorgeous plugin; probably move this functionality to the interface defined in the utilities module of the core
	const TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
	check(ThisPlugin.IsValid());

	UGameplayTagsManager::Get().AddTagIniSearchPath(ThisPlugin->GetBaseDir() / TEXT("Config") / TEXT("Tags"));

	UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands();
}

void FGorgeousCoreRuntimeModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("FGorgeousCoreRuntimeModule has shut down!"));
}

IMPLEMENT_MODULE(FGorgeousCoreRuntimeModule, GorgeousCoreRuntime)