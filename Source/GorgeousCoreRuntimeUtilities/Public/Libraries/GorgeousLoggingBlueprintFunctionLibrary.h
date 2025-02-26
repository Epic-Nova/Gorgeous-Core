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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousLoggingBlueprintFunctionLibrary.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Provides a set of blueprint callable functions for logging messages with different importances.
 *
 * Key features include:
 * - Logging messages with customizable importances, durations, and output targets.
 * - Convenience functions for logging information, success, warning, error, and fatal messages.
 * - Support for logging keys to manage and override existing log messages.
 *
 * @note This class provides a centralized and easy-to-use logging system for blueprints.
 */
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousLoggingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

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
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogMessage(UPARAM(ref) const FString& Message, EGorgeousLoggingImportancy Importancy, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, bool bOverrideLoggingIfPresent = true, UObject* WorldContextObject = nullptr);

    /**
     * Logs an information message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param Duration The duration to display the message on screen.
     * @param bPrintToScreen Whether to print the message to the screen.
     * @param bPrintToLog Whether to print the message to the log.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogInformationMessage(const FString Message, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, UObject* WorldContextObject = nullptr);

    /**
     * Logs a success message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param Duration The duration to display the message on screen.
     * @param bPrintToScreen Whether to print the message to the screen.
     * @param bPrintToLog Whether to print the message to the log.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogSuccessMessage(const FString Message, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, UObject* WorldContextObject = nullptr);

    /**
     * Logs a warning message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param Duration The duration to display the message on screen.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogWarningMessage(const FString Message, FString LoggingKey, const float Duration = 2.0f, UObject* WorldContextObject = nullptr);

    /**
     * Logs an error message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param Duration The duration to display the message on screen.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogErrorMessage(const FString Message, FString LoggingKey, const float Duration = 2.0f, UObject* WorldContextObject = nullptr);

    /**
     * Logs a fatal message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogFatalMessage(const FString Message, FString LoggingKey, UObject* WorldContextObject = nullptr);
};

