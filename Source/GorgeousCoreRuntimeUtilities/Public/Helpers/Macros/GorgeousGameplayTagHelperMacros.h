// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"

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