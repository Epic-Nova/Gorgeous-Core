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
#include "GorgeousLoggingBlueprintFunctionLibrary.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "Helpers/GorgeousLoggingHelper.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousLoggingBlueprintFunctionLibrary Implementation
//=============================================================================

void UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage(const FString Message, const FString LoggingKey,
                                                                     const float Duration, const bool bPrintToScreen, const bool bPrintToLog, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Information, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage(const FString Message, const FString LoggingKey,
    const float Duration, const bool bPrintToScreen, const bool bPrintToLog, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Success, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(const FString Message, const FString LoggingKey,
    const float Duration, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Warning, LoggingKey, Duration, true, true, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogErrorMessage(const FString Message, const FString LoggingKey,
    const float Duration, UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Error, LoggingKey, Duration, true, true, true, WorldContextObject);
}

void UGorgeousLoggingBlueprintFunctionLibrary::LogFatalMessage(const FString Message, const FString LoggingKey,
    UObject* WorldContextObject)
{
    GorgeousLogging::LogMessage_Internal(Message, Logging_Fatal, LoggingKey, 0.0f, true, true, true, WorldContextObject);
}
