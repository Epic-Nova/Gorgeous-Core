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
//<--------------------------=== Engine Includes ===------------------------->
#include "Misc/ConfigCacheIni.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
#if WITH_EDITOR
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

#if WITH_EDITOR
	TArray<FNotificationButtonInfo> Buttons;
	bool bUseLargeFont = true;
#endif
};

//<============================--- C++ Only ---=============================>
#pragma region C++ Only

/**
 * Immediately displays a toast notification without queuing.
 * Internal use - prefer ShowToastNotification which handles queuing.
 *
 * @param Params Struct containing all notification parameters.
 * @return The Slate notification item created for the toast.
 */
#if WITH_EDITOR
GORGEOUSCORERUNTIMEUTILITIES_API TSharedPtr<SNotificationItem> ShowToastNotificationImmediate(const FGorgeousToastParams& Params);

/**
 * Displays an actionable toast when Slate is ready and notifications are allowed.
 *
 * @param Params The notification text, lifetime, appearance and actions.
 * @return The dismissible item, or null while notifications are unavailable.
 */
GORGEOUSCORERUNTIMEUTILITIES_API TSharedPtr<SNotificationItem> ShowTrackedToastNotification(const FGorgeousToastParams& Params);
#endif

/**
 * Attempts to flush all queued toast notifications if the editor is ready.
 * Called automatically when notifications are queued.
 */
GORGEOUSCORERUNTIMEUTILITIES_API void TryFlushQueuedToastNotifications();

/**
 * Shows a toast notification with full customization.
 * Use this when you need complete control over the notification appearance.
 *
 * @param Params Struct containing all notification parameters.
 */
GORGEOUSCORERUNTIMEUTILITIES_API void ShowToastNotification(const FGorgeousToastParams& Params);

/**
 * Shows a toast notification with minimal parameters.
 * Convenience function for quick error/warning notifications.
 *
 * @param Title The title of the notification.
 * @param Message The message to display.
 * @param ToastIconKind Icon style: 0 = information, 1 = warning, 2 = success, 3 = error.
 */
