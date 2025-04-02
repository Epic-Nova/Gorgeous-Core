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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

/**
 * Converts an absolute file path to a gorgeous relative file path.
 * 
 * @param FilePath An absolute file path that can be relativated to the gorgeous plugin directory schema. 
 * @return The relativated file path that comes from an absolute gorgeous file path e.g. D:/MyUnrealProjects/AwesomeProject/Plugins/GorgeousThings/GorgeousCore/...
 */
static FORCEINLINE FString GorgeousPathToRelativePath(FString FilePath)
{
	FString PluginBaseDir = FPaths::GetPath(IPluginManager::Get().FindPlugin("GorgeousCore")->GetBaseDir());
	
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
	UE_LOG(LogGorgeousCoreRuntimeUtilities, Warning, TEXT("File path '%s' is not within the Gorgeous plugin directory."), *FilePath);
	return FilePath;
}

/**
 * Converts a relative file path (within the Gorgeous plugin) to an absolute file path.
 *
 * @param RelativePath A file path relative to the Gorgeous plugin's base directory,
 * e.g., "Content/MyAsset.uasset".
 * @return The absolute file path corresponding to the relative path within the
 * Gorgeous plugin, e.g.,
 * "D:/MyUnrealProjects/AwesomeProject/Plugins/GorgeousThings/GorgeousCore/Content/MyAsset.uasset".
 */
static FORCEINLINE FString RelativePathToGorgeousPath(FString RelativePath)
{
	FString PluginBaseDir = FPaths::GetPath(IPluginManager::Get().FindPlugin("GorgeousCore")->GetBaseDir());
	
	FPaths::NormalizeFilename(PluginBaseDir);
	
	if (!PluginBaseDir.EndsWith("/"))
	{
		PluginBaseDir += "/";
	}
	
	FPaths::NormalizeFilename(RelativePath);
	
	return FPaths::Combine(PluginBaseDir, RelativePath);
}

/**
 * Returns an enum index from the enum value as the string.
 * 
 * @tparam Enumeration the enum that should be converted
 * @param InValue The string value that should be converted to a enum index
 * @return The string value
 */
template <typename Enumeration>
static FORCEINLINE Enumeration GorgeousStringToEnum(const FString InValue)
{
	return static_cast<Enumeration>(StaticEnum<Enumeration>()->GetValueByNameString(InValue));
}

/**
 * Returns the specified enum value as a string.
 * 
 * @tparam Enumeration the enum that should be converted
 * @param InValue The value of the enum that should be converted
 * @return The enum value as a string
 */
template <typename Enumeration>
static FORCEINLINE FString GorgeousEnumToString(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetNameStringByValue(static_cast<int64>(InValue));
}