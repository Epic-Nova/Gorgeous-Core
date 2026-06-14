// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Events - Events functionality provider              |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#include "Validators/GorgeousPlaylistSystemValidator.h"
#include "Helpers/Macros/GorgeousExtensionHelperMacros.h"

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
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(PLAYLIST)
	return InAssetData.PackageName.ToString().Contains(TEXT("PlaylistObject")) && InObject->IsA<UBlueprint>();
#else
	return false;
#endif
}

EDataValidationResult UGorgeousPlaylistSystemValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(PLAYLIST)
	const UDataRegistrySettings* DataRegistrySettings = GetMutableDefault<UDataRegistrySettings>();
	
	// Ensure the asset is in a registered directory
	bool bIsRegisteredDir = false;
	for (const auto& [Path] : DataRegistrySettings->DirectoriesToScan)
	{
		if (InAssetData.PackagePath.ToString() == Path)
		{
			bIsRegisteredDir = true;
			break;
		}
	}
	
	if (!bIsRegisteredDir)
	{
		UGT_EditorLogging_FL::LogMessageWithActionHyperlink(
			FString::Printf(TEXT("The DataRegistry asset %s is located in %s, but this path is not registered in the DataRegistry settings. This may prevent proper loading."), *InAssetData.AssetName.ToString(), *InAssetData.PackagePath.ToString()),
			"GT.Systems.Playlist.DataRegistry_Entry",
			Logging_Warning,
			FName("GT.Systems.PlaylistSystem.Validator.RegisterDirectory"),
			InAssetData.PackagePath.ToString(),
			"Add Path");
			
		return EDataValidationResult::Invalid;
	}
#endif
	
	return EDataValidationResult::Valid;
}

void UGorgeousPlaylistSystemValidator::HandleRegisterDirectoryHyperlink(const FString& Payload)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(PLAYLIST)
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
#endif
}
