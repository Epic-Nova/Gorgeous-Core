// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#include "GorgeousCoreEditorUtilitiesModule.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "GorgeousLoggingDeveloperSettings.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Editor.h"
#include "IMessageLogListing.h"
#include "Framework/Notifications/NotificationManager.h"
#include "MessageLogModule.h"
#include "Logging/MessageLog.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameplayTagsManager.h"
#include "GameplayTagsModule.h"
#include "GameplayTagsSettings.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "SourceCodeNavigation.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreEditorUtilitiesModule Implementation
//=============================================================================

#define LOCTEXT_NAMESPACE "GorgeousCoreEditorUtilities"

namespace GorgeousEditorLogging
{
	void ExecuteLogHyperlinkAction(const FGorgeousLogHyperlink& Hyperlink);
	bool EvaluateLogHyperlinkCondition(const FName& ConditionName, const FString& ActionPayload);
}

namespace
{
    // Only the editor/game thread reads the description cache and builds Slate log tokens.
    TMap<FName, FString> LoggingDescriptionCache;
    TAtomic<bool> bLoggingDescriptionsDirty(true);
    FDelegateHandle LoggingTagTreeChangedHandle;
    FDelegateHandle ClearLogOnBeginPIEHandle;
    FTSTicker::FDelegateHandle PendingLogTicker;
    FCriticalSection PendingLogLock;
    TArray<FGorgeousLogEntry> PendingLogEntries;
    bool bAcceptLogEntries = false;

    FString GetLoggingKeyDescription(FName Key)
    {
        if (bLoggingDescriptionsDirty.Exchange(false)) LoggingDescriptionCache.Reset();
        if (Key.IsNone() || IsEngineExitRequested()) return FString();
        // UObject registration can allocate the manager before the module's
        // StartupModule runs. Read existing data without forcing initialization.
        UGameplayTagsManager* Manager = UGameplayTagsManager::GetIfAllocated();
        if (!Manager) return FString();
        if (const FString* Cached = LoggingDescriptionCache.Find(Key)) return *Cached;
        FString Comment;
        TArray<FName> Sources;
        bool bExplicit = false, bRestricted = false, bAllowChildren = false;
        const bool bRegistered = Manager->GetTagEditorData(Key, Comment, Sources, bExplicit, bRestricted, bAllowChildren);
        bool bFoundIniRow = false;
        for (const FName SourceName : Sources)
        {
            const FGameplayTagSource* Source = Manager->FindTagSource(SourceName);
            if (!Source || !Source->SourceTagList) continue;
            const FGameplayTagTableRow* Row = Source->SourceTagList->GameplayTagList.FindByPredicate(
                [Key](const FGameplayTagTableRow& Candidate) { return Candidate.Tag == Key; });
            if (!Row) continue;
            // A native child can fill its implicit parent's merged node comment
            // before the parent INI row loads. Prefer that exact row's metadata.
            const bool bLoggingSource = SourceName.ToString().EndsWith(TEXT("LoggingTags.ini"), ESearchCase::IgnoreCase);
            if (!bFoundIniRow || bLoggingSource)
            {
                Comment = Row->DevComment;
                bFoundIniRow = true;
            }
            if (bLoggingSource) break;
        }
        // Implicit/empty startup metadata can be populated later. Cache only real
        // described entries, not arbitrary user keys or temporary empty comments.
        if (bRegistered && bExplicit && !Comment.IsEmpty()) LoggingDescriptionCache.Add(Key, Comment);
        return Comment;
    }

