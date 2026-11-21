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
#include "Helpers/GorgeousLoggingHelper.h"
//<-------------------------------------------------------------------------->

#define GT_LOGGING_ENABLED 1

#if WITH_EDITOR
/**
 * Displays a tracked editor toast and returns its Slate notification item.
 *
 * @param ... The FGorgeousToastParams forwarded to ShowTrackedToastNotification.
 * @return The notification item, or null when no toast is shown.
 */
#define GT_TOAST_FULL_EX(...) (::ShowTrackedToastNotification(__VA_ARGS__))
#endif

#define GT_I_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_S_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_W_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_E_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_F_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_D_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_V_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_VV_LOG_ENABLED (GT_LOGGING_ENABLED && 1)

// Captures editor callsites; non-editor builds omit local source paths.
#if WITH_EDITOR
#define GT_LOG_SOURCE_LOCATION __FILE__, __LINE__, __FUNCTION__
#else
#define GT_LOG_SOURCE_LOCATION nullptr, 0, nullptr
#endif

/**
 * Controls defaults at the point where each logging invocation expands.
 * Override values before this header or inside a balanced local macro scope.
 * Information, display and verbose logs use GT_PRINT_TO_SCREEN_ON_INFO;
 * other severities use GT_PRINT_TO_SCREEN. Restore incoming definitions
 * before leaving a scope so unity neighbours retain their own settings.
 */
#ifndef GT_DURATION
#define GT_DURATION 4.0f
#endif

// Provides the world context for short logging and assertion macros.
#ifndef GT_WORLD_CONTEXT
#define GT_WORLD_CONTEXT nullptr
#endif

#ifndef GT_PRINT_TO_SCREEN
#define GT_PRINT_TO_SCREEN true
#endif

#ifndef GT_PRINT_TO_SCREEN_ON_INFO
#define GT_PRINT_TO_SCREEN_ON_INFO false
#endif

#ifndef GT_PRINT_TO_LOG
#define GT_PRINT_TO_LOG true
#endif

#ifndef GT_OVERRIDE_LOGGING_IF_PRESENT
#define GT_OVERRIDE_LOGGING_IF_PRESENT true
#endif

#ifndef GT_SHOW_AS_TOAST
#define GT_SHOW_AS_TOAST false
#endif


/**
 * Logs a message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param Importance The importance level of the log message.
 * @param Message The message to log.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 */
#define GT_LOG_MESSAGE_FULL_EX(LoggingKey, Importance, Message, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr) \
do { \
	GorgeousLogging::LogMessage_Internal( \
		GorgeousLogging::GT_LogToText(Message), \
		Importance, \
		FString(LoggingKey), \
		Duration, \
		bPrintToScreen, \
		bPrintToLog, \
		bOverrideLoggingIfPresent, \
		bShowAsToast, \
		WorldContextObject, \
		HyperlinkPtr, \
		GT_LOG_SOURCE_LOCATION \
	); \
} while (false)

/**
 * Logs a message with formatted arguments and customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param Importance The importance level of the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 * @param ... Variable arguments for the format string.
 */
#define GT_LOG_FORMAT_FULL_EX(LoggingKey, Importance, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	GT_LOG_MESSAGE_FULL_EX( \
		LoggingKey, \
		Importance, \
		FormattedMessage, \
		Duration, \
		bPrintToScreen, \
		bPrintToLog, \
		bOverrideLoggingIfPresent, \
		bShowAsToast, \
		WorldContextObject, \
		HyperlinkPtr \
	); \
} while (false)

#if GT_I_LOG_ENABLED
/**
 * Logs an information message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_I_LOG(LoggingKey, MessageFormat, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Information, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN_ON_INFO, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs an information message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param WorldContextObject The world context object.
 */
#define GT_I_LOG_FULL(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, WorldContextObject, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Information, MessageFormat, Duration, bPrintToScreen, bPrintToLog, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs an information message with formatted arguments and customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 */
#define GT_I_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Information, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_I_LOG(...) ((void)0)
#define GT_I_LOG_FULL(...) ((void)0)
#define GT_I_LOG_FULL_EX(...) ((void)0)
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
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Success, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a success message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param WorldContextObject The world context object.
 */
#define GT_S_LOG_FULL(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, WorldContextObject, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Success, MessageFormat, Duration, bPrintToScreen, bPrintToLog, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a success message with formatted arguments and customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 */
#define GT_S_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Success, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_S_LOG(...) ((void)0)
#define GT_S_LOG_FULL(...) ((void)0)
#define GT_S_LOG_FULL_EX(...) ((void)0)
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
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Warning, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a warning message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param WorldContextObject The world context object.
 */
