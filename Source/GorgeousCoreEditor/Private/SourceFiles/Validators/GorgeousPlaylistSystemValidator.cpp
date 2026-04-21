// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Events - Events functionality provider              |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "Validators/GorgeousPlaylistSystemValidator.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "DataRegistry.h"
#include "DataRegistrySettings.h"
#include "UnrealEdMisc.h"
#include "Engine/AssetManager.h"
#include "Engine/Blueprint.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousPlaylistSystemValidator Implementation
//=============================================================================

UGorgeousPlaylistSystemValidator::UGorgeousPlaylistSystemValidator()
{
	UGT_EditorLogging_FL::RegisterLogHyperlinkAction(
	StaticClass(),
	FName("GT.Systems.PlaylistSystem.Validator.RegisterDirectory"),
	FName("HandleRegisterDirectoryHyperlink"));
}

UGorgeousPlaylistSystemValidator::~UGorgeousPlaylistSystemValidator()
{
	UGT_EditorLogging_FL::UnregisterLogHyperlinkAction("GT.Systems.PlaylistSystem.Validator.RegisterDirectory");
}

bool UGorgeousPlaylistSystemValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (InAssetData.PackageName.ToString().Contains(TEXT("PlaylistObject")) && InObject->IsA<UBlueprint>())
	{
		bIndirectValidationFlag = true;
		return true;
	}
	
	return InObject->GetClass()->IsChildOf(UDataRegistry::StaticClass())
		&& InAssetData.PackagePath.ToString().Contains(TEXT("Systems/Playlist/Data/AdvancedData"));
}

EDataValidationResult UGorgeousPlaylistSystemValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UDataRegistrySettings* DataRegistrySettings = GetMutableDefault<UDataRegistrySettings>();
	
	if (bIndirectValidationFlag)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
		TArray<FAssetData> AssetData;
		
		AssetRegistry.SearchAllAssets(true);
		FARFilter Filter;
		Filter.PackagePaths.Add("/GorgeousCore/Systems/Playlist/Data/AdvancedData");
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = true;
		
		const UClass* Class = UDataRegistry::StaticClass();
		Filter.ClassPaths.Add(Class->GetClassPathName());
		
		AssetRegistry.GetAssets(Filter, AssetData);
		
		TArray<FAssetData> DataRegistryAssets;
		for (const FAssetData& Asset : AssetData)
		{
			bool bIsDataRegistryAsset = false;
			for (const auto& [Path] : DataRegistrySettings->DirectoriesToScan)
			{
				if (Asset.PackagePath.ToString() == Path)
				{
					bIsDataRegistryAsset = true;
					break;
				}
			}
			
			if (!bIsDataRegistryAsset)
			{
				DataRegistryAssets.Add(Asset);
			}
		}
		
		for (const FAssetData& LoadedAsset : DataRegistryAssets)
		{
			if (LoadedAsset.GetClass()->IsChildOf(UDataRegistry::StaticClass()))
			{
				UGT_EditorLogging_FL::LogMessageWithActionHyperlink(
				FString::Printf(TEXT("The DataRegistry asset %s is located in %s, but this path is not registered in the DataRegistry settings. Please add this path to the DirectoriesToScan array to ensure proper functionality."), *LoadedAsset.AssetName.ToString(), *LoadedAsset.PackagePath.ToString()),
				"GT.Systems.Playlist.DataRegistry_Entry",
				Logging_Warning,
				FName("GT.Systems.PlaylistSystem.Validator.RegisterDirectory"),
				LoadedAsset.PackagePath.ToString(),
				"Add Path");
				
				return EDataValidationResult::Invalid;
			}
		}
		return EDataValidationResult::Valid;
	}
	
	for (const auto& [Path] : DataRegistrySettings->DirectoriesToScan)
	{
		if (Path == InAssetData.PackagePath.ToString())
		{
			return EDataValidationResult::Valid;
		}
	}
		
	UGT_EditorLogging_FL::LogMessageWithActionHyperlink(
		FString::Printf(TEXT("The DataRegistry asset is located in %s, but this path is not registered in the DataRegistry settings. Please add this path to the DirectoriesToScan array to ensure proper functionality."), *InAssetData.PackagePath.ToString()),
		"GT.Systems.Playlist.DataRegistry_Entry",
		Logging_Warning,
		FName("GT.Systems.PlaylistSystem.Validator.RegisterDirectory"),
		InAssetData.PackagePath.ToString(),
		"Add Path");
			
	return EDataValidationResult::Invalid;
}

void UGorgeousPlaylistSystemValidator::HandleRegisterDirectoryHyperlink(const FString& Payload)
{
	UDataRegistrySettings* Settings = GetMutableDefault<UDataRegistrySettings>();

	Settings->Modify();

	FDirectoryPath NewPath;
	NewPath.Path = Payload;

	const bool bAlreadyExists = Settings->DirectoriesToScan.ContainsByPredicate(
		[&](const FDirectoryPath& Existing)
		{
			return Existing.Path == NewPath.Path;
		});

	if (!bAlreadyExists)
	{
		Settings->DirectoriesToScan.Add(NewPath);
	}

	// Force write to config file
	Settings->TryUpdateDefaultConfigFile();
	Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());
	
	const EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::FromString(TEXT("Directory added to DataRegistry settings. Please restart the editor for changes to take effect. \n\nRestart now?"))
	);

	if (Result == EAppReturnType::Yes)
	{
		FUnrealEdMisc::Get().RestartEditor(false);
	}
}
