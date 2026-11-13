// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "Validators/GorgeousPrimaryAssetDiscoveryValidator.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
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

		GT_W_LOG(TEXT("GT.Validation"), TEXT("%s"), *(FString::Printf(TEXT("Primary Asset '%s' of type '%s' is not registered in the Asset Manager. This will cause slow lookups at runtime."), *InAssetData.AssetName.ToString(), *ClassName)));

		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

#include "GeneralSystems/GorgeousPrimaryDataAsset.h"

void UGorgeousPrimaryAssetDiscoveryValidator::HandleRegisterAssetType(const FString& Payload)
{
	GT_W_LOG(TEXT("GT.Validation"), TEXT("Validators only diagnose. Configure the system through a GSC Contribute Project Settings action or edit Project Settings explicitly."));
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
