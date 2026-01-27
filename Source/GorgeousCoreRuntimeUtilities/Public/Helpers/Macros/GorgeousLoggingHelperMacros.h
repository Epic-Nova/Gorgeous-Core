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

namespace GorgeousLogging
{
	FORCEINLINE FText GT_LogToText(const FText& In)
	{
		return In;
	}

	FORCEINLINE FText GT_LogToText(const FString& In)
	{
		return FText::FromString(In);
	}

	FORCEINLINE FText GT_LogToText(const TCHAR* In)
	{
		return FText::FromString(In);
	}
}

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
	GT_LOG_FORMAT_FULL_EX(Logging_Information, MessageFormat, LoggingKey, 5.0f, true, true, true, false, nullptr, nullptr, ##__VA_ARGS__); \
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

#define GT_I_LOG_MESSAGE(Message, LoggingKey) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Information, Message, LoggingKey, 5.0f, true, true, true, false, nullptr, nullptr); \
}

#define GT_I_LOG_MESSAGE_FULL(Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, WorldContextObject) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Information, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, false, WorldContextObject, nullptr); \
}

#define GT_I_LOG_FULL_EX(MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Information, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

#define GT_I_LOG_MESSAGE_FULL_EX(Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Information, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr); \
}
#else
#define GT_I_LOG(...) ((void)0)
#define GT_I_LOG_FULL(...) ((void)0)
#define GT_I_LOG_MESSAGE(...) ((void)0)
#define GT_I_LOG_MESSAGE_FULL(...) ((void)0)
#define GT_I_LOG_FULL_EX(...) ((void)0)
#define GT_I_LOG_MESSAGE_FULL_EX(...) ((void)0)
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
	GT_LOG_FORMAT_FULL_EX(Logging_Success, MessageFormat, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr, ##__VA_ARGS__); \
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

#define GT_S_LOG_MESSAGE(Message, LoggingKey) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Success, Message, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr); \
}

#define GT_S_LOG_MESSAGE_FULL(Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, WorldContextObject) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Success, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, true, false, WorldContextObject, nullptr); \
}

#define GT_S_LOG_FULL_EX(MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Success, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

#define GT_S_LOG_MESSAGE_FULL_EX(Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Success, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr); \
}
#else
#define GT_S_LOG(...) ((void)0)
#define GT_S_LOG_FULL(...) ((void)0)
#define GT_S_LOG_MESSAGE(...) ((void)0)
#define GT_S_LOG_MESSAGE_FULL(...) ((void)0)
#define GT_S_LOG_FULL_EX(...) ((void)0)
#define GT_S_LOG_MESSAGE_FULL_EX(...) ((void)0)
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

#define GT_W_LOG_MESSAGE(Message, LoggingKey) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Warning, Message, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr); \
}

#define GT_W_LOG_MESSAGE_FULL(Message, LoggingKey, Duration, WorldContextObject) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Warning, Message, LoggingKey, Duration, true, true, true, false, WorldContextObject, nullptr); \
}

#define GT_W_LOG_FULL_EX(MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Warning, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

#define GT_W_LOG_MESSAGE_FULL_EX(Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Warning, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr); \
}
#else
#define GT_W_LOG(...) ((void)0)
#define GT_W_LOG_FULL(...) ((void)0)
#define GT_W_LOG_MESSAGE(...) ((void)0)
#define GT_W_LOG_MESSAGE_FULL(...) ((void)0)
#define GT_W_LOG_FULL_EX(...) ((void)0)
#define GT_W_LOG_MESSAGE_FULL_EX(...) ((void)0)
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
		ensureMsgf(Condition, *FormattedMessage); \
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
		verifyMsgf(Condition, *FormattedMessage); \
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

#define GT_E_LOG_MESSAGE(Message, LoggingKey) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Error, Message, LoggingKey, 2.0f, true, true, true, false, nullptr, nullptr); \
}

#define GT_E_LOG_MESSAGE_FULL(Message, LoggingKey, Duration, WorldContextObject) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Error, Message, LoggingKey, Duration, true, true, true, false, WorldContextObject, nullptr); \
}

#define GT_E_LOG_FULL_EX(MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Error, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

#define GT_E_LOG_MESSAGE_FULL_EX(Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Error, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr); \
}
#else
#define GT_E_LOG(...) ((void)0)
#define GT_E_LOG_ENSURE(...) ((void)0)
#define GT_E_LOG_VERIFY(...) ((void)0)
#define GT_E_LOG_FULL(...) ((void)0)
#define GT_E_LOG_MESSAGE(...) ((void)0)
#define GT_E_LOG_MESSAGE_FULL(...) ((void)0)
#define GT_E_LOG_FULL_EX(...) ((void)0)
#define GT_E_LOG_MESSAGE_FULL_EX(...) ((void)0)
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

#define GT_F_LOG_MESSAGE(Message, LoggingKey) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Fatal, Message, LoggingKey, 0.0f, true, true, true, false, nullptr, nullptr); \
}

#define GT_F_LOG_MESSAGE_FULL(Message, LoggingKey, WorldContextObject) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Fatal, Message, LoggingKey, 0.0f, true, true, true, false, WorldContextObject, nullptr); \
}

#define GT_F_LOG_FULL_EX(MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ...) \
{ \
	GT_LOG_FORMAT_FULL_EX(Logging_Fatal, MessageFormat, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr, ##__VA_ARGS__); \
}

#define GT_F_LOG_MESSAGE_FULL_EX(Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr) \
{ \
	GT_LOG_MESSAGE_FULL_EX(Logging_Fatal, Message, LoggingKey, Duration, bPrintToScreen, bPrintToLog, bOverrideLoggingIfPresent, bShowAsToast, WorldContextObject, HyperlinkPtr); \
}
#else
#define GT_F_LOG(...) ((void)0)
#define GT_F_LOG_CHECKF(...) ((void)0)
#define GT_F_LOG_FULL(...) ((void)0)
#define GT_F_LOG_MESSAGE(...) ((void)0)
#define GT_F_LOG_MESSAGE_FULL(...) ((void)0)
#define GT_F_LOG_FULL_EX(...) ((void)0)
#define GT_F_LOG_MESSAGE_FULL_EX(...) ((void)0)
#endif
