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
#pragma once

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

#include "GorgeousConversationHelper.h"
#include "GorgeousDirectoryHelper.h"

/*! @param FilePath The Gorgeous Path to the file e.g. Source/GorgeousExample/MySuperCoolFile.txt */
static FORCEINLINE bool IsGorgeousFilePresent(FString FilePath)
{
	return IFileManager::Get().FileExists(*GorgeousPathToRelativePath(FilePath));
}

static FORCEINLINE bool IsGorgeousFileReadOnly(FString FilePath)
{
	return IFileManager::Get().IsReadOnly(*GorgeousPathToRelativePath(FilePath));
}

static FORCEINLINE FDateTime GetLastGorgeousModification(FString FilePath)
{
	return IFileManager::Get().GetTimeStamp(*GorgeousPathToRelativePath(FilePath));
}

static FORCEINLINE bool WriteGorgeousFile(FString FilePath, FString Content = "", bool bOverrideExisting = false)
{
	if(!bOverrideExisting && IsGorgeousFilePresent(FilePath)) { return false; }
	
	return FFileHelper::SaveStringToFile(FStringView(Content), *GorgeousPathToRelativePath(FilePath), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), 0);
}

static FORCEINLINE bool ReadGorgeousFile(FString FilePath, FString& Result, bool bVerifyHash = true)
{
	if(IsGorgeousFilePresent(FilePath))
	{
		return FFileHelper::LoadFileToString(Result, *IFileManager::Get().CreateFileReader(*GorgeousPathToRelativePath(FilePath)), bVerifyHash ? FFileHelper::EHashOptions::EnableVerify : FFileHelper::EHashOptions::None);
	}
	return false;
}

static FORCEINLINE bool DeleteGorgeousFile(FString FilePath, bool bIgnoreReadOnly = false)
{
	if(IsGorgeousFilePresent(FilePath))
	{
		return IFileManager::Get().Delete(*GorgeousPathToRelativePath(FilePath), true, bIgnoreReadOnly);
	}
	return false;
}

static FORCEINLINE void CopyGorgeousFile(FString FilePath, FString CopyPath, bool bReplaceIfAlreadyExist, bool bIgnoreReadOnly = false)
{
	if(IsGorgeousFilePresent(FilePath))
	{
		IFileManager::Get().Copy(*GorgeousPathToRelativePath(FilePath), *GorgeousPathToRelativePath(CopyPath), bReplaceIfAlreadyExist, bIgnoreReadOnly);
	}
}

static FORCEINLINE bool MoveOrRenameGorgeousFile(FString FilePath, FString NewFilePath, bool bReplaceIfAlreadyExist, bool bIgnoreReadOnly = false)
{
	if(IsGorgeousFilePresent(FilePath))
	{
		return IFileManager::Get().Move(*GorgeousPathToRelativePath(FilePath), *GorgeousPathToRelativePath(NewFilePath), bReplaceIfAlreadyExist, bIgnoreReadOnly);
	}
	return false;
}

static FORCEINLINE bool ExistInGorgeousFile(FString FilePath, FString Content)
{
	FString LoadedFileString;
	return IsGorgeousFilePresent(FilePath) && ReadGorgeousFile(FilePath, LoadedFileString) && LoadedFileString.Contains(Content);
}

static FORCEINLINE bool InsertInGorgeousFile(FString FilePath, FString Content)
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

static FORCEINLINE bool ReplaceInGorgeousFile(FString FilePath, FString Content, FString NewContent)
{
	if(ExistInGorgeousFile(FilePath, Content))
	{
		FString LoadedFileString;
		ReadGorgeousFile(FilePath, LoadedFileString);
		
		WriteGorgeousFile(FilePath, LoadedFileString.Replace(*Content, *NewContent), true);
	}
	return false;
}

static FORCEINLINE bool DeleteInGorgeousFile(FString FilePath, FString Content)
{
	if(ExistInGorgeousFile(FilePath, Content))
	{
		FString LoadedFileString;
		ReadGorgeousFile(FilePath, LoadedFileString);
		
		WriteGorgeousFile(FilePath, LoadedFileString.Replace(*Content, TEXT("")), true);
	}
	return false;
}

static FORCEINLINE TArray<FString> FindGorgeousFiles(FString Directory)
{
	TArray<FString> FoundFiles;
	if(IsGorgeousDirectoryPresent(Directory))
	{
		IFileManager::Get().FindFilesRecursive(FoundFiles, *GorgeousPathToRelativePath(Directory), TEXT("*"), true, false);
	}
	return FoundFiles;
}

static FORCEINLINE int64 GetGorgeousFileSize(FString FilePath)
{
	if(IsGorgeousFilePresent(FilePath))
	{
		return IFileManager::Get().FileSize(*GorgeousPathToRelativePath(FilePath));
	}
	return 0;
}

static FORCEINLINE TArray<FString> GetGorgeousFilesInDirectory(const FString& Directory)
{
	TArray<FString> FoundFiles;
	if(IsGorgeousDirectoryPresent(Directory))
	{
		IFileManager::Get().FindFilesRecursive(FoundFiles, *GorgeousPathToRelativePath(Directory), TEXT("*"), true, false);
	}
	return FoundFiles;
}