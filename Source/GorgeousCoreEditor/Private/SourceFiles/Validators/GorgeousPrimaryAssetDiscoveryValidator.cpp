// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "Validators/GorgeousPrimaryAssetDiscoveryValidator.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"
#include "Misc/MessageDialog.h"

// Assets to validate
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"

UGorgeousPrimaryAssetDiscoveryValidator::UGorgeousPrimaryAssetDiscoveryValidator()
{
	UGT_EditorLogging_FL::RegisterLogHyperlinkAction(
		StaticClass(),
		FName("GT.AssetManager.RegisterType"),
		FName("HandleRegisterAssetType"));
}

bool UGorgeousPrimaryAssetDiscoveryValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (!InObject) return false;

	return InObject->IsA<UGorgeousPrimaryDataAsset>();
}

EDataValidationResult UGorgeousPrimaryAssetDiscoveryValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UGorgeousPrimaryDataAsset* PDA = Cast<UGorgeousPrimaryDataAsset>(InAsset);
	if (!PDA) return EDataValidationResult::Valid;

	if (!PDA->RegistrationConfig.bShouldRegister || !PDA->RegistrationConfig.bRequireRegistration)
	{
		return EDataValidationResult::Valid;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	FPrimaryAssetId AssetId = AssetManager.GetPrimaryAssetIdForObject(PDA);

	if (!AssetId.IsValid())
	{
		const FString ClassName = PDA->GetClass()->GetName();
		const FString AssetPath = PDA->GetPathName();

		UGT_EditorLogging_FL::LogMessageWithActionHyperlink(
			FString::Printf(TEXT("Primary Asset '%s' of type '%s' is not registered in the Asset Manager. This will cause slow lookups at runtime."), *InAssetData.AssetName.ToString(), *ClassName),
			FString::Printf(TEXT("GT.AssetManager.Unregistered.%s"), *InAssetData.AssetName.ToString()),
			Logging_Warning,
			FName("GT.AssetManager.RegisterType"),
			FString::Printf(TEXT("%s|%s"), *ClassName, *AssetPath),
			"Register in Asset Manager"
		);

		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

#include "GeneralSystems/GorgeousPrimaryDataAsset.h"

void UGorgeousPrimaryAssetDiscoveryValidator::HandleRegisterAssetType(const FString& Payload)
{
	FString ClassName;
	FString AssetPath;
	Payload.Split(TEXT("|"), &ClassName, &AssetPath);

	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();
	if (!Settings) return;

	// Load the asset to access its dynamic metadata
	UGorgeousPrimaryDataAsset* PDA = LoadObject<UGorgeousPrimaryDataAsset>(nullptr, *AssetPath);
	if (!PDA) return;

	const FPrimaryAssetType Type = PDA->GetPrimaryAssetType();
	if (!Type.IsValid()) return;

	const FName TypeName = Type.GetName();

	FString PackagePath = FPackageName::GetLongPackagePath(AssetPath);
	FDirectoryPath FallbackDir;
	FallbackDir.Path = PackagePath;

	// Check if already exists
	FPrimaryAssetTypeInfo* ExistingInfo = nullptr;
	for (FPrimaryAssetTypeInfo& Info : Settings->PrimaryAssetTypesToScan)
	{
		if (Info.PrimaryAssetType == TypeName)
		{
			ExistingInfo = &Info;
			break;
		}
	}

	bool bModified = false;

	if (ExistingInfo)
	{
		const bool bHasFallbackDir = ExistingInfo->GetDirectories().ContainsByPredicate(
			[&FallbackDir](const FDirectoryPath& Existing)
			{
				return Existing.Path.Equals(FallbackDir.Path, ESearchCase::IgnoreCase);
			});

		if (!bHasFallbackDir)
		{
			ExistingInfo->GetDirectories().Add(FallbackDir);
			bModified = true;
		}
	}
	else
	{
		FPrimaryAssetTypeInfo NewInfo;
		NewInfo.PrimaryAssetType = Type;
		
		// Use the actual class of the asset as the base class for the scanner
		NewInfo.SetAssetBaseClass(PDA->GetClass());
		NewInfo.bHasBlueprintClasses = false;

		// 1. Add preferred scan paths if defined
		TArray<FString> PreferredPaths = PDA->GetPreferredScanPaths();
		if (PreferredPaths.Num() > 0)
		{
			// Try to resolve preferred paths relative to the plugin content root
			FString PluginContentRoot;
			if (AssetPath.StartsWith(TEXT("/")))
			{
				int32 SecondSlash;
				if (AssetPath.RightChop(1).FindChar(TEXT('/'), SecondSlash))
				{
					PluginContentRoot = AssetPath.Left(SecondSlash + 2);
				}
			}

			if (!PluginContentRoot.IsEmpty())
			{
				for (const FString& RelPath : PreferredPaths)
				{
					FDirectoryPath Dir;
					Dir.Path = PluginContentRoot / RelPath;
					NewInfo.GetDirectories().Add(Dir);
				}
			}
		}

		// 2. Always add the current asset's directory as a fallback/immediate fix
		const bool bHasFallbackDir = NewInfo.GetDirectories().ContainsByPredicate(
			[&FallbackDir](const FDirectoryPath& Existing)
			{
				return Existing.Path.Equals(FallbackDir.Path, ESearchCase::IgnoreCase);
			});
		if (!bHasFallbackDir)
		{
			NewInfo.GetDirectories().Add(FallbackDir);
		}

		Settings->PrimaryAssetTypesToScan.Add(NewInfo);
		bModified = true;
	}

	if (bModified)
	{
		Settings->Modify();
		Settings->TryUpdateDefaultConfigFile();
		Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());

		UAssetManager::Get().ReinitializeFromConfig();

		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(NSLOCTEXT("GT.AssetManager", "Registered", "Registered Type '{0}' and path '{1}' in Asset Manager settings."), FText::FromName(TypeName), FText::FromString(PackagePath)));
	}
}

bool UGorgeousPrimaryAssetDiscoveryValidator::IsTypeRegistered(const FPrimaryAssetType& Type, UClass* BaseClass) const
{
	const UAssetManagerSettings* Settings = GetDefault<UAssetManagerSettings>();
	for (const FPrimaryAssetTypeInfo& Info : Settings->PrimaryAssetTypesToScan)
	{
		if (Info.PrimaryAssetType == Type.GetName())
		{
			return true;
		}
	}
	return false;
}
