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
#include "Helpers/GorgeousLoggingHelper.h"
//<-------------------------------------------------------------------------->

#define GT_LOGGING_ENABLED 1

#define GT_I_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_S_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_W_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_E_LOG_ENABLED (GT_LOGGING_ENABLED && 1)
#define GT_F_LOG_ENABLED (GT_LOGGING_ENABLED && 1)

/**
 * Default parameters for the logging macros. These can be overridden by defining them before including this header.
 */
#ifndef GT_DURATION
#define GT_DURATION 4.0f
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
 * @param Importance The importance level of the log message.
 * @param Message The message to log.
 * @param LoggingKey A unique key to manage the log message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 */
#define GT_LOG_MESSAGE_FULL_EX(Importance, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr) \
{ \
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
		HyperlinkPtr \
	); \
}

/**
 * Logs a message with formatted arguments and customizable parameters.
 *
 * @param Importance The importance level of the log message.
 * @param MessageFormat The format string for the message.
 * @param LoggingKey A unique key to manage the log message.
 * @param Duration The duration to display the message on screen.
 * @param bPrintToScreen Whether to print the message to the screen.
 * @param bPrintToLog Whether to print the message to the log.
 * @param bOverrideLoggingIfPresent Whether to override existing log messages with the same key.
 * @param bShowAsToast Whether to show the message as a toast notification.
 * @param WorldContextObject The world context object.
 * @param HyperlinkPtr Optional hyperlink associated with the log message.
 * @param ... Variable arguments for the format string.
 */
#define GT_LOG_FORMAT_FULL_EX(Importance, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	GT_LOG_MESSAGE_FULL_EX( \
		Importance, \
		FormattedMessage, \
		LoggingKey, \
		Duration, \
		bPrintToScreen, \
		bPrintToLog, \
		bOverrideLoggingIfPresent, \
		bShowAsToast, \
		WorldContextObject, \
		HyperlinkPtr \
	); \
}

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
	GT_LOG_FORMAT_FULL_EX(Logging_Information, MessageFormat, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN_ON_INFO, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Information, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Information, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Success, MessageFormat, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Success, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Success, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Warning, MessageFormat, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Warning, MessageFormat, LoggingKey, Duration, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Warning, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Error, MessageFormat, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr, ##__VA_ARGS__); \
}

/**
 * Logs an error message if ensure failes.
 * 
 * @param Condition The condition to check.
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_E_LOG_ENSURE(Condition, LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	if(!Condition) \
	{ \
		GT_LOG_MESSAGE_FULL_EX(Logging_Error, FormattedMessage, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr); \
	}\
	ensureMsgf(Condition, TEXT("%s"), *FormattedMessage); \
} \

/**
 * Logs an error message if verify failes.
 * 
 * @param Condition The condition to check.
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_E_LOG_VERIFY(Condition, LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	if(!Condition) \
	{ \
		GT_LOG_MESSAGE_FULL_EX(Logging_Error, FormattedMessage, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr); \
	}\
	verifyf(Condition, TEXT("%s"), *FormattedMessage); \
} \

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
	GT_LOG_FORMAT_FULL_EX(Logging_Error, MessageFormat, LoggingKey, Duration, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Error, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Fatal, MessageFormat, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr, ##__VA_ARGS__); \
}

/**
 * Logs a fatal message if checkf failes.
 * 
 * @param Condition The condition to check.
 * @param LoggingKey A unique key to manage the log message.
 * @param MessageFormat The format string for the message.
 * @param ... Variable arguments for the format string.
 */
#define GT_F_LOG_CHECKF(Condition, LoggingKey, MessageFormat, ...) \
{ \
	FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
	if(!Condition) \
	{ \
		GT_LOG_MESSAGE_FULL_EX(Logging_Fatal, FormattedMessage, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, nullptr, nullptr); \
	}\
	checkf(Condition, TEXT("%s"), *FormattedMessage); \
} \

/**
 * Logs a fatal message with customizable parameters.
 *
 * @param Message The message to log.
 * @param LoggingKey A unique key to manage the log message.
 * @param WorldContextObject The world context object.
 */
#define GT_F_LOG_FULL(MessageFormat, LoggingKey, WorldContextObject, ...) \
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Fatal, MessageFormat, LoggingKey, GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG, GT_OVERRIDE_LOGGING_IF_PRESENT, GT_SHOW_AS_TOAST, WorldContextObject, nullptr, ##__VA_ARGS__); \
}

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
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Fatal, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

#else
#define GT_F_LOG(...) ((void)0)
#define GT_F_LOG_CHECKF(...) ((void)0)
#define GT_F_LOG_FULL(...) ((void)0)
#define GT_F_LOG_FULL_EX(...) ((void)0)
#endif

#undef GT_LOGGING_ENABLED
#undef GT_I_LOG_ENABLED
#undef GT_S_LOG_ENABLED
#undef GT_W_LOG_ENABLED
#undef GT_E_LOG_ENABLED
#undef GT_F_LOG_ENABLED