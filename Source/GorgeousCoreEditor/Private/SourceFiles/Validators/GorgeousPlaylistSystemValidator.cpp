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
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "DataRegistry.h"
#include "Helpers/Macros/GorgeousExtensionHelperMacros.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "DataRegistrySettings.h"
#include "UnrealEdMisc.h"
#include "Engine/AssetManager.h"
#include "Engine/Blueprint.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousPlaylistSystemValidator Implementation
//=============================================================================

UGorgeousPlaylistSystemValidator::UGorgeousPlaylistSystemValidator()
{
	
}

UGorgeousPlaylistSystemValidator::~UGorgeousPlaylistSystemValidator()
{
	UGT_EditorLogging_FL::UnregisterLogHyperlinkAction("GT.Systems.PlaylistSystem.Validator.RegisterDirectory");
}

bool UGorgeousPlaylistSystemValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UDataRegistry>() && InAssetData.PackageName.ToString().StartsWith(TEXT("/GorgeousCore/Systems/Playlist/"));
}

EDataValidationResult UGorgeousPlaylistSystemValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
#if defined(GORGEOUS_SYSTEM_INSTALLED_PLAYLIST)
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
		GT_W_LOG(TEXT("GT.Validation"), TEXT("%s"), *(FString::Printf(TEXT("The DataRegistry asset %s is located in %s, but this path is not registered in the DataRegistry settings. This may prevent proper loading."), *InAssetData.AssetName.ToString(), *InAssetData.PackagePath.ToString())));
			
		return EDataValidationResult::Invalid;
	}
#endif
#endif // defined(GORGEOUS_SYSTEM_INSTALLED_PLAYLIST)
	
	return EDataValidationResult::Valid;
}

void UGorgeousPlaylistSystemValidator::HandleRegisterDirectoryHyperlink(const FString& Payload)
{
	GT_W_LOG(TEXT("GT.Validation"), TEXT("Validators only diagnose. Configure the system through a GSC Contribute Project Settings action or edit Project Settings explicitly."));
}
