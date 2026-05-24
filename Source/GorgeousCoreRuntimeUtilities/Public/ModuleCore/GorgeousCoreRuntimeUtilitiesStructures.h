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
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousCoreRuntimeUtilitiesStructures.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Struct representing a hyperlink in a gorgeous log entry.
 * 
 * @author Nils Bergemann
 * 
 * @note https://c.tenor.com/m7WYX_Q1_mUAAAAd/tenor.gif - Bread
 */
USTRUCT()
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousLogHyperlink
{
	GENERATED_BODY()

	// The text to display for the hyperlink.
	UPROPERTY()
	FString LinkText;

	// The target asset to open when the hyperlink is clicked.
	UPROPERTY()
	FSoftObjectPath TargetAsset;

	// The action name to execute when the hyperlink is clicked.
	UPROPERTY()
	FName ActionName;

	// The payload associated with the action.
	UPROPERTY()
	FString ActionPayload;

	// If true, the action hyperlink can only be executed once.
	UPROPERTY()
	bool bSingleUse = true;

	// Optional action name to check for availability/condition of this hyperlink.
	UPROPERTY()
	FName ConditionName;

	/** 
	 * Checks if the hyperlink is valid.
	 * A hyperlink is considered valid if it has non-empty link text and either a valid target asset or a non-empty action name.
	 * 
	 * @return true if the hyperlink is valid, false otherwise.
	 */
	FORCEINLINE bool IsValid() const
	{
		return !LinkText.IsEmpty() && (TargetAsset.IsValid() || !ActionName.IsNone());
	}
};

/**
 * Struct representing a log entry in the Gorgeous logging system.
 * 
 * @author Nils Bergemann
 */
USTRUCT()
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousLogEntry
{
	GENERATED_BODY()

	// The log message.
	UPROPERTY()
	FString Message;

	// The logging key associated with the log entry.
	UPROPERTY()
	FName LoggingKey;

	// The importance level of the log entry.
	UPROPERTY()
	TEnumAsByte<EGorgeousLoggingImportance> Importance = Logging_Information;

	// The duration for which the log entry should be displayed.
	UPROPERTY()
	float Duration = 0.0f;

	// Whether to print the log entry to the screen.
	UPROPERTY()
	bool bPrintToScreen = true;

	// Whether to print the log entry to the log file.
	UPROPERTY()
	bool bPrintToLog = true;

	// The world context object associated with the log entry.
	UPROPERTY()
	TWeakObjectPtr<const UObject> WorldContextObject;

	// The hyperlink associated with the log entry.
	UPROPERTY()
	FGorgeousLogHyperlink Hyperlink;
};