#define GT_W_LOG_FULL(LoggingKey, MessageFormat, Duration, WorldContextObject, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Warning, MessageFormat, Duration, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a warning message with formatted arguments and customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 */
#define GT_W_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Warning, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_W_LOG(...) ((void)0)
#define GT_W_LOG_FULL(...) ((void)0)
#define GT_W_LOG_FULL_EX(...) ((void)0)
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
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Error, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs an error message if ensure failes.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param Condition The condition to check.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_E_LOG_ENSURE(LoggingKey, Condition, MessageFormat, ...) \
do { \
	const bool bGorgeousCondition = !!(Condition); \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	if (!bGorgeousCondition) \
	{ \
		GT_LOG_MESSAGE_FULL_EX(LoggingKey, Logging_Error, FormattedMessage, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr); \
	}\
	ensureMsgf(bGorgeousCondition, TEXT("%s"), *FormattedMessage); \
} while (false)

/**
 * Logs an error message if verify failes.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param Condition The condition to check.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_E_LOG_VERIFY(LoggingKey, Condition, MessageFormat, ...) \
do { \
	const bool bGorgeousCondition = !!(Condition); \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	if (!bGorgeousCondition) \
	{ \
		GT_LOG_MESSAGE_FULL_EX(LoggingKey, Logging_Error, FormattedMessage, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr); \
	}\
	verifyf(bGorgeousCondition, TEXT("%s"), *FormattedMessage); \
} while (false)

/**
 * Logs an error message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param WorldContextObject The world context object.
 */
#define GT_E_LOG_FULL(LoggingKey, MessageFormat, Duration, WorldContextObject, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Error, MessageFormat, Duration, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs an error message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 */
#define GT_E_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Error, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_E_LOG(...) ((void)0)
#define GT_E_LOG_ENSURE(...) ((void)0)
#define GT_E_LOG_VERIFY(...) ((void)0)
#define GT_E_LOG_FULL(...) ((void)0)
#define GT_E_LOG_FULL_EX(...) ((void)0)
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
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Fatal, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a fatal message if checkf failes.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param Condition The condition to check.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_F_LOG_CHECKF(LoggingKey, Condition, MessageFormat, ...) \
do { \
	const bool bGorgeousCondition = !!(Condition); \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	if (!bGorgeousCondition) \
	{ \
		GT_LOG_MESSAGE_FULL_EX(LoggingKey, Logging_Fatal, FormattedMessage, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr); \
	}\
	checkf(bGorgeousCondition, TEXT("%s"), *FormattedMessage); \
} while (false)

/**
 * Logs a fatal message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param WorldContextObject The world context object.
 */
#define GT_F_LOG_FULL(LoggingKey, MessageFormat, WorldContextObject, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Fatal, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a fatal message with customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 */
#define GT_F_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Fatal, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_F_LOG(...) ((void)0)
#define GT_F_LOG_CHECKF(...) ((void)0)
#define GT_F_LOG_FULL(...) ((void)0)
#define GT_F_LOG_FULL_EX(...) ((void)0)
#endif


#if GT_D_LOG_ENABLED
/**
 * Logs a display message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_D_LOG(LoggingKey, MessageFormat, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Display, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN_ON_INFO, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a display message with formatted arguments and customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 * @param ... Variable arguments for the format string.
 */
#define GT_D_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Display, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_D_LOG(...) ((void)0)
#define GT_D_LOG_FULL_EX(...) ((void)0)
#endif


#if GT_V_LOG_ENABLED
/**
 * Logs a verbose message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_V_LOG(LoggingKey, MessageFormat, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Verbose, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN_ON_INFO, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a verbose message with formatted arguments and customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 * @param ... Variable arguments for the format string.
 */
#define GT_V_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_Verbose, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_V_LOG(...) ((void)0)
#define GT_V_LOG_FULL_EX(...) ((void)0)
#endif


#if GT_VV_LOG_ENABLED
/**
 * Logs a very verbose message with formatted arguments and default parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_VV_LOG(LoggingKey, MessageFormat, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_VeryVerbose, MessageFormat, GT_DURATION, GT_PRINT_TO_SCREEN_ON_INFO, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, GT_WORLD_CONTEXT, nullptr, ##__VA_ARGS__); \
} while (false)

/**
 * Logs a very verbose message with formatted arguments and customizable parameters.
 *
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 * @param ... Variable arguments for the format string.
 */
#define GT_VV_LOG_FULL_EX(LoggingKey, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
do { \
	GT_LOG_FORMAT_FULL_EX(LoggingKey, Logging_VeryVerbose, MessageFormat, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
} while (false)

#else
#define GT_VV_LOG(...) ((void)0)
#define GT_VV_LOG_FULL_EX(...) ((void)0)
#endif

#undef GT_LOGGING_ENABLED
#undef GT_I_LOG_ENABLED
#undef GT_S_LOG_ENABLED
#undef GT_W_LOG_ENABLED
#undef GT_E_LOG_ENABLED
#undef GT_F_LOG_ENABLED
#undef GT_D_LOG_ENABLED
#undef GT_V_LOG_ENABLED
#undef GT_VV_LOG_ENABLED