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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

#include <HAL/FileManager.h>
#include "GorgeousConversationHelper.h"

static FORCEINLINE bool IsGorgeousDirectoryPresent(FString DirectoryPath)
{
	return IFileManager::Get().DirectoryExists(*GorgeousPathToRelativePath(DirectoryPath));
}

static FORCEINLINE bool MakeGorgeousDirectory(FString DirectoryPath)
{
	return IFileManager::Get().MakeDirectory(*GorgeousPathToRelativePath(DirectoryPath));
}

static FORCEINLINE bool DeleteGorgeousDirectory(FString DirectoryPath)
{
	if(IsGorgeousDirectoryPresent(DirectoryPath))
	{
		return IFileManager::Get().DeleteDirectory(*GorgeousPathToRelativePath(DirectoryPath), true);
	}
	return false;
}