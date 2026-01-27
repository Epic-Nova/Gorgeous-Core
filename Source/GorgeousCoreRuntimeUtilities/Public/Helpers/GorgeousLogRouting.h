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
#include "CoreMinimal.h"
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
#include "GorgeousLogRouting.generated.h"
//<-------------------------------------------------------------------------->

USTRUCT()
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousLogHyperlink
{
	GENERATED_BODY()

	UPROPERTY()
	FString LinkText;

	UPROPERTY()
	FSoftObjectPath TargetAsset;

	UPROPERTY()
	FName ActionName;

	UPROPERTY()
	FString ActionPayload;

	FORCEINLINE bool IsValid() const
	{
		return !LinkText.IsEmpty() && (TargetAsset.IsValid() || !ActionName.IsNone());
	}
};

USTRUCT()
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousLogEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString Message;

	UPROPERTY()
	FName LoggingKey;

	UPROPERTY()
	TEnumAsByte<EGorgeousLoggingImportance> Importance = Logging_Information;

	UPROPERTY()
	float Duration = 0.0f;

	UPROPERTY()
	bool bPrintToScreen = true;

	UPROPERTY()
	bool bPrintToLog = true;

	UPROPERTY()
	TWeakObjectPtr<const UObject> WorldContextObject;

	UPROPERTY()
	FGorgeousLogHyperlink Hyperlink;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FGorgeousLogEntryDelegate, const FGorgeousLogEntry&);

GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousLogEntryDelegate& GetGorgeousLogEntryDelegate();
