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
#include "Helpers/GorgeousPathHelper.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagsManager.h"
#include "GameplayTagsSettings.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
//<-------------------------------------------------------------------------->

#if WITH_EDITOR
/**
 * Adds a gameplay tag to a specific permanent configuration file in the editor.
 *
 * @param TagName        The full tag name (e.g., "Gorgeous.Inventory.Action.Add").
 * @param SourceName     The name of the tag source / ini file (e.g., "GorgeousInventoryTags").
 *                       If empty or "Default", it goes into DefaultGameplayTags.ini.
 *                       A registered source accepts its basename with or without .ini
 *                       and retains its actual file path, including nested tag folders.
 * @param TagComment     Optional comment to describe the tag's purpose.
 * @return True if the tag was added successfully or already exists.
 */
static FORCEINLINE bool AddTagToGorgeousConfig(const FString& TagName, const FString& SourceName = TEXT("Default"), const FString& TagComment = TEXT(""))
{
	if (TagName.IsEmpty())
	{
		return false;
	}

	// 1. Handle Default Source (DefaultGameplayTags.ini)
	if (SourceName.IsEmpty() || SourceName == TEXT("Default"))
	{
		UGameplayTagsSettings* Settings = GetMutableDefault<UGameplayTagsSettings>();
		if (!Settings)
		{
			return false;
		}

		for (const FGameplayTagTableRow& ExistingTag : Settings->GameplayTagList)
		{
			if (ExistingTag.Tag == *TagName)
			{
				return true;
			}
		}

		FGameplayTagTableRow NewTag;
		NewTag.Tag = *TagName;
		NewTag.DevComment = TagComment;
		Settings->GameplayTagList.Add(NewTag);
		Settings->TryUpdateDefaultConfigFile();
		Settings->SaveConfig();

		UGameplayTagsManager::Get().EditorRefreshGameplayTagTree();
		return true;
	}

	// 2. A registered source owns its physical file, regardless of subdirectory.
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FString SourceFilename = FPaths::GetCleanFilename(SourceName);
	if (!SourceFilename.EndsWith(TEXT(".ini"))) SourceFilename += TEXT(".ini");
	const FGameplayTagSource* RegisteredSource = Manager.FindTagSource(FName(*SourceFilename));
	FString IniPath = RegisteredSource ? RegisteredSource->GetConfigFileName() : FString();
	FString TagFolder;
	if (!IniPath.IsEmpty())
	{
		TagFolder = FPaths::GetPath(IniPath);
	}
	else
	{
		// Preserve the existing generic plugin-name fallback for unregistered sources.
		const FString TagFolderRelative = SourceName / TEXT("Config/Tags");
		IniPath = RelativePathToGorgeousPath(TagFolderRelative / (SourceName + TEXT(".ini")));
		TagFolder = RelativePathToGorgeousPath(TagFolderRelative);
	}

	// Ensure the 'Config/Tags' directory exists, otherwise GConfig might fail to create the file.
	if (!IFileManager::Get().DirectoryExists(*TagFolder))
	{
		IFileManager::Get().MakeDirectory(*TagFolder, true);
	}

	// Load the INI and add the tag entry
	// Format:
	// [/Script/GameplayTags.GameplayTagsList]
	// GameplayTagList=(Tag="Gorgeous.Inventory.Action.Add",DevComment="...")

	const FString Section = TEXT("/Script/GameplayTags.GameplayTagsList");
	const FString Key = TEXT("GameplayTagList");
	const FString Entry = FString::Printf(TEXT("(Tag=\"%s\",DevComment=\"%s\")"), *TagName, *TagComment);

	// Newer engines discard loose tag configs from GConfig after registration.
	// Reload the owned file before appending so its existing rows remain intact.
	GConfig->LoadFile(IniPath);
	TArray<FString> ExistingEntries;
	GConfig->GetArray(*Section, *Key, ExistingEntries, IniPath);

	for (const FString& Existing : ExistingEntries)
	{
		if (Existing.Contains(FString::Printf(TEXT("Tag=\"%s\""), *TagName)))
		{
			return true;
		}
	}

	ExistingEntries.Add(Entry);
	GConfig->SetArray(*Section, *Key, ExistingEntries, IniPath);
	GConfig->Flush(false, IniPath);

	// Refresh the manager so it picks up the file change
	UGameplayTagsManager::Get().EditorRefreshGameplayTagTree();

	return true;
}

/**
 * Bulk adds an array of gameplay tags to a specific project configuration file.
 *
 * @param TagNames       Array of tag names to register.
 * @param SourceName     The name of the tag source / ini file.
 * @param CommentPrefix  Prefix for the developer comment.
 */
static FORCEINLINE void AddTagsToGorgeousConfig(const TArray<FString>& TagNames, const FString& SourceName = TEXT("Default"), const FString& CommentPrefix = TEXT("Gorgeous Generated Tag"))
{
	for (const FString& Name : TagNames)
	{
		AddTagToGorgeousConfig(Name, SourceName, CommentPrefix);
	}
}
#endif // WITH_EDITOR
