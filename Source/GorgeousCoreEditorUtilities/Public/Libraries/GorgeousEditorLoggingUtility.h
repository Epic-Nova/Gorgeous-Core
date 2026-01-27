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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
#include "UObject/SoftObjectPath.h"
#include "GorgeousEditorLoggingUtility.generated.h"
//<-------------------------------------------------------------------------->

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousEditorLoggingUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Emits a Message Log entry with a clickable hyperlink to an asset.
	 * If the asset is missing, a notification is shown.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor", meta = (WorldContext = "WorldContextObject"))
	static void LogMessageWithAssetHyperlink(const FString Message, const FString LoggingKey,
		EGorgeousLoggingImportance Importance, const FSoftObjectPath& AssetPath,
		const FString LinkText, UObject* WorldContextObject = nullptr);

	/** Registers a callable action for hyperlink execution. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor")
	static void RegisterLogHyperlinkAction(UObject* HandlerObject, FName ActionName, FName FunctionName);

	/** Emits a Message Log entry with a clickable hyperlink that executes a registered action. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor", meta = (WorldContext = "WorldContextObject"))
	static void LogMessageWithActionHyperlink(const FString Message, const FString LoggingKey,
		EGorgeousLoggingImportance Importance, FName ActionName, const FString ActionPayload,
		const FString LinkText, UObject* WorldContextObject = nullptr);
};