    FText MakeLogMetadataTooltip(const FGorgeousLogEntry& Entry)
    {
        // The entry already contains the message formatted with this call's arguments.
        FString Tooltip = Entry.Message;
        if (Tooltip.IsEmpty())
            Tooltip = Entry.LoggingKey.IsNone() ? TEXT("Gorgeous Things") : Entry.LoggingKey.ToString();

        FString Description = GetLoggingKeyDescription(Entry.LoggingKey);
        // Older generated tag comments append format examples from other calls.
        // Keep the tag's description, without treating those examples as event data.
        const int32 ExampleStart = Description.Find(TEXT(" Examples:"), ESearchCase::IgnoreCase);
        if (ExampleStart != INDEX_NONE) Description.LeftInline(ExampleStart);
        Description.TrimStartAndEndInline();
        if (!Description.IsEmpty()) Tooltip += TEXT("\n\n") + Description;
        if (!Entry.SourceFile.IsEmpty() && Entry.SourceLine > 0)
        {
            Tooltip += FString::Printf(TEXT("\n\n%s:%d"), *Entry.SourceFile, Entry.SourceLine);
            if (!Entry.SourceFunction.IsEmpty()) Tooltip += TEXT("\n") + Entry.SourceFunction;
        }
        else
        {
            Tooltip += TEXT("\n\nNo C++ callsite was provided.");
        }
        return FText::FromString(Tooltip);
    }

    void AddLogMetadataTokens(const TSharedRef<FTokenizedMessage>& Message, const FGorgeousLogEntry& Entry)
    {
        Message->AddToken(FTextToken::Create(FText::FromString(Entry.Message)));
        const FText Tooltip = MakeLogMetadataTooltip(Entry);
        if (!Entry.SourceFile.IsEmpty() && Entry.SourceLine > 0)
        {
            const FString SourceFile = FPaths::ConvertRelativePathToFull(Entry.SourceFile);
            const int32 SourceLine = Entry.SourceLine;
            const auto CanOpenSource = [SourceFile]() { return FPaths::FileExists(SourceFile); };
            const auto OpenSource = [SourceFile, SourceLine, CanOpenSource]()
            {
                if (CanOpenSource()) FSourceCodeNavigation::OpenSourceFile(SourceFile, SourceLine);
            };

            const auto SourceToken = FActionToken::Create(LOCTEXT("OpenLogSource", "Source"), Tooltip,
                FOnActionTokenExecuted::CreateLambda(OpenSource),
                FCanExecuteActionToken::CreateLambda(CanOpenSource), false);
            SourceToken->OnMessageTokenActivated(FOnMessageTokenActivated::CreateLambda(
                [OpenSource](const TSharedRef<IMessageToken>&) { OpenSource(); }));
            Message->AddToken(SourceToken);
            Message->SetMessageLink(SourceToken);
        }
        else
        {
            // Blueprint callers do not have a C++ macro location. Expose their
            // tag description without inventing a link to a helper implementation.
            Message->AddToken(FActionToken::Create(
                LOCTEXT("CopyLogDetails", "Copy details"), Tooltip,
                FOnActionTokenExecuted::CreateLambda([Tooltip]()
                {
                    FPlatformApplicationMisc::ClipboardCopy(*Tooltip.ToString());
                }), false));
        }
    }
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

	if ((!Settings->bEnableGorgeousMessageLog || !Entry.bPrintToMessageLog) && !bForce)
	{
		return;
	}

