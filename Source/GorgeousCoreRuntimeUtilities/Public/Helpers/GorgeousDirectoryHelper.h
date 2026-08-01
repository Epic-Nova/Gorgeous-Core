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

/**
 * Checks if a given directory exists inside the gorgeous folder structure.
 * 
 * @param DirectoryPath The relatative path that should be checked
 * @return True if the directory exists, false otherwise
 */
static FORCEINLINE bool IsGorgeousDirectoryPresent(const FString& DirectoryPath)
{
	return IFileManager::Get().DirectoryExists(*RelativePathToGorgeousPath(DirectoryPath));
}

/**
 * Creates a gorgeous directory under the specified relative file path.
 * 
 * @param DirectoryPath The directory that should be created. 
 * @param bRecursive     Whether to create parent directories if they don't exist.
 * @return True if the directory was created, false otherwise.
 */
static FORCEINLINE bool MakeGorgeousDirectory(const FString& DirectoryPath, bool bRecursive = true)
{
	return IFileManager::Get().MakeDirectory(*RelativePathToGorgeousPath(DirectoryPath), bRecursive);
}

/**
 * Deletes a gorgeous directory under the specified relative file path.
 * 
 * @param DirectoryPath The directory that should be deleted.
 * @return True if the directory was deleted, false otherwise.
 */
static FORCEINLINE bool DeleteGorgeousDirectory(const FString& DirectoryPath)
{
	if(IsGorgeousDirectoryPresent(DirectoryPath))
	{
		return IFileManager::Get().DeleteDirectory(*RelativePathToGorgeousPath(DirectoryPath), true);
	}
	return false;
}