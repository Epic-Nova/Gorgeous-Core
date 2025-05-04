// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
#include "GorgeousCoreRuntimeUtilitiesLogging.h"
//<-------------------------------------------------------------------------->

namespace GorgeousLogging
{
    // The messages that were logged and are currently on cooldown with their gameplay tag as key.
    static TMap<FGameplayTag, FString> LoggedMessages;
    
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
	 * @param WorldContextObject The world context object.
	 */
	FORCEINLINE static void LogMessage_Internal(UPARAM(ref) const FString& Message,
	                                            const EGorgeousLoggingImportance Importancy, const FString& LoggingKey,
	                                            const float Duration = 5.0f, const bool bPrintToScreen = true,
	                                            const bool bPrintToLog = true, const bool bOverrideLoggingIfPresent = true,
	                                            const UObject* WorldContextObject = nullptr)
	{
	    FGameplayTag GameplayLoggingKey = FGameplayTag::RequestGameplayTag(*LoggingKey, false);

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

            LoggedMessages.Add(GameplayLoggingKey, Message);


            // Schedule removal of the key after the duration.
            if (const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
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
        
        const FString FinalMessage = FString::Printf(TEXT("[%s] %s"), *LoggingKey, *Message);
        
        switch (Importancy)
        {
            case Logging_Information:
                if (bPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Log, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Success:
                if (bPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Display, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Warning:
                if (bPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Warning, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Error:
                if (bPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Error, TEXT("%s"), *FinalMessage);
                }
                break;

            case Logging_Fatal:
                if (bPrintToLog)
                {
                    UE_LOG(LogGorgeousThings, Fatal, TEXT("%s"), *FinalMessage);
                }
                break;

            default:
                UE_LOG(LogGorgeousThings, Warning, TEXT("Unexpected importance level in LogMessage: %d"), static_cast<int32>(Importancy));
                break;
        }

        // Print to the screen if enabled
	    check(GEngine);
        if (bPrintToScreen && GEngine)
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

            // Generate a unique key using GameplayLoggingKey
            const uint64 UniqueKey = GetTypeHash(GameplayLoggingKey.GetTagName());
            
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
	}
}