	const TSharedRef<bool> Consumed = MakeShared<bool>(false);
	const TSharedRef<TWeakPtr<SNotificationItem>> Notification = MakeShared<TWeakPtr<SNotificationItem>>();
	auto CanExecuteHyperlink = [Entry, Consumed]()
	{
		return (!Entry.Hyperlink.bSingleUse || !*Consumed) &&
			GorgeousEditorLogging::EvaluateLogHyperlinkCondition(Entry.Hyperlink.ConditionName, Entry.Hyperlink.ActionPayload);
	};
	auto ExecuteHyperlink = [Entry, Consumed, Notification, CanExecuteHyperlink]()
	{
		if (!CanExecuteHyperlink()) return;
		if (Entry.Hyperlink.bSingleUse)
		{
			*Consumed = true;
			if (TSharedPtr<SNotificationItem> Item = Notification->Pin()) Item->ExpireAndFadeout();
		}
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
	// Runtime logging independently routes Output Log; keep this UI sink from mirroring it a second time.
	MessageLog.SuppressLoggingToOutputLog(true);
	const TSharedRef<FTokenizedMessage> Tokenized = FTokenizedMessage::Create(ToMessageSeverity(Entry.Importance));
	AddLogMetadataTokens(Tokenized, Entry);

	if (Entry.Hyperlink.IsValid())
	{
		Tokenized->AddToken(FActionToken::Create(
			FText::FromString(Entry.Hyperlink.LinkText),
			FText::FromString(Entry.Message),
			FOnActionTokenExecuted::CreateLambda(ExecuteHyperlink),
			FCanExecuteActionToken::CreateLambda(CanExecuteHyperlink),
			Entry.Hyperlink.bSingleUse
		));

		FNotificationInfo Info(FText::FromString(Entry.Message));
		Info.bUseThrobber = false;
		Info.bUseLargeFont = false;
		Info.ExpireDuration = Entry.Duration > 0.0f ? Entry.Duration : 5.0f;
		Info.HyperlinkText = FText::FromString(Entry.Hyperlink.LinkText);
		Info.Hyperlink = FSimpleDelegate::CreateLambda(ExecuteHyperlink);
		*Notification = FSlateNotificationManager::Get().AddNotification(Info);
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
	
    {
        FScopeLock Lock(&PendingLogLock);
        bAcceptLogEntries = true;
    }
    // BeginPIE runs after startup checks and before the play world initializes.
    ClearLogOnBeginPIEHandle = FEditorDelegates::BeginPIE.AddLambda([](bool)
    {
        {
            FScopeLock Lock(&PendingLogLock);
            PendingLogEntries.Reset();
        }

        const auto* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
        if (auto* MessageLog = FModuleManager::GetModulePtr<FMessageLogModule>("MessageLog");
            Settings && MessageLog && MessageLog->IsRegisteredLogListing(Settings->MessageLogListingName))
        {
            MessageLog->GetLogListing(Settings->MessageLogListingName)->ClearMessages();
        }
    });

    LoggingTagTreeChangedHandle = IGameplayTagsModule::OnGameplayTagTreeChanged.AddLambda([]()
    {
        bLoggingDescriptionsDirty.Store(true);
    });
    PendingLogTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float)
    {
        TArray<FGorgeousLogEntry> Entries;
        {
            FScopeLock Lock(&PendingLogLock);
            Swap(Entries, PendingLogEntries);
        }
        for (const FGorgeousLogEntry& Entry : Entries) EmitGorgeousLogEntry(Entry, false);
        return true;
    }));
    LogEntryHandle = GetGorgeousLogEntryDelegate().AddLambda([](const FGorgeousLogEntry& Entry)
    {
        if (IsInGameThread()) EmitGorgeousLogEntry(Entry, false);
        else
        {
            // Native output remains synchronous. Its editor presentation waits for
            // the game thread rather than creating Message Log/Slate widgets here.
            FScopeLock Lock(&PendingLogLock);
            if (bAcceptLogEntries) PendingLogEntries.Add(Entry);
        }
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
    FEditorDelegates::BeginPIE.Remove(ClearLogOnBeginPIEHandle);
    ClearLogOnBeginPIEHandle.Reset();

    {
        FScopeLock Lock(&PendingLogLock);
        bAcceptLogEntries = false;
        PendingLogEntries.Reset();
    }
    if (PendingLogTicker.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(PendingLogTicker);
        PendingLogTicker.Reset();
    }
    if (LoggingTagTreeChangedHandle.IsValid())
    {
        IGameplayTagsModule::OnGameplayTagTreeChanged.Remove(LoggingTagTreeChangedHandle);
        LoggingTagTreeChangedHandle.Reset();
    }
    LoggingDescriptionCache.Reset();

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