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

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include <HAL/FileManager.h>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousDirectoryHelper.h"
//<-------------------------------------------------------------------------->

/**
 * Checks if a file exists within the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file to check.
 * @return True if the file exists, false otherwise.
 */
static FORCEINLINE bool IsGorgeousFilePresent(const FString& FilePath)
{
    return IFileManager::Get().FileExists(*RelativePathToGorgeousPath(FilePath));
}

/**
 * Checks if a file within the Gorgeous file structure is read-only.
 *
 * @param FilePath The relative path to the file to check.
 * @return True if the file is read-only, false otherwise.
 */
static FORCEINLINE bool IsGorgeousFileReadOnly(const FString& FilePath)
{
    return IFileManager::Get().IsReadOnly(*RelativePathToGorgeousPath(FilePath));
}

/**
 * Gets the last modification timestamp of a file within the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file.
 * @return The last modification timestamp as an FDateTime object.
 */
static FORCEINLINE FDateTime GetLastGorgeousModification(const FString& FilePath)
{
    return IFileManager::Get().GetTimeStamp(*RelativePathToGorgeousPath(FilePath));
}

/**
 * Writes content to a file within the Gorgeous file structure.
 * Creates the file if it doesn't exist, and optionally overrides existing content.
 *
 * @param FilePath The relative path to the file to write to.
 * @param Content The string content to write to the file (defaults to empty).
 * @param bOverrideExisting If true, overwrites the file if it already exists. If false, the operation fails if the file exists.
 * @return True if the write operation was successful, false otherwise.
 */
static FORCEINLINE bool WriteGorgeousFile(const FString& FilePath, const FString& Content = "", const bool bOverrideExisting = false)
{
    if(!bOverrideExisting && IsGorgeousFilePresent(FilePath)) { return false; }

    return FFileHelper::SaveStringToFile(FStringView(Content), *RelativePathToGorgeousPath(FilePath), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), 0);
}

/**
 * Reads the content of a file within the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file to read.
 * @param Result A reference to the FString where the file content will be stored.
 * @param bVerifyHash If true, verifies the file's hash after loading.
 * @return True if the file was read successfully, false otherwise.
 */
static FORCEINLINE bool ReadGorgeousFile(const FString& FilePath, FString& Result, const bool bVerifyHash = true)
{
    if(IsGorgeousFilePresent(FilePath))
    {
       return FFileHelper::LoadFileToString(Result, *IFileManager::Get().CreateFileReader(*RelativePathToGorgeousPath(FilePath)), bVerifyHash ? FFileHelper::EHashOptions::EnableVerify : FFileHelper::EHashOptions::None);
    }
    return false;
}

/**
 * Deletes a file within the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file to delete.
 * @param bIgnoreReadOnly If true, attempts to delete the file even if it's marked as read-only.
 * @return True if the file was deleted successfully, false otherwise.
 */
static FORCEINLINE bool DeleteGorgeousFile(const FString& FilePath, const bool bIgnoreReadOnly = false)
{
    if(IsGorgeousFilePresent(FilePath))
    {
       return IFileManager::Get().Delete(*RelativePathToGorgeousPath(FilePath), true, bIgnoreReadOnly);
    }
    return false;
}

/**
 * Copies a file within the Gorgeous file structure to a new location within the Gorgeous structure.
 *
 * @param FilePath The relative path to the source file.
 * @param CopyPath The relative path to the destination for the copy.
 * @param bReplaceIfAlreadyExist If true, overwrites the destination file if it already exists.
 * @param bIgnoreReadOnly If true, attempts to copy even if the source file is read-only.
 */
static FORCEINLINE void CopyGorgeousFile(const FString& FilePath, const FString& CopyPath, const bool bReplaceIfAlreadyExist, const bool bIgnoreReadOnly = false)
{
    if(IsGorgeousFilePresent(FilePath))
    {
       IFileManager::Get().Copy(*RelativePathToGorgeousPath(FilePath), *RelativePathToGorgeousPath(CopyPath), bReplaceIfAlreadyExist, bIgnoreReadOnly);
    }
}

/**
 * Moves or renames a file within the Gorgeous file structure.
 *
 * @param FilePath The relative path to the current file.
 * @param NewFilePath The relative path to the new location or name for the file.
 * @param bReplaceIfAlreadyExist If true, overwrites the destination file if it already exists.
 * @param bIgnoreReadOnly If true, attempts to move even if the source file is read-only.
 * @return True if the move or rename operation was successful, false otherwise.
 */
