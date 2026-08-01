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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreRuntimeUtilitiesGlobals.h"
#include "GorgeousCoreRuntimeUtilitiesLogging.h"
#include "Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif
//<-------------------------------------------------------------------------->

/**
 * Struct containing all customizable options for toast notifications.
 *
 * @author Nils Bergemann
 */
struct FGorgeousToastParams
{
	// The title displayed at the top of the toast notification.
	FString Title;

	// The detailed message or description shown below the title.
	FString Message;

    // Icon style: 0 = information, 1 = warning, 2 = success, 3 = error.
    int32 ToastIconKind = 3;

	// Duration in seconds the toast will be visible before auto-dismissing.
	float ExpireDuration = 8.0f;

	// Whether to use the throbber animation (spinner).
	bool bUseThrobber = false;

	// Whether to display icons.
	bool bUseIcons = true;

	// Whether the notification auto-dismisses (true) or requires user interaction (false).
	bool bFireAndForget = true;

	// Optional hyperlink text
	FText HyperlinkText;

	// Optional hyperlink callback
	FSimpleDelegate HyperlinkDelegate;
};

//<============================--- Variables ---============================>

// Queue for notifications that need to be shown when editor is ready
static TArray<FGorgeousToastParams> QueuedToastNotifications;

// Flag indicating if queued notifications have been flushed
static bool GbToastNotificationsFlushed = false;
//<------------------------------------------------------------------------->


//<============================--- C++ Only ---=============================>

/**
 * Immediately displays a toast notification without queuing.
 * Internal use - prefer ShowToastNotification which handles queuing.
 *
 * @param Params Struct containing all notification parameters.
 */
FORCEINLINE static void ShowToastNotificationImmediate(const FGorgeousToastParams& Params)
{
#if WITH_EDITOR
	auto& NotificationManager = FSlateNotificationManager::Get();

	FNotificationInfo Info(FText::FromString(Params.Title));
	Info.SubText = FText::FromString(Params.Message);
	Info.bFireAndForget = Params.bFireAndForget;
	Info.ExpireDuration = Params.ExpireDuration;
	Info.bUseThrobber = Params.bUseThrobber;
	Info.bUseSuccessFailIcons = Params.bUseIcons;

    switch (Params.ToastIconKind)
    {
        case 0:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.InfoWithColor"));
            break;
        case 1:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.WarningWithColor"));
            break;
        case 2:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.SuccessWithColor"));
            break;
        case 3:
        default:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.ErrorWithColor"));
            break;
    }

	if (!Params.HyperlinkText.IsEmpty() && Params.HyperlinkDelegate.IsBound())
	{
		Info.Hyperlink = Params.HyperlinkDelegate;
		Info.HyperlinkText = Params.HyperlinkText;
	}

	NotificationManager.AddNotification(Info);
#endif
}

/**
 * Attempts to flush all queued toast notifications if the editor is ready.
 * Called automatically when notifications are queued.
 */
FORCEINLINE static void TryFlushQueuedToastNotifications()
{
#if WITH_EDITOR
	if (!FSlateNotificationManager::Get().AreNotificationsAllowed())
	{
		// Schedule retry
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float DeltaTime)
		{
			TryFlushQueuedToastNotifications();
			return false;
		}), 0.5f);
		return;
	}

	GbToastNotificationsFlushed = true;

	for (const FGorgeousToastParams& Params : QueuedToastNotifications)
	{
		ShowToastNotificationImmediate(Params);
	}

	QueuedToastNotifications.Empty();
#endif
}

/**
 * Shows a toast notification with full customization.
 * Use this when you need complete control over the notification appearance.
 *
 * @param Params Struct containing all notification parameters.
 */
FORCEINLINE static void ShowToastNotification(const FGorgeousToastParams& Params)
{
#if WITH_EDITOR
	// Queue if editor not ready yet
	if (!GbToastNotificationsFlushed)
	{
		QueuedToastNotifications.Add(Params);
		TryFlushQueuedToastNotifications();
		return;
	}

	ShowToastNotificationImmediate(Params);
#endif
}

/**
 * Shows a toast notification with minimal parameters.
 * Convenience function for quick error/warning notifications.
 *
 * @param Title The title of the notification.
 * @param Message The message to display.
 * @param ToastIconKind Icon style: 0 = information, 1 = warning, 2 = success, 3 = error.
 */
FORCEINLINE static void ShowToastNotification(const FString& Title, const FString& Message, const int32 ToastIconKind = 3)
{
	FGorgeousToastParams Params;
	Params.Title = Title;
	Params.Message = Message;
    Params.ToastIconKind = ToastIconKind;
	ShowToastNotification(Params);
}

