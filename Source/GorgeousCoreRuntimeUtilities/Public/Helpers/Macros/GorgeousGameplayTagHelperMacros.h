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
#include "Interfaces/IPluginManager.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "GameplayTagsManager.h"
//<-------------------------------------------------------------------------->

/**
 * Macro to register a plugin-specific gameplay tag search path.
 * This should be called in the StartupModule() of any runtime module that
 * provides its own .ini-based gameplay tags.
 *
 * It expects a 'Config/Tags' folder to exist within the plugin directory.
 *
 * @param PluginName  The name of the plugin (e.g., "GorgeousInventory").
 */
#define GT_REGISTER_TAG_SOURCE(PluginName) \
{ \
	TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT(PluginName)); \
	if (ThisPlugin.IsValid()) \
	{ \
		const FString TagPath = ThisPlugin->GetBaseDir() / TEXT("Config") / TEXT("Tags"); \
		UGameplayTagsManager::Get().AddTagIniSearchPath(TagPath); \
	} \
}