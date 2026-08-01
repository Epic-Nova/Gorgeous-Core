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
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

/**
 * Gets the Gorgeous plugin base directory.
 * This returns the root folder containing all Gorgeous plugins (e.g., .../Plugins/GorgeousThings/).
 *
 * @return The base directory path of the GorgeousThings plugins folder.
 */
static FORCEINLINE FString GetGorgeousPluginBaseDir()
{
	// We use GorgeousCore as the anchor for the entire ecosystem.
	const TSharedPtr<IPlugin> CorePlugin = IPluginManager::Get().FindPlugin("GorgeousCore");
	if (CorePlugin.IsValid())
	{
		return FPaths::GetPath(CorePlugin->GetBaseDir());
	}

	// Fallback for non-monolithic builds or unusual structures:
	// Find the first plugin that starts with 'Gorgeous'
	TArray<TSharedRef<IPlugin>> AllPlugins = IPluginManager::Get().GetDiscoveredPlugins();
	for (const TSharedRef<IPlugin>& Plugin : AllPlugins)
	{
		if (Plugin->GetName().StartsWith(TEXT("Gorgeous")))
		{
			return FPaths::GetPath(Plugin->GetBaseDir());
		}
	}

	return FPaths::ProjectPluginsDir() / TEXT("GorgeousThings");
}

/**
 * Converts an absolute file path to a gorgeous relative file path.
 *
 * @param FilePath An absolute file path that can be relativated to the gorgeous plugin directory schema.
 * @return The relativated file path that comes from an absolute gorgeous file path.
 */
static FORCEINLINE FString GorgeousPathToRelativePath(FString FilePath)
{
	FString PluginBaseDir = GetGorgeousPluginBaseDir();

	FPaths::NormalizeFilename(FilePath);
	FPaths::NormalizeFilename(PluginBaseDir);

	if (!PluginBaseDir.EndsWith("/"))
	{
		PluginBaseDir += "/";
	}

	if (FilePath.StartsWith(PluginBaseDir))
	{
		return FilePath.RightChop(PluginBaseDir.Len());
	}

	return FilePath;
}

/**
 * Converts a relative file path (within the Gorgeous plugin ecosystem) to an absolute file path.
 * Supports:
 * 1. Unreal package paths (e.g., "/GorgeousInventory/Items/...")
 * 2. Plugin-prefixed paths (e.g., "GorgeousInventory/Content/...")
 * 3. Fallback to ecosystem-relative paths.
 *
 * @param RelativePath A file path relative to a plugin, the ecosystem, or a long package name.
 * @return The absolute file path corresponding to the input path.
 */
static FORCEINLINE FString RelativePathToGorgeousPath(FString RelativePath)
{
	// 1. Handle Unreal Package Paths (starting with '/')
	// This is the most robust way as FPackageName handles all mapping.
	if (RelativePath.StartsWith(TEXT("/")))
	{
		FString PhysicalPath;
		if (FPackageName::TryConvertLongPackageNameToFilename(RelativePath, PhysicalPath))
		{
			FPaths::NormalizeFilename(PhysicalPath);
			return PhysicalPath;
		}
	}

	FPaths::NormalizeFilename(RelativePath);

	// 2. Handle Plugin-Prefixed Paths (e.g., "GorgeousInventory/Config/...")
	// We check if the first part of the path is a registered plugin.
	FString PluginName, RemainingPath;
	if (RelativePath.Split(TEXT("/"), &PluginName, &RemainingPath))
	{
		const TSharedPtr<IPlugin> FoundPlugin = IPluginManager::Get().FindPlugin(PluginName);
		if (FoundPlugin.IsValid())
		{
			return FPaths::Combine(FoundPlugin->GetBaseDir(), RemainingPath);
		}
	}
	else
	{
		// Single-word path: check if it's just a plugin name
		const TSharedPtr<IPlugin> FoundPlugin = IPluginManager::Get().FindPlugin(RelativePath);
		if (FoundPlugin.IsValid())
		{
			return FoundPlugin->GetBaseDir();
		}
	}

	// 3. Fallback: Ecosystem-Relative (Legacy sibling-based logic)
	FString PluginBaseDir = GetGorgeousPluginBaseDir();
	FPaths::NormalizeFilename(PluginBaseDir);

	if (!PluginBaseDir.EndsWith("/"))
	{
		PluginBaseDir += "/";
	}

	return FPaths::Combine(PluginBaseDir, RelativePath);
}