namespace GorgeousLogging
{

    /**
     * Converts various input types to FText for logging purposes.
     *
     * @param In The input value to convert.
     * @return The converted FText.
     */
    FORCEINLINE FText GT_LogToText(const FText& In)
    {
        return In;
    }

    /** Converts various input types to FText for logging purposes.
     *
     * @param In The input value to convert.
     * @return The converted FText.
     */
    FORCEINLINE FText GT_LogToText(const FString& In)
    {
        return FText::FromString(In);
    }

    /** Converts various input types to FText for logging purposes.
     *
     * @param In The input value to convert.
     * @return The converted FText.
     */
    FORCEINLINE FText GT_LogToText(const TCHAR* In)
    {
        return FText::FromString(In);
    }

    // The messages that were logged and are currently on cooldown with their gameplay tag as key.
    inline TMap<FGameplayTag, FString> LoggedMessages;

    // The set of logging keys that are currently suppressed.
    inline TSet<FName> SuppressedLoggingKeys;

    /**
     * Checks if the GameplayTags module is available for use.
     *
     * @return True if GameplayTags can be used, false otherwise.
     */
    FORCEINLINE static bool CanUseGameplayTags()
    {
        return !IsEngineExitRequested() && FModuleManager::Get().IsModuleLoaded(TEXT("GameplayTags"));
    }

    /**
     * Determines if logging for a specific key should be suppressed.
     *
     * @param LoggingKey The unique key associated with the log message.
     * @return True if logging for the key is suppressed, false otherwise.
     */
    FORCEINLINE static bool ShouldSuppressKey(const FName& LoggingKey)
    {
        return LoggingKey != NAME_None && SuppressedLoggingKeys.Contains(LoggingKey);
    }

    /**
     * Sets the suppression state for a specific logging key.
     *
     * @param LoggingKey The unique key associated with the log message.
     * @param bShouldSuppress True to suppress logging for the key, false to allow it.
     */
    FORCEINLINE static void SetLoggingKeySuppressed(const FName& LoggingKey, const bool bShouldSuppress)
    {
        if (LoggingKey == NAME_None)
        {
            return;
        }

        if (SuppressedLoggingKeys.Contains(LoggingKey))
        {
            UE_LOG(LogGorgeousThings, Log, TEXT("Logging for key '%s' is already suppressed."), *LoggingKey.ToString());
             return;
        }

        if (bShouldSuppress)
        {
            SuppressedLoggingKeys.Add(LoggingKey);
            UE_LOG(LogGorgeousThings, Log, TEXT("Logging for key '%s' has been suppressed."), *LoggingKey.ToString());
        }
        else
        {
            SuppressedLoggingKeys.Remove(LoggingKey);
            UE_LOG(LogGorgeousThings, Log, TEXT("Logging for key '%s' has been unsuppressed."), *LoggingKey.ToString());
        }
    }

    /**
     * Checks if logging for a specific key is currently suppressed.
     *
     * @param LoggingKey The unique key associated with the log message.
     * @return True if logging for the key is suppressed, false otherwise.
     */
    FORCEINLINE static bool IsLoggingKeySuppressed(const FName& LoggingKey)
    {
        return ShouldSuppressKey(LoggingKey);
    }

    // Clears all logging suppressions, allowing all messages to be logged again.
    FORCEINLINE static void ClearAllLoggingSuppressions()
    {
        SuppressedLoggingKeys.Reset();
    }

    /**
     * Snapshot of the current logging settings.
     *
     * @author Nils Bergemann
     */
    struct FGorgeousLoggingSettingsSnapshot
    {
        bool bEnableGorgeousMessageLog = true;
        FName MessageLogListingName = TEXT("Gorgeous Things");
        EGorgeousLoggingImportance MinMessageLogVerbosity = Logging_Information;
        bool bMirrorToOutputLog = true;
        bool bShowOnScreen = true;
    };

