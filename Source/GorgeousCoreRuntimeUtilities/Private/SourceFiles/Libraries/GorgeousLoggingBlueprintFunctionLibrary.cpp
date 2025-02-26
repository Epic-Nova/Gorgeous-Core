// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#include "GorgeousLoggingBlueprintFunctionLibrary.h"

#include "GorgeousCoreRuntimeUtilitiesLogging.h"

static TMap<FGameplayTag, FString> LoggedMessages;

void UGorgeousLoggingBlueprintFunctionLibrary::LogMessage(const FString& Message, const EGorgeousLoggingImportancy Importancy,
	const FString LoggingKey, const float Duration, const bool bPrintToScreen, const bool bPrintToLog, const bool bOverrideLoggingIfPresent, UObject* WorldContextObject)
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

void UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage(const FString Message, const FString LoggingKey,
    const float Duration, const bool bPrintToScreen, const bool bPrintToLog, UObject* WorldContextObject)
{
    LogMessage(Message, Logging_Information, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage(const FString Message, const FString LoggingKey,
    const float Duration, const bool bPrintToScreen, const bool bPrintToLog, UObject* WorldContextObject)
{
    LogMessage(Message, Logging_Success, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(const FString Message, const FString LoggingKey,
    const float Duration, UObject* WorldContextObject)
{
    LogMessage(Message, Logging_Warning, LoggingKey, Duration, true, true, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogErrorMessage(const FString Message, const FString LoggingKey,
    const float Duration, UObject* WorldContextObject)
{
    LogMessage(Message, Logging_Error, LoggingKey, Duration, true, true, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogFatalMessage(const FString Message, const FString LoggingKey,
    UObject* WorldContextObject)
{
    LogMessage(Message, Logging_Fatal, LoggingKey, 0.0f, true, true, true, WorldContextObject);
}
