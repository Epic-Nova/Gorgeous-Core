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
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "MessageLogModule.h"
#include "Logging/TokenizedMessage.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "GorgeousLoggingDeveloperSettings.h"
#include "Libraries/GorgeousEditorLoggingUtility.h"
#include "Libraries/GorgeousLoggingBlueprintFunctionLibrary.h"
#include "Logging/MessageLog.h"
#include "Styling/CoreStyle.h"
#include "ToolMenus.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
//<--------------------------=== Module Includes ===------------------------->
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
			UObject* Asset = Entry.Hyperlink.TargetAsset.ResolveObject();
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
	TSharedRef<FTokenizedMessage> Tokenized = FTokenizedMessage::Create(ToMessageSeverity(Entry.Importance));
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

namespace
{
	FToolMenuOwnerScoped GDebugMenuOwner{TEXT("GorgeousCoreEditorUtilities")};

	void RegisterDebugMenuEntry()
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			return;
		}

		UToolMenus* ToolMenus = UToolMenus::Get();
		if (!ToolMenus)
		{
			return;
		}

		UToolMenu* AuditMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Tools.Audit");
		if (!AuditMenu)
		{
			return;
		}

		FToolMenuSection& AuditSection = AuditMenu->FindOrAddSection("GorgeousCoreAudit");
		AuditSection.AddMenuEntry(
			"GorgeousCore_ShowDebugPanel",
			NSLOCTEXT("GorgeousCoreAudit", "ShowDebugPanel", "Gorgeous Insight Matrix"),
			NSLOCTEXT("GorgeousCoreAudit", "ShowDebugPanel_Tooltip", "Open the Gorgeous Insight Matrix debug panel."),
			FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "Icons.Info"),
			FUIAction(
				FExecuteAction::CreateLambda([]
				{
					if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
					{
						Subsystem->ShowDebugPanel();
					}
				}),
				FCanExecuteAction::CreateLambda([]
				{
					return UGorgeousInsightMatrixSubsystem::Get() != nullptr;
				})));
	}

	void UnregisterDebugMenuEntry()
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			return;
		}

		UToolMenus* ToolMenus = UToolMenus::Get();
		if (!ToolMenus)
		{
			return;
		}

		ToolMenus->UnregisterOwner(GDebugMenuOwner.GetOwner());
	}
}

void FGorgeousCoreEditorUtilitiesModule::GorgeousStartupModule()
{
	const UGorgeousLoggingDeveloperSettings* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
	if (Settings && (Settings->bEnableGorgeousMessageLog || Settings->bEmitSampleLogsOnStartup))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
		FMessageLogInitializationOptions Options;
		Options.bShowInLogWindow = true;
		Options.bAllowClear = true;
		Options.bScrollToBottom = true;
		MessageLogModule.RegisterLogListing(Settings->MessageLogListingName, LOCTEXT("GorgeousThingsLog", "Gorgeous Things"), Options);
		RegisteredLogListingName = Settings->MessageLogListingName;
	}

	if (Settings && Settings->bEmitSampleLogsOnStartup)
	{
		FGorgeousLogEntry Entry;
		Entry.Message = TEXT("[GT.Core.MessageLog.Sample] Gorgeous Things Message Log is enabled.");
		Entry.LoggingKey = TEXT("GT.Core.MessageLog.Sample");
		Entry.Importance = Logging_Information;
		Entry.Duration = 5.0f;
		Entry.bPrintToLog = true;
		Entry.bPrintToScreen = false;
		EmitGorgeousLogEntry(Entry, true);

		FGorgeousLogEntry HyperlinkEntry;
		HyperlinkEntry.Message = TEXT("[GT.Core.MessageLog.Hyperlink] Example hyperlink log entry (click to open asset).");
		HyperlinkEntry.LoggingKey = TEXT("GT.Core.MessageLog.Hyperlink");
		HyperlinkEntry.Importance = Logging_Information;
		HyperlinkEntry.Duration = 5.0f;
		HyperlinkEntry.bPrintToLog = true;
		HyperlinkEntry.bPrintToScreen = false;
		HyperlinkEntry.Hyperlink.LinkText = TEXT("Open Asset");
		HyperlinkEntry.Hyperlink.TargetAsset = FSoftObjectPath(TEXT("/Game/NewBlueprint.NewBlueprint"));
		EmitGorgeousLogEntry(HyperlinkEntry, true);
	}

	LogEntryHandle = GetGorgeousLogEntryDelegate().AddRaw(this, &FGorgeousCoreEditorUtilitiesModule::HandleGorgeousLogEntry);
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FGorgeousCoreEditorUtilitiesModule::HandleBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddRaw(this, &FGorgeousCoreEditorUtilitiesModule::HandleEndPIE);
	RegisterDebugMenuEntry();
}

void FGorgeousCoreEditorUtilitiesModule::GorgeousShutdownModule()
{
	if (BeginPIEHandle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
		BeginPIEHandle.Reset();
	}

	if (EndPIEHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(EndPIEHandle);
		EndPIEHandle.Reset();
	}

	if (LogEntryHandle.IsValid())
	{
		GetGorgeousLogEntryDelegate().Remove(LogEntryHandle);
		LogEntryHandle.Reset();
	}

	UnregisterDebugMenuEntry();

	if (!RegisteredLogListingName.IsNone() && FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		if (FMessageLogModule* MessageLogModule = FModuleManager::GetModulePtr<FMessageLogModule>("MessageLog"))
		{
			MessageLogModule->UnregisterLogListing(RegisteredLogListingName);
		}
		RegisteredLogListingName = NAME_None;
	}
}

TArray<FName> FGorgeousCoreEditorUtilitiesModule::GetDependentPlugins() const
{
	return TArray<FName>();
}

int32 FGorgeousCoreEditorUtilitiesModule::GetMinimumRequiredCoreVersion() const
{
	//Actually not needed as the Core does not perform checks against itself.
	return 90; // Version 0.9
}

void FGorgeousCoreEditorUtilitiesModule::HandleGorgeousLogEntry(const FGorgeousLogEntry& Entry)
{
	EmitGorgeousLogEntry(Entry, false);
}

void FGorgeousCoreEditorUtilitiesModule::HandleBeginPIE(bool bIsSimulating)
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		if (Subsystem->GetQueuedTestCount() > 0)
		{
			Subsystem->RunQueuedTests();
			Subsystem->RefreshDebugPanel();
		}
	}
}

void FGorgeousCoreEditorUtilitiesModule::HandleEndPIE(bool bIsSimulating)
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->RefreshDebugPanel();
	}
}

IMPLEMENT_MODULE(FGorgeousCoreEditorUtilitiesModule, GorgeousCoreEditorUtilities)

#undef LOCTEXT_NAMESPACE
