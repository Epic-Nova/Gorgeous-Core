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

#define GT_LOGGING_ENABLED 1

#define GT_I_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_S_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_W_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_E_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_F_LOG_ENABLED (GT_LOGGING_ENABLED && 1)

#if GT_I_LOG_ENABLED
/**
 * Logs an information message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_I_LOG(LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		5.0f, \
		true, \
		true, \
		nullptr \
	); \
}

/**
 * Logs an information message with customizable parameters.
 *
 * @param Message The message to log.
 * @param LoggingKey A unique key to manage the log message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param WorldContextObject The world context object.
 */
#define GT_I_LOG_FULL(MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, WorldContextObject, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		Duration, \
		bPrintToScreen, \
		bPrintToLog, \
		WorldContextObject \
	); \
}
#else
#define GT_I_LOG(...) ((void)0)
#define GT_I_LOG_FULL(...) ((void)0)
#endif


#if GT_S_LOG_ENABLED
/**
 * Logs a success message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_S_LOG(LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		2.0f, \
		true, \
		true, \
		nullptr \
	); \
}

/**
 * Logs a success message with customizable parameters.
 *
 * @param Message The message to log.
 * @param LoggingKey A unique key to manage the log message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param WorldContextObject The world context object.
 */
#define GT_S_LOG_FULL(MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, WorldContextObject, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		Duration, \
		bPrintToScreen, \
		bPrintToLog, \
		WorldContextObject \
	); \
}
#else
#define GT_S_LOG(...) ((void)0)
#define GT_S_LOG_FULL(...) ((void)0)
#endif


#if GT_W_LOG_ENABLED
/**
 * Logs a warning message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_W_LOG(LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		2.0f, \
		nullptr \
	); \
}

/**
 * Logs a warning message with customizable parameters.
 *
 * @param Message The message to log.
 * @param LoggingKey A unique key to manage the log message.
 * @param Duration The duration to display the message on screen.
 * @param WorldContextObject The world context object.
 */
#define GT_W_LOG_FULL(MessageFormat, LoggingKey, Duration, WorldContextObject, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		Duration, \
		WorldContextObject \
	); \
}
#else
#define GT_W_LOG(...) ((void)0)
#define GT_W_LOG_FULL(...) ((void)0)
#endif


#if GT_E_LOG_ENABLED
/**
 * Logs an error message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_E_LOG(LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogErrorMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		2.0f, \
		nullptr \
	); \
}

/**
 * Logs an error message with customizable parameters.
 *
 * @param Message The message to log.
 * @param LoggingKey A unique key to manage the log message.
 * @param Duration The duration to display the message on screen.
 * @param WorldContextObject The world context object.
 */
#define GT_E_LOG_FULL(MessageFormat, LoggingKey, Duration, WorldContextObject, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogErrorMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		Duration, \
		WorldContextObject \
	); \
}
#else
#define GT_E_LOG(...) ((void)0)
#define GT_E_LOG_FULL(...) ((void)0)
#endif


#if GT_F_LOG_ENABLED
/**
 * Logs a fatal message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_F_LOG(LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogFatalMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		nullptr \
	); \
}

/**
 * Logs a fatal message with customizable parameters.
 *
 * @param Message The message to log.
 * @param LoggingKey A unique key to manage the log message.
 * @param WorldContextObject The world context object.
 */
#define GT_F_LOG_FULL(MessageFormat, LoggingKey, WorldContextObject, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	UGorgeousLoggingBlueprintFunctionLibrary::LogFatalMessage( \
		FormattedMessage, \
		FString(LoggingKey), \
		WorldContextObject \
	); \
}
#else
#define GT_F_LOG(...) ((void)0)
#define GT_F_LOG_FULL(...) ((void)0)
#endif
