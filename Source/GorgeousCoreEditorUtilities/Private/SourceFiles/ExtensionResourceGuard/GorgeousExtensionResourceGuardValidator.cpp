// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuardValidator.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuardEnforcer.h"
#define GT_DURATION 15.0f
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"
#include "Validation/GorgeousValidationHelpers.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetRegistry/AssetRegistryModule.h"
#include "UnrealEdMisc.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousExtensionResourceGuardValidator Implementation
//=============================================================================

UGorgeousExtensionResourceGuardValidator::UGorgeousExtensionResourceGuardValidator()
{
	// Register the hyperlink action so the Gorgeous logging system can invoke
	// HandleEnableRequiredPlugin when the user clicks the "Enable" link.
	UGT_EditorLogging_FL::RegisterLogHyperlinkAction(
		StaticClass(),
		FName("GT.ExtensionGuard.EnablePlugin"),
		FName("HandleEnableRequiredPlugin"));
}

bool UGorgeousExtensionResourceGuardValidator::CanValidateAsset_Implementation(
	const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UGorgeousExtensionResourceGuard>();
}

EDataValidationResult UGorgeousExtensionResourceGuardValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UGorgeousExtensionResourceGuard* Guard = Cast<UGorgeousExtensionResourceGuard>(InAsset);
	if (!Guard)
	{
		return EDataValidationResult::NotValidated;
	}

	// Skip entirely if the content pack is not present on disk
	// The system is optional — if it was removed, the guard is inert.
	if (!Guard->IsContentPresent())
	{
		AssetPasses(Guard);
		return EDataValidationResult::Valid;
	}

	bool bHasErrors = false;

	// Validate SystemIdentifier is set
	if (Guard->SystemIdentifier.IsNone())
	{
		AssetFails(Guard, FText::FromString(TEXT("SystemIdentifier is not set.")));
		bHasErrors = true;
	}

	// Check RequiredPlugins are enabled
	for (const FName& PluginName : Guard->RequiredPlugins)
	{
		if (PluginName.IsNone())
		{
			if (GorgeousValidation::ReportWarningOrError(this, Guard,
				TEXT("RequiredPlugins contains an empty entry.")))
			{
				bHasErrors = true;
			}
			continue;
		}

		if (const bool bPluginIsEnabled = IPluginManager::Get().FindEnabledPlugin(PluginName.ToString()).IsValid(); 
			!bPluginIsEnabled)
		{
			const FString ErrorMessage = FString::Printf(
				TEXT("Plugin '%s' is required by system '%s' but is not enabled. "
					"\n Content in this extension pack will not function correctly without it"),
				*PluginName.ToString(),
				*Guard->SystemDisplayName.ToString());

			// Hard validation error — this goes to the Data Validation message log.
			AssetFails(Guard, FText::FromString(ErrorMessage));
			bHasErrors = true;

			// Begin enforcement immediately — the user may never click the
			// hyperlink. If they do click it and accept the restart,
			// HandleEnableRequiredPlugin will call StopEnforcement().
			if (GEditor)
			{
				if (UGorgeousExtensionResourceGuardEnforcer* Enforcer =
					GEditor->GetEditorSubsystem<UGorgeousExtensionResourceGuardEnforcer>())
				{
					Enforcer->BeginEnforcement(
						PluginName.ToString(), Guard->SystemDisplayName.ToString());
				}
			}
		}
	}

	// Enforce uniqueness per SystemIdentifier
	if (!Guard->SystemIdentifier.IsNone())
	{
		const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
			TEXT("AssetRegistry")).Get();

		TArray<FAssetData> AllGuardAssets;
		AssetRegistry.GetAssetsByClass(UGorgeousExtensionResourceGuard::StaticClass()->GetClassPathName(),
			AllGuardAssets, /*bSearchSubClasses=*/ true);

		int32 DuplicateCount = 0;
		for (const FAssetData& OtherAsset : AllGuardAssets)
		{
			if (OtherAsset.GetSoftObjectPath() == InAssetData.GetSoftObjectPath())
			{
				continue; // Skip self
			}

			// Load the other guard to compare SystemIdentifier
			if (const UGorgeousExtensionResourceGuard* OtherGuard =
				Cast<UGorgeousExtensionResourceGuard>(OtherAsset.GetAsset()))
			{
				if (OtherGuard->SystemIdentifier == Guard->SystemIdentifier)
				{
					DuplicateCount++;
				}
			}
		}

		if (DuplicateCount > 0)
		{
			const FString DuplicateMessage = FString::Printf(
				TEXT("Duplicate Extension Resource Guard: %d other guard(s) share SystemIdentifier '%s'. "
					"Each system must have exactly one guard instance."),
				DuplicateCount,
				*Guard->SystemIdentifier.ToString());

			AssetFails(Guard, FText::FromString(DuplicateMessage));
			bHasErrors = true;

			GT_E_LOG("GT.ExtensionResourceGuard",
				TEXT("Duplicate guard detected for system '%s'. Remove extra instances to resolve."),
				*Guard->SystemIdentifier.ToString());
		}
	}
	
	if (bHasErrors)
	{
		return EDataValidationResult::Invalid;
	}

	AssetPasses(Guard);
	return EDataValidationResult::Valid;
}

void UGorgeousExtensionResourceGuardValidator::HandleEnableRequiredPlugin(const FString& PluginName)
{
	if (PluginName.IsEmpty())
	{
		GT_W_LOG("GT.ExtensionResourceGuard", TEXT("HandleEnableRequiredPlugin called with empty plugin name."));
		return;
	}

	// Verify the plugin exists at all
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
	if (!Plugin.IsValid())
	{
		GT_E_LOG("GT.ExtensionResourceGuard",
			TEXT("Plugin '%s' was not found. It may not be installed."), *PluginName);
		return;
	}

	// Already enabled — nothing to do
	if (Plugin->IsEnabled())
	{
		GT_I_LOG("GT.ExtensionResourceGuard",
			TEXT("Plugin '%s' is already enabled."), *PluginName);
		return;
	}

	const EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::Format(
			NSLOCTEXT("GorgeousExtensionResourceGuard", "RestartPrompt",
				"Plugin '{0}' has been automatically added as a dependency and will be activated on restart.\n\nRestart now?"),
			FText::FromString(PluginName)));

	if (Result == EAppReturnType::Yes)
	{
		// User accepted — stop enforcement if it was running
		if (GEditor)
		{
			if (UGorgeousExtensionResourceGuardEnforcer* Enforcer =
				GEditor->GetEditorSubsystem<UGorgeousExtensionResourceGuardEnforcer>())
			{
				Enforcer->StopEnforcement(PluginName);
			}
		}

		FUnrealEdMisc::Get().RestartEditor(/*bWarn=*/ false);
	}
	else
	{
		// User rejected — begin enforcement countdown
		if (GEditor)
		{
			if (UGorgeousExtensionResourceGuardEnforcer* Enforcer =
				GEditor->GetEditorSubsystem<UGorgeousExtensionResourceGuardEnforcer>())
			{
				Enforcer->BeginEnforcement(PluginName, PluginName);
			}
		}
	}
}