GORGEOUSCORERUNTIMEUTILITIES_API void ShowToastNotification(const FString& Title, const FString& Message, const int32 ToastIconKind = 3);
//<------------------------------------------------------------------------->
#pragma endregion C++ Only

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
    GORGEOUSCORERUNTIMEUTILITIES_API TSet<FName>& GetSuppressedLoggingKeys();

    // Serializes suppression access across worker-thread diagnostics and editor commands.
    GORGEOUSCORERUNTIMEUTILITIES_API FCriticalSection& GetLoggingSuppressionLock();

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
        FScopeLock Lock(&GetLoggingSuppressionLock());
        return LoggingKey != NAME_None && GetSuppressedLoggingKeys().Contains(LoggingKey);
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

        {
            FScopeLock Lock(&GetLoggingSuppressionLock());
            if (GetSuppressedLoggingKeys().Contains(LoggingKey) == bShouldSuppress)
            {
                return;
            }
            if (bShouldSuppress) GetSuppressedLoggingKeys().Add(LoggingKey);
            else GetSuppressedLoggingKeys().Remove(LoggingKey);
        }

        // This low-level sink must not recurse through the logging macros.
        UE_LOG(LogGorgeousThings, Log, TEXT("Logging for key '%s' has been %s."), *LoggingKey.ToString(),
            bShouldSuppress ? TEXT("suppressed") : TEXT("unsuppressed"));
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
        FScopeLock Lock(&GetLoggingSuppressionLock());
        GetSuppressedLoggingKeys().Reset();
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

    // Serialized enum values remain stable; filtering uses semantic severity instead of ordinal values.
    FORCEINLINE int32 GetImportanceRank(const EGorgeousLoggingImportance Importance)
    {
        switch (Importance)
        {
            case Logging_VeryVerbose: return -2;
            case Logging_Verbose: return -1;
            case Logging_Information: return 0;
            case Logging_Display:
            case Logging_Success: return 1;
            case Logging_Warning: return 2;
            case Logging_Error: return 3;
            case Logging_Fatal: return 4;
            default: return -3;
        }
    }

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
            else if (VerbosityString == TEXT("Logging_Verbose"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_Verbose;
            }
            else if (VerbosityString == TEXT("Logging_VeryVerbose"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_VeryVerbose;
            }
            else if (VerbosityString == TEXT("Logging_Display"))
            {
                Snapshot.MinMessageLogVerbosity = Logging_Display;
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
     * @param SourceFile Optional editor C++ macro callsite; omitted by source-less APIs and packaged macro calls.
     * @param SourceLine One-based source line, or zero when no origin was supplied.
     * @param SourceFunction Optional emitting C++ function for the metadata tooltip.
	 */
	FORCEINLINE static void LogMessage_Internal(UPARAM(ref) const FText& Message,
                                                const EGorgeousLoggingImportance Importancy, const FString& LoggingKey,
                                                const float Duration = 5.0f, const bool bPrintToScreen = true,
                                                const bool bPrintToLog = true, const bool bOverrideLoggingIfPresent = true,
                                                const bool bShowAsToast = false, const UObject* WorldContextObject = nullptr,
                                                const FGorgeousLogHyperlink* Hyperlink = nullptr,
                                                const ANSICHAR* SourceFile = nullptr, const int32 SourceLine = 0,
                                                const ANSICHAR* SourceFunction = nullptr)
	{
        // Fatal is a control-flow contract, even when output is disabled or the key is suppressed.
        if (Importancy == Logging_Fatal)
        {
            UE_LOG(LogGorgeousThings, Fatal, TEXT("[%s] %s"), *LoggingKey, *Message.ToString());
        }

        const FName LoggingKeyName = LoggingKey.IsEmpty() ? NAME_None : FName(*LoggingKey);
        if (ShouldSuppressKey(LoggingKeyName))
        {
            return;
        }

        const auto [bEnableGorgeousMessageLog, MessageLogListingName, MinMessageLogVerbosity, bMirrorToOutputLog, bShowOnScreen] = GetLoggingSettingsSnapshot();
        const bool bAllowedBySettings = GetImportanceRank(Importancy) >= GetImportanceRank(MinMessageLogVerbosity);
        const bool bShouldSendToMessageLog = WITH_EDITOR && bEnableGorgeousMessageLog && bAllowedBySettings;
        // Message Log and Output Log are independent destinations; packaged builds have no Message Log.
        const bool bShouldPrintToLog = bPrintToLog && bMirrorToOutputLog;
        const bool bShouldPrintToScreen = bPrintToScreen && bShowOnScreen;

        // GameplayTag/world bookkeeping is game-thread state. Worker logs still reach the textual sinks.
        const bool bShouldUseGameplayTags = IsInGameThread() && !LoggingKey.IsEmpty() && CanUseGameplayTags();
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
                if (GEngine) { GEngine->RemoveOnScreenDebugMessage(UniqueKey); }
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

            case Logging_Display:
            case Logging_Success:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Display, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Verbose:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Verbose, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_VeryVerbose:
                if (bShouldPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, VeryVerbose, TEXT("%s"), *FinalMessage);
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
                case Logging_Display:
                case Logging_Verbose:
                case Logging_VeryVerbose:
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
                : (LoggingKeyName.IsNone() ? uint64(-1) : uint64(GetTypeHash(LoggingKeyName)));

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

        // Runtime subscribers (including the optional in-game widget) share this feed.
        if (bShouldSendToMessageLog || bShouldPrintToLog || bShouldPrintToScreen)
        {
            FGorgeousLogEntry Entry;
            Entry.Message = FinalMessage;
            Entry.LoggingKey = LoggingKeyName;
            Entry.Importance = Importancy;
            Entry.Duration = Duration;
            Entry.bPrintToScreen = bShouldPrintToScreen;
            Entry.bPrintToLog = bShouldPrintToLog;
            Entry.bPrintToMessageLog = bShouldSendToMessageLog;
            Entry.WorldContextObject = WorldContextObject;
#if WITH_EDITOR
            if (SourceFile && SourceLine > 0)
            {
                Entry.SourceFile = UTF8_TO_TCHAR(SourceFile);
                Entry.SourceLine = SourceLine;
                Entry.SourceFunction = SourceFunction ? UTF8_TO_TCHAR(SourceFunction) : TEXT("");
            }
#endif
            if (Hyperlink && Hyperlink->IsValid())
            {
                Entry.Hyperlink = *Hyperlink;
            }
            GetGorgeousLogEntryDelegate().Broadcast(Entry);
        }

#if WITH_EDITOR
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
