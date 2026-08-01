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
#include "GorgeousLoggingBlueprintFunctionLibrary.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/GorgeousLoggingHelper.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousLoggingBlueprintFunctionLibrary Implementation
//=============================================================================

void UGorgeousLoggingBlueprintFunctionLibrary::LogMessage(const FText Message,
    const EGorgeousLoggingImportance Importance, const FString LoggingKey, const float Duration,
    const bool bPrintToScreen, const bool bPrintToLog, const bool bOverrideLoggingIfPresent, const bool bShowAsToast,
    UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Importance, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage(const FText Message, const FString LoggingKey,
                                                                     const float Duration, const bool bPrintToScreen, const bool bPrintToLog, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Information, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, false, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage(const FText Message, const FString LoggingKey,
    const float Duration, const bool bPrintToScreen, const bool bPrintToLog, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Success, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, false, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(const FText Message, const FString LoggingKey,
    const float Duration, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Warning, LoggingKey, Duration, true, true, true, false, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogErrorMessage(const FText Message, const FString LoggingKey,
    const float Duration, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Error, LoggingKey, Duration, true, true, true, false, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogFatalMessage(const FText Message, const FString LoggingKey,
    UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Fatal, LoggingKey, 0.0f, true, true, true, false, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::SetLoggingKeySuppressed(const FName LoggingKey, const bool bShouldSuppress)
{
    GorgeousLogging::SetLoggingKeySuppressed(LoggingKey, bShouldSuppress);
}

bool UGorgeousLoggingBlueprintFunctionLibrary::IsLoggingKeySuppressed(const FName LoggingKey)
{
    return GorgeousLogging::IsLoggingKeySuppressed(LoggingKey);
}

void UGorgeousLoggingBlueprintFunctionLibrary::ClearAllLoggingSuppressions()
{
    GorgeousLogging::ClearAllLoggingSuppressions();
}
