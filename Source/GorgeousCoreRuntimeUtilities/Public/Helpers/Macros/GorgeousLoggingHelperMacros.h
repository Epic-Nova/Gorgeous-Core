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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

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
	GT_LOG_FORMAT_FULL_EX(Logging_Information, MessageFormat, LoggingKey, 5.0f, false, true, true, false, nullptr, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Information, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, false, WorldContextObject, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Success, MessageFormat, LoggingKey, 2.5f, true, true, true, false, nullptr, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Success, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, false, WorldContextObject, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Warning, MessageFormat, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Warning, MessageFormat, LoggingKey, Duration, true, true, true, false, WorldContextObject, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Error, MessageFormat, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr, ##__VA_ARGS__); \
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
	if(Condition) \
	{ \
		FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
		GT_LOG_MESSAGE_FULL_EX(Logging_Error, FormattedMessage, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr); \
		ensureMsgf(Condition, TEXT("%s"), *FormattedMessage); \
	}\
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
	if(Condition) \
	{ \
		FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
		GT_LOG_MESSAGE_FULL_EX(Logging_Error, FormattedMessage, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr); \
		verifyf(Condition, TEXT("%s"), *FormattedMessage); \
	}\
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
	GT_LOG_FORMAT_FULL_EX(Logging_Error, MessageFormat, LoggingKey, Duration, true, true, true, false, WorldContextObject, nullptr, ##__VA_ARGS__); \
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
	GT_LOG_FORMAT_FULL_EX(Logging_Fatal, MessageFormat, LoggingKey, 0.0f, true, true, true, false, nullptr, nullptr, ##__VA_ARGS__); \
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
	if(Condition) \
	{ \
		FString FormattedMessage = FString::Printf(MessageFormat, ##__VA_ARGS__); \
		GT_LOG_MESSAGE_FULL_EX(Logging_Fatal, FormattedMessage, LoggingKey, 0.0f, true, true, true, false, nullptr, nullptr); \
		checkf(Condition, TEXT("%s"), *FormattedMessage); \
	}\
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
	GT_LOG_FORMAT_FULL_EX(Logging_Fatal, MessageFormat, LoggingKey, 0.0f, true, true, true, false, WorldContextObject, nullptr, ##__VA_ARGS__); \
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