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
#include "Validators/GorgeousCommonUIFoundationSystemValidator.h"
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
#include "Engine/GameViewportClient.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousCommonUIFoundationSystemValidator Implementation
//=============================================================================

UGorgeousCommonUIFoundationSystemValidator::UGorgeousCommonUIFoundationSystemValidator()
{
	

	

	
}

UGorgeousCommonUIFoundationSystemValidator::~UGorgeousCommonUIFoundationSystemValidator()
{
	UGT_EditorLogging_FL::UnregisterLogHyperlinkAction("GT.Systems.CommonUIFoundation.Validator.RegisterDirectory");
	UGT_EditorLogging_FL::UnregisterLogHyperlinkAction("GT.Systems.CommonUIFoundation.Validator.FixViewportClient");
	UGT_EditorLogging_FL::UnregisterLogHyperlinkCondition("GT.Systems.CommonUIFoundation.Validator.CanFixViewportClient");
}

bool UGorgeousCommonUIFoundationSystemValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData,
	UObject* InObject, FDataValidationContext& InContext) const
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(COMMONUIFOUNDATION)
	return InObject && ((InObject->IsA<UDataRegistry>() && InAssetData.PackageName.ToString().StartsWith(TEXT("/GorgeousCore/Systems/CommonUIFoundation/"))) || InAssetData.AssetName == TEXT("PDA_CommonUIFoundation"));
#else
	return false;
#endif
}

EDataValidationResult UGorgeousCommonUIFoundationSystemValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(COMMONUIFOUNDATION)
	ValidateViewportClient();
	if (!InAsset || !InAsset->IsA<UDataRegistry>()) return EDataValidationResult::Valid;

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
	
	return EDataValidationResult::Valid;
}

void UGorgeousCommonUIFoundationSystemValidator::ValidateViewportClient()
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(COMMONUIFOUNDATION)
	// Check Game Viewport Client
	const UEngine* Engine = GetDefault<UEngine>();
	const FSoftClassPath CommonUIViewportClassPath(TEXT("/Script/CommonUI.CommonGameViewportClient"));
	
	bool bIsValidViewportClient = (Engine->GameViewportClientClassName == CommonUIViewportClassPath);
	if (!bIsValidViewportClient)
	{
		// Check for subclass
		if (UClass* CurrentClass = Engine->GameViewportClientClassName.TryLoadClass<UGameViewportClient>())
		{
			if (UClass* TargetBaseClass = CommonUIViewportClassPath.TryLoadClass<UGameViewportClient>())
			{
				bIsValidViewportClient = CurrentClass->IsChildOf(TargetBaseClass);
			}
		}
	}

	if (!bIsValidViewportClient)
	{
		GT_W_LOG(TEXT("GT.Validation"), TEXT("The Game Viewport Client is not set to CommonGameViewportClient (or a subclass). This is required for proper UI foundation functionality."));
	}
#endif
}

void UGorgeousCommonUIFoundationSystemValidator::HandleRegisterDirectoryHyperlink(const FString& Payload)
{
	GT_W_LOG(TEXT("GT.Validation"), TEXT("Validators only diagnose. Configure the system through a GSC Contribute Project Settings action or edit Project Settings explicitly."));
}

void UGorgeousCommonUIFoundationSystemValidator::HandleFixViewportClientHyperlink(const FString& Payload)
{
	GT_W_LOG(TEXT("GT.Validation"), TEXT("Validators only diagnose. Configure the system through a GSC Contribute Project Settings action or edit Project Settings explicitly."));
}

bool UGorgeousCommonUIFoundationSystemValidator::HandleCanFixViewportClientHyperlink(const FString& Payload)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(COMMONUIFOUNDATION)
	const UEngine* Engine = GetDefault<UEngine>();
	const FSoftClassPath CommonUIViewportClassPath(TEXT("/Script/CommonUI.CommonGameViewportClient"));
	
	if (Engine->GameViewportClientClassName == CommonUIViewportClassPath)
	{
		return false;
	}
	
	// Check for subclass
	if (UClass* CurrentClass = Engine->GameViewportClientClassName.TryLoadClass<UGameViewportClient>())
	{
		if (UClass* TargetBaseClass = CommonUIViewportClassPath.TryLoadClass<UGameViewportClient>())
		{
			return !CurrentClass->IsChildOf(TargetBaseClass);
		}
	}
	
	return true;
#else
	return false;
#endif
}
