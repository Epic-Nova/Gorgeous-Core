// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "GorgeousCoreEditorUtilitiesModule.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Editor.h"
#include "MessageLogModule.h"
#include "Logging/MessageLog.h"
#include "Subsystems/AssetEditorSubsystem.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "GorgeousLoggingDeveloperSettings.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#include "AssetRegistry/AssetRegistryModule.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreEditorUtilitiesModule Implementation
//=============================================================================

#define LOCTEXT_NAMESPACE "GorgeousCoreEditorUtilities"

namespace GorgeousEditorLogging
{
	void ExecuteLogHyperlinkAction(const FGorgeousLogHyperlink& Hyperlink);
}

static EMessageSeverity::Type ToMessageSeverity(const EGorgeousLoggingImportance Importance)
{
	switch (Importance)
	{
		case Logging_Warning:
			return EMessageSeverity::Warning;
		case Logging_Error:
		case Logging_Fatal:
			return EMessageSeverity::Error;
		case Logging_Success:
		case Logging_Information:
		default:
			return EMessageSeverity::Info;
	}
}

static void OpenAssetInEditor(const FSoftObjectPath& AssetPath)
{
	if (!AssetPath.IsValid())
		return;

	UObject* LoadedAsset = AssetPath.TryLoad();
	if (!LoadedAsset || !GEditor)
		return;

	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->OpenEditorForAsset(LoadedAsset);
	}
}

static void ShowMissingAssetNotification()
{
	FNotificationInfo Info(LOCTEXT("MissingAssetNotification", "Asset registry entry missing."));
	Info.ExpireDuration = 5.0f;
	Info.bUseLargeFont = false;

	FSlateNotificationManager::Get().AddNotification(Info);
}

static void EmitGorgeousLogEntry(const FGorgeousLogEntry& Entry, bool bForce)
{
	const UGorgeousLoggingDeveloperSettings* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	if (!Settings->bEnableGorgeousMessageLog && !bForce)
	{
		return;
	}

	auto ExecuteHyperlink = [Entry]()
	{
		if (!Entry.Hyperlink.IsValid())
		{
			ShowMissingAssetNotification();
			return;
		}

		if (!Entry.Hyperlink.ActionName.IsNone())
		{
			GorgeousEditorLogging::ExecuteLogHyperlinkAction(Entry.Hyperlink);
			return;
		}

		if (Entry.Hyperlink.TargetAsset.IsValid())
		{
			const UObject* Asset = Entry.Hyperlink.TargetAsset.ResolveObject();
			if (!Asset)
			{
				Asset = Entry.Hyperlink.TargetAsset.TryLoad();
			}
			if (Asset)
			{
				OpenAssetInEditor(Entry.Hyperlink.TargetAsset);
				return;
			}
		}

		ShowMissingAssetNotification();
	};

	FMessageLog MessageLog(Settings->MessageLogListingName);
	const TSharedRef<FTokenizedMessage> Tokenized = FTokenizedMessage::Create(ToMessageSeverity(Entry.Importance));
	Tokenized->AddToken(FTextToken::Create(FText::FromString(Entry.Message)));

	if (Entry.Hyperlink.IsValid())
	{
		Tokenized->AddToken(FActionToken::Create(
			FText::FromString(Entry.Hyperlink.LinkText),
			FText::FromString(Entry.Message),
			FOnActionTokenExecuted::CreateLambda(ExecuteHyperlink)
		));

		FNotificationInfo Info(FText::FromString(Entry.Message));
		Info.bUseThrobber = false;
		Info.bUseLargeFont = false;
		Info.ExpireDuration = Entry.Duration > 0.0f ? Entry.Duration : 5.0f;
		Info.HyperlinkText = FText::FromString(Entry.Hyperlink.LinkText);
		Info.Hyperlink = FSimpleDelegate::CreateLambda(ExecuteHyperlink);
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	MessageLog.AddMessage(Tokenized);
}

void FGorgeousCoreEditorUtilitiesModule::GorgeousStartupModule()
{
	if (const UGorgeousLoggingDeveloperSettings* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
		Settings && (Settings->bEnableGorgeousMessageLog))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
		FMessageLogInitializationOptions Options;
		Options.bShowInLogWindow = true;
		Options.bAllowClear = true;
		Options.bScrollToBottom = true;
		MessageLogModule.RegisterLogListing(Settings->MessageLogListingName, LOCTEXT("GorgeousThingsLog", "Gorgeous Things"), Options);
		RegisteredLogListingName = Settings->MessageLogListingName;
	}
	
	LogEntryHandle = GetGorgeousLogEntryDelegate().AddLambda([](const FGorgeousLogEntry& Entry)
	{
		EmitGorgeousLogEntry(Entry, false);
	});


	
	// Reconcile plugin dependencies: remove stale ones from removed content
	// packs and add any missing ones for active guards.
	// Defer until the Asset Registry has finished its initial scan —
	// at module startup time the content scan is still in progress.
	IAssetRegistry& AssetRegistryRef = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry")).Get();

	if (AssetRegistryRef.IsLoadingAssets())
	{
		AssetRegistryRef.OnFilesLoaded().AddLambda([]() 
		{
			UGorgeousExtensionResourceGuard::ReconcilePluginDependencies();
		});
	}
	else
	{
		UGorgeousExtensionResourceGuard::ReconcilePluginDependencies();
	}
}

void FGorgeousCoreEditorUtilitiesModule::GorgeousShutdownModule()
{
	if (LogEntryHandle.IsValid())
	{
		GetGorgeousLogEntryDelegate().Remove(LogEntryHandle);
		LogEntryHandle.Reset();
	}
	
	if (!RegisteredLogListingName.IsNone() && FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		if (FMessageLogModule* MessageLogModule = FModuleManager::GetModulePtr<FMessageLogModule>("MessageLog"))
		{
			MessageLogModule->UnregisterLogListing(RegisteredLogListingName);
		}
		RegisteredLogListingName = NAME_None;
	}
}

IMPLEMENT_MODULE(FGorgeousCoreEditorUtilitiesModule, GorgeousCoreEditorUtilities)

#undef LOCTEXT_NAMESPACE