    /**
     * Retrieves a snapshot of the current logging settings from the configuration.
     *
     * @return A struct containing the current logging settings.
     */
    FORCEINLINE static FGorgeousLoggingSettingsSnapshot GetLoggingSettingsSnapshot()
    {
        FGorgeousLoggingSettingsSnapshot Snapshot;

        if (!GConfig)
        {
            return Snapshot;
        }

        const TCHAR* SettingsSection = TEXT("/Script/GorgeousCoreRuntime.GorgeousLoggingDeveloperSettings");

        bool bBoolValue = false;
        if (GConfig->GetBool(SettingsSection, TEXT("bEnableGorgeousMessageLog"), bBoolValue, GGameIni))
        {
            Snapshot.bEnableGorgeousMessageLog = bBoolValue;
        }
        if (GConfig->GetBool(SettingsSection, TEXT("bMirrorToOutputLog"), bBoolValue, GGameIni))
        {
            Snapshot.bMirrorToOutputLog = bBoolValue;
        }
        if (GConfig->GetBool(SettingsSection, TEXT("bShowOnScreen"), bBoolValue, GGameIni))
        {
            Snapshot.bShowOnScreen = bBoolValue;
        }

        if (FString ListingName; GConfig->GetString(SettingsSection, TEXT("MessageLogListingName"), ListingName, GGameIni) &&
            !ListingName.IsEmpty())
        {
            Snapshot.MessageLogListingName = FName(*ListingName);
        }

        // TEnumAsByte is saved as a string (enum name), not an integer
        if (FString VerbosityString; GConfig->GetString(SettingsSection, TEXT("MinMessageLogVerbosity"), VerbosityString, GGameIni) &&
            !VerbosityString.IsEmpty())
        {
            if (VerbosityString == TEXT("Logging_Information"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_Information;
            }
            else if (VerbosityString == TEXT("Logging_Success"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_Success;
            }
            else if (VerbosityString == TEXT("Logging_Warning"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_Warning;
            }
            else if (VerbosityString == TEXT("Logging_Error"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_Error;
            }
            else if (VerbosityString == TEXT("Logging_Fatal"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_Fatal;
            }
        }

        return Snapshot;
    }

    /**
     * Shows a toast notification with full customization.
     * Use this when you need complete control over the notification appearance.
     *
     * @param Params Struct containing all notification parameters.
     */
    FORCEINLINE static void ShowToastNotification(const ::FGorgeousToastParams& Params)
    {
        ::ShowToastNotification(Params);
    }

    /**
     * Shows a toast notification with minimal parameters.
     * Convenience function for quick error/warning notifications.
     *
     * @param Title The title of the notification.
     * @param Message The message to display.
     * @param ToastIconKind Icon style: 0 = information, 1 = warning, 2 = success, 3 = error.
     */
    FORCEINLINE static void ShowToastNotification(const FString& Title, const FString& Message, const int32 ToastIconKind = 3)
    {
        ::ShowToastNotification(Title, Message, ToastIconKind);
    }

	/**
	 * Logs a message with customizable parameters.
	 *
	 * @param Message The message to log.
	 * @param Importancy The importance of the message.
	 * @param LoggingKey A unique key to manage the log message.
	 * @param Duration The duration to display the message on screen.
	 * @param bPrintToScreen Whether to print the message to the screen.
	 * @param bPrintToLog Whether to print the message to the log.
	 * @param bOverrideLoggingIfPresent Whether to override an existing log message with the same key.
     * @param bShowAsToast Whether to show this log as a toast notification.
	 * @param WorldContextObject The world context object.
	 * @param Hyperlink Optional hyperlink data for the log message.
	 */
	FORCEINLINE static void LogMessage_Internal(UPARAM(ref) const FText& Message,
                                                const EGorgeousLoggingImportance Importancy, const FString& LoggingKey,
                                                const float Duration = 5.0f, const bool bPrintToScreen = true,
                                                const bool bPrintToLog = true, const bool bOverrideLoggingIfPresent = true,
                                                const bool bShowAsToast = false, const UObject* WorldContextObject = nullptr,
                                                const FGorgeousLogHyperlink* Hyperlink = nullptr)
	{
        const FName LoggingKeyName = LoggingKey.IsEmpty() ? NAME_None : FName(*LoggingKey);
        if (ShouldSuppressKey(LoggingKeyName))
        {
            return;
        }

        const auto [bEnableGorgeousMessageLog, MessageLogListingName, MinMessageLogVerbosity, bMirrorToOutputLog, bShowOnScreen] = GetLoggingSettingsSnapshot();
        const bool bAllowedBySettings = static_cast<int32>(Importancy) >= static_cast<int32>(MinMessageLogVerbosity);
        const bool bShouldSendToMessageLog = bEnableGorgeousMessageLog && bAllowedBySettings;
        // When a message is already being routed to the Gorgeous Message Log, suppress the parallel
        // UE_LOG call to avoid duplicating the same line in both output destinations.
        const bool bShouldPrintToLog = bPrintToLog && bMirrorToOutputLog && !bShouldSendToMessageLog;
        const bool bShouldPrintToScreen = bPrintToScreen && bShowOnScreen;

        const bool bShouldUseGameplayTags = !LoggingKey.IsEmpty() && CanUseGameplayTags();
        FGameplayTag GameplayLoggingKey = bShouldUseGameplayTags
            ? FGameplayTag::RequestGameplayTag(*LoggingKey, false)
            : FGameplayTag();

        // If this logging key is already present AND we're not overriding, skip logging to avoid duplication
        if (GameplayLoggingKey.IsValid() && LoggedMessages.Contains(GameplayLoggingKey) && !bOverrideLoggingIfPresent)
        {
            return;
        }

        if (GameplayLoggingKey.IsValid() && IsValid(WorldContextObject))
        {
            if (bOverrideLoggingIfPresent && LoggedMessages.Contains(GameplayLoggingKey))
            {
                LoggedMessages.Remove(GameplayLoggingKey);

                // Remove the on-screen message if it exists.  This requires a bit of a hack since AddOnScreenDebugMessage uses a hash.
                const uint64 UniqueKey = GetTypeHash(GameplayLoggingKey.GetTagName());
                GEngine->RemoveOnScreenDebugMessage(UniqueKey);
            }

            LoggedMessages.Add(GameplayLoggingKey, Message.ToString());


            // Schedule removal of the key after the duration.
            const UWorld* World = (GEngine && WorldContextObject)
                ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
                : nullptr;
            if (World)
            {
                FTimerHandle TimerHandle;
                World->GetTimerManager().SetTimer(
                    TimerHandle,
                    [GameplayLoggingKey]()
                    {
                        LoggedMessages.Remove(GameplayLoggingKey);
                    },
                    Duration,
                    false
                );
            }
        }

        const FString FinalMessage = FString::Printf(TEXT("[%s] %s"), *LoggingKey, *Message.ToString());

        switch (Importancy)
        {
            case Logging_Information:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Log, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Success:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Display, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Warning:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Warning, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Error:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Error, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Fatal:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Fatal, TEXT("%s"), *FinalMessage);
                }
                break;

            default:
                UE_LOG(LogGorgeousThings, Warning, TEXT("Unexpected importance level in LogMessage: %d"), static_cast<int32>(Importancy));
                break;
        }

        // Print to the screen if enabled
        if (bShouldPrintToScreen && GEngine)
        {
            FColor TextColor;
            switch (Importancy)
            {
                case Logging_Information:
                    TextColor = FColor::Blue;
                    break;

                case Logging_Success:
                    TextColor = FColor::Emerald;
                    break;

                case Logging_Warning:
                    TextColor = FColor::Yellow;
                    break;

                case Logging_Error:
                    TextColor = FColor::Red;
                    break;

                case Logging_Fatal:
                    TextColor = FColor::FromHex("#940000");
                    break;

                default:
                    TextColor = FColor::White;
                    break;
            }

            // Generate a unique key using GameplayLoggingKey when available, otherwise fall back to the logging key name
            const uint64 UniqueKey = GameplayLoggingKey.IsValid()
                ? GetTypeHash(GameplayLoggingKey.GetTagName())
                : GetTypeHash(LoggingKeyName);

            const FVector2D DefaultTextScale(1.0f, 1.0f);
            constexpr bool bNewerOnTop = true;

            // Add the message to the screen
            GEngine->AddOnScreenDebugMessage(
                UniqueKey,
                Duration,
                TextColor,
                FinalMessage,
                bNewerOnTop,
                DefaultTextScale
            );
        }

#if WITH_EDITOR
        if (bShouldSendToMessageLog)
        {
            FGorgeousLogEntry Entry;
            Entry.Message = FinalMessage;
            Entry.LoggingKey = LoggingKeyName;
            Entry.Importance = Importancy;
            Entry.Duration = Duration;
            Entry.bPrintToScreen = bShouldPrintToScreen;
            Entry.bPrintToLog = bShouldPrintToLog;
            Entry.WorldContextObject = WorldContextObject;
            if (Hyperlink && Hyperlink->IsValid())
            {
                Entry.Hyperlink = *Hyperlink;
            }
            GetGorgeousLogEntryDelegate().Broadcast(Entry);
        }

        if (bShowAsToast)
        {
            FGorgeousToastParams ToastParams;
            ToastParams.Title = LoggingKey.IsEmpty()
                ? MessageLogListingName.ToString()
                : LoggingKey;
            ToastParams.Message = Message.ToString();
            ToastParams.ToastIconKind = (Importancy == Logging_Error || Importancy == Logging_Fatal)
                ? 3
                : (Importancy == Logging_Warning ? 1 : (Importancy == Logging_Success ? 2 : 0));
            ToastParams.ExpireDuration = Duration;
            ::ShowToastNotification(ToastParams);
        }
#endif
	}
}