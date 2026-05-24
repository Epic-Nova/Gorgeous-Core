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
#include "DataSchemaMapping/GorgeousDataSchemaMappingBatchCommandlet.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorService.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Parse.h"
#include "UObject/SoftObjectPath.h"
//<-------------------------------------------------------------------------->

UGorgeousDataSchemaMappingBatchCommandlet::UGorgeousDataSchemaMappingBatchCommandlet()
{
	LogToConsole = true;
	IsEditor = true;
	IsServer = false;
	ShowErrorCount = true;
	HelpDescription = TEXT("Batch-executes a Data Schema Mapping asset over selected source assets.");
	HelpUsage = TEXT("-Schema=<ObjectPath> [-Sources=<PathA,PathB,...>] [-SourceFolder=<FolderPathA,FolderPathB,...>] [-DryRun]");
}

int32 UGorgeousDataSchemaMappingBatchCommandlet::Main(const FString& Params)
{
	FString SchemaPath;
	if (!FParse::Value(*Params, TEXT("Schema="), SchemaPath) || SchemaPath.IsEmpty())
	{
		GT_E_LOG("GT.Core.Commandlet", TEXT("Missing required -Schema argument."));
		GT_I_LOG("GT.Core.Commandlet", TEXT("Usage: %s"), *HelpUsage);
		return 1;
	}

	const bool bDryRun = FParse::Param(*Params, TEXT("DryRun")) || FParse::Param(*Params, TEXT("Preview"));

	UGorgeousDataSchemaMapping_DA* SchemaMap = LoadObject<UGorgeousDataSchemaMapping_DA>(nullptr, *SchemaPath);
	if (!SchemaMap)
	{
		GT_E_LOG("GT.Core.Commandlet", TEXT("Failed to load schema map at '%s'."), *SchemaPath);
		return 2;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	TArray<FAssetData> SelectedAssets;
	TSet<FSoftObjectPath> UniqueAssetPaths;

	FString SourcesArg;
	if (FParse::Value(*Params, TEXT("Sources="), SourcesArg) && !SourcesArg.IsEmpty())
	{
		TArray<FString> SourcePaths;
		ParseListArgument(SourcesArg, SourcePaths);
		for (const FString& SourcePath : SourcePaths)
		{
			const FSoftObjectPath SoftObjectPath(SourcePath);
			if (!SoftObjectPath.IsValid())
			{
				GT_W_LOG("GT.Core.Commandlet", TEXT("Ignoring invalid source object path '%s'."), *SourcePath);
				continue;
			}

			if (UniqueAssetPaths.Contains(SoftObjectPath))
			{
				continue;
			}

			FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(SoftObjectPath);
			if (!AssetData.IsValid())
			{
				if (UObject* LoadedObject = SoftObjectPath.TryLoad())
				{
					AssetData = FAssetData(LoadedObject);
				}
			}

			if (!AssetData.IsValid())
			{
				GT_W_LOG("GT.Core.Commandlet", TEXT("Failed to resolve source object path '%s'."), *SourcePath);
				continue;
			}

			UniqueAssetPaths.Add(SoftObjectPath);
			SelectedAssets.Add(MoveTemp(AssetData));
		}
	}

	FString SourceFoldersArg;
	if (FParse::Value(*Params, TEXT("SourceFolder="), SourceFoldersArg) && !SourceFoldersArg.IsEmpty())
	{
		TArray<FString> SourceFolders;
		ParseListArgument(SourceFoldersArg, SourceFolders);
		for (const FString& SourceFolder : SourceFolders)
		{
			FARFilter FolderFilter;
			FolderFilter.PackagePaths.Add(*SourceFolder);
			FolderFilter.bRecursivePaths = true;

			TArray<FAssetData> FolderAssets;
			AssetRegistry.GetAssets(FolderFilter, FolderAssets);
			for (const FAssetData& AssetData : FolderAssets)
			{
				const FSoftObjectPath ObjectPath = AssetData.GetSoftObjectPath();
				if (!ObjectPath.IsValid() || UniqueAssetPaths.Contains(ObjectPath))
				{
					continue;
				}

				UniqueAssetPaths.Add(ObjectPath);
				SelectedAssets.Add(AssetData);
			}
		}
	}

	if (SelectedAssets.IsEmpty())
	{
		GT_E_LOG("GT.Core.Commandlet", TEXT("No source assets were resolved. Provide -Sources and/or -SourceFolder."));
		return 3;
	}

	GT_I_LOG(
		"GT.Core.Commandlet",
		TEXT("Running Data Schema Mapping batch (%s). Schema='%s'. SourceCount=%d"),
		bDryRun ? TEXT("DryRun") : TEXT("Execute"),
		*SchemaMap->GetPathName(),
		SelectedAssets.Num());

	if (bDryRun)
	{
		FGorgeousDataSchemaMappingEditorService::PreviewMigration(SchemaMap, SelectedAssets);
	}
	else
	{
		FGorgeousDataSchemaMappingEditorService::ExecuteMigration(SchemaMap, SelectedAssets);
	}

	return 0;
}

void UGorgeousDataSchemaMappingBatchCommandlet::ParseListArgument(const FString& RawList, TArray<FString>& OutValues)
{
	OutValues.Reset();
	FString NormalizedList = RawList;
	NormalizedList.ReplaceInline(TEXT(";"), TEXT(","));

	TArray<FString> SplitValues;
	NormalizedList.ParseIntoArray(SplitValues, TEXT(","), true);
	for (FString& Value : SplitValues)
	{
		Value = Value.TrimStartAndEnd();
		if (!Value.IsEmpty())
		{
			OutValues.Add(MoveTemp(Value));
		}
	}
}