static FORCEINLINE bool MoveOrRenameGorgeousFile(const FString& FilePath, const FString& NewFilePath, bool bReplaceIfAlreadyExist, const bool bIgnoreReadOnly = false)
{
    if(IsGorgeousFilePresent(FilePath))
    {
       return IFileManager::Get().Move(*RelativePathToGorgeousPath(FilePath), *RelativePathToGorgeousPath(NewFilePath), bReplaceIfAlreadyExist, bIgnoreReadOnly);
    }
    return false;
}

/**
 * Checks if a specific string exists within a file in the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file to search within.
 * @param Content The string to search for.
 * @return True if the string is found in the file, false otherwise.
 */
static FORCEINLINE bool ExistInGorgeousFile(const FString& FilePath, const FString& Content)
{
    FString LoadedFileString;
    return IsGorgeousFilePresent(FilePath) && ReadGorgeousFile(FilePath, LoadedFileString) && LoadedFileString.Contains(Content);
}

/**
 * Inserts content at the end of a file within the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file to insert into.
 * @param Content The string content to insert.
 * @return True if the insertion was successful, false otherwise.
 */
static FORCEINLINE bool InsertInGorgeousFile(const FString& FilePath, FString Content)
{
    if(IsGorgeousFilePresent(FilePath))
    {
       FString LoadedFileString;
       ReadGorgeousFile(FilePath, LoadedFileString);

       LoadedFileString.Append(Content);

       WriteGorgeousFile(FilePath, LoadedFileString, true);
    }
    return false;
}

/**
 * Replaces all occurrences of a specific string within a file in the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file to modify.
 * @param Content The string to be replaced.
 * @param NewContent The string to replace the old content with.
 * @return True if the replacement was successful, false otherwise.
 */
static FORCEINLINE bool ReplaceInGorgeousFile(const FString& FilePath, const FString& Content, const FString& NewContent)
{
    if(ExistInGorgeousFile(FilePath, Content))
    {
       FString LoadedFileString;
       ReadGorgeousFile(FilePath, LoadedFileString);

       WriteGorgeousFile(FilePath, LoadedFileString.Replace(*Content, *NewContent), true);
    }
    return false;
}

/**
 * Deletes all occurrences of a specific string within a file in the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file to modify.
 * @param Content The string to be deleted.
 * @return True if the deletion was successful, false otherwise.
 */
static FORCEINLINE bool DeleteInGorgeousFile(const FString& FilePath, const FString& Content)
{
    if(ExistInGorgeousFile(FilePath, Content))
    {
       FString LoadedFileString;
       ReadGorgeousFile(FilePath, LoadedFileString);

       WriteGorgeousFile(FilePath, LoadedFileString.Replace(*Content, TEXT("")), true);
    }
    return false;
}

/**
 * Finds all files within a specific directory in the Gorgeous file structure.
 *
 * @param Directory The relative path to the directory to search within.
 * @return An array of FStrings, where each string is the full relative path to a found file.
 */
static FORCEINLINE TArray<FString> FindGorgeousFiles(const FString& Directory)
{
    TArray<FString> FoundFiles;
    if(IsGorgeousDirectoryPresent(Directory))
    {
       IFileManager::Get().FindFilesRecursive(FoundFiles, *RelativePathToGorgeousPath(Directory), TEXT("*"), true, false);
    }
    return FoundFiles;
}

/**
 * Gets the size of a file within the Gorgeous file structure.
 *
 * @param FilePath The relative path to the file.
 * @return The size of the file in bytes, or 0 if the file does not exist.
 */
static FORCEINLINE int64 GetGorgeousFileSize(const FString& FilePath)
{
    if(IsGorgeousFilePresent(FilePath))
    {
       return IFileManager::Get().FileSize(*RelativePathToGorgeousPath(FilePath));
    }
    return 0;
}

/**
 * Gets all files within a specific directory in the Gorgeous file structure.
 * This is an alias for FindGorgeousFiles.
 *
 * @param Directory The relative path to the directory to search within.
 * @return An array of FStrings, where each string is the full relative path to a found file.
 */
static FORCEINLINE TArray<FString> GetGorgeousFilesInDirectory(const FString& Directory)
{
    TArray<FString> FoundFiles;
    if(IsGorgeousDirectoryPresent(Directory))
    {
       IFileManager::Get().FindFilesRecursive(FoundFiles, *RelativePathToGorgeousPath(Directory), TEXT("*"), true, false);
    }
    return FoundFiles;
}