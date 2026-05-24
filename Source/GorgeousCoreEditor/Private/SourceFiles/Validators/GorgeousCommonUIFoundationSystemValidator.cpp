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
#include "ENgine/GameViewportClient.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousCommonUIFoundationSystemValidator Implementation
//=============================================================================

UGorgeousCommonUIFoundationSystemValidator::UGorgeousCommonUIFoundationSystemValidator()
{
	UGT_EditorLogging_FL::RegisterLogHyperlinkAction(
		StaticClass(),
		FName("GT.Systems.CommonUIFoundation.Validator.RegisterDirectory"),
		FName("HandleRegisterDirectoryHyperlink"));

	UGT_EditorLogging_FL::RegisterLogHyperlinkAction(
		StaticClass(),
		FName("GT.Systems.CommonUIFoundation.Validator.FixViewportClient"),
		FName("HandleFixViewportClientHyperlink"));

	UGT_EditorLogging_FL::RegisterLogHyperlinkCondition(
		StaticClass(),
		FName("GT.Systems.CommonUIFoundation.Validator.CanFixViewportClient"),
		FName("HandleCanFixViewportClientHyperlink"));
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
	return InAssetData.PackageName.ToString().Contains(TEXT("InputData")) && InObject->IsA<UBlueprint>();
#else
	return false;
#endif
}

EDataValidationResult UGorgeousCommonUIFoundationSystemValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(COMMONUIFOUNDATION)
	ValidateViewportClient();

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
			"GT.Systems.CommonUIFoundation.DataRegistry_Entry",
			Logging_Warning,
			FName("GT.Systems.CommonUIFoundation.Validator.RegisterDirectory"),
			InAssetData.PackagePath.ToString(),
			"Add Path");
			
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
		UGT_EditorLogging_FL::LogMessageWithActionHyperlink(
			TEXT("The Game Viewport Client is not set to CommonGameViewportClient (or a subclass). This is required for proper UI foundation functionality."),
			"GT.Systems.CommonUIFoundation.ViewportClient",
			Logging_Warning,
			FName("GT.Systems.CommonUIFoundation.Validator.FixViewportClient"),
			TEXT(""),
			TEXT("Fix Viewport Client"),
			true,
			FName("GT.Systems.CommonUIFoundation.Validator.CanFixViewportClient"));
	}
#endif
}

void UGorgeousCommonUIFoundationSystemValidator::HandleRegisterDirectoryHyperlink(const FString& Payload)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(COMMONUIFOUNDATION)
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

void UGorgeousCommonUIFoundationSystemValidator::HandleFixViewportClientHyperlink(const FString& Payload)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(COMMONUIFOUNDATION)
	UEngine* Engine = GetMutableDefault<UEngine>();
	Engine->Modify();
	Engine->GameViewportClientClassName = FSoftClassPath(TEXT("/Script/CommonUI.CommonGameViewportClient"));
	
	// Save to DefaultEngine.ini
	Engine->TryUpdateDefaultConfigFile();
	Engine->SaveConfig(CPF_Config, *Engine->GetDefaultConfigFilename());

	const EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::FromString(TEXT("Game Viewport Client set to CommonGameViewportClient. Please restart the editor for changes to take effect. \n\nRestart now?"))
	);

	if (Result == EAppReturnType::Yes)
	{
		FUnrealEdMisc::Get().RestartEditor(false);
	}
#endif
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
