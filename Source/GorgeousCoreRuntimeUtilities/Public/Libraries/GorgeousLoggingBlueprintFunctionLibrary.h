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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ModuleCore/GorgeousCoreRuntimeUtilitiesEnums.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousLoggingBlueprintFunctionLibrary.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Logging Blueprint Function Library
| Functional Name: UGorgeousLoggingBlueprintFunctionLibrary
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Function Library providing logging functionalities from the Logging Helper
| exposed to Blueprints. Allows logging messages with different importance levels,
| managing logging keys, and controlling suppression of log messages.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousLoggingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    /**
     * Logs a message with specified importance.
     *
     * @param Message The message to log.
     * @param Importance The importance level of the log message.
     * @param LoggingKey A unique key to manage the log message.
     * @param Duration The duration to display the message on screen.
     * @param bPrintToScreen Whether to print the message to the screen.
     * @param bPrintToLog Whether to print the message to the log.
     * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
     * @param bShowAsToast Whether to show the message as a toast notification.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogMessage(const FText Message, const EGorgeousLoggingImportance Importance, const FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, const bool bOverrideLoggingIfPresent = true, const bool bShowAsToast = false, UObject* WorldContextObject = nullptr);
    
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
    static void LogInformationMessage(const FText Message, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, UObject* WorldContextObject = nullptr);

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
    static void LogSuccessMessage(const FText Message, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, UObject* WorldContextObject = nullptr);

    /**
     * Logs a warning message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param Duration The duration to display the message on screen.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogWarningMessage(const FText Message, FString LoggingKey, const float Duration = 2.0f, UObject* WorldContextObject = nullptr);

    /**
     * Logs an error message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param Duration The duration to display the message on screen.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogErrorMessage(const FText Message, FString LoggingKey, const float Duration = 2.0f, UObject* WorldContextObject = nullptr);

    /**
     * Logs a fatal message.
     *
     * @param Message The message to log.
     * @param LoggingKey A unique key to manage the log message.
     * @param WorldContextObject The world context object.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
    static void LogFatalMessage(const FText Message, FString LoggingKey, UObject* WorldContextObject = nullptr);

    /**
     * Sets whether a logging key should be suppressed.
     * 
     * @param LoggingKey The logging key to suppress or unsuppress.
     * @param bShouldSuppress True to suppress the logging key, false to unsuppress.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging")
    static void SetLoggingKeySuppressed(const FName LoggingKey, const bool bShouldSuppress);

    /**
     * Checks if a logging key is currently suppressed.
     * 
     * @param LoggingKey The logging key to check.
     * @return True if the logging key is suppressed, false otherwise.
     */
    UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Logging")
    static bool IsLoggingKeySuppressed(const FName LoggingKey);

    /** Clears all runtime logging suppressions. */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging")
    static void ClearAllLoggingSuppressions();
};

using UGT_Logging_FL = UGorgeousLoggingBlueprintFunctionLibrary;