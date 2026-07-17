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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousEditorLoggingBlueprintFunctionLibrary.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Editor Logging Blueprint Function Library
| Functional Name: UGorgeousEditorLoggingBlueprintFunctionLibrary
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Function Library providing logging functionalities from the Logging Helper
| exposed to Blueprints in the Editor.
| Provides functionalities to create hyperlinks in the Message Log,
| that can open assets or execute registered actions when clicked.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousEditorLoggingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Registers a callable action that can be executed when a hyperlink in the Message Log is clicked.
	 *
	 * @param HandlerClass The class that will handle the action when the hyperlink is clicked.
	 * @param ActionName A unique name for the action to be registered.
	 * @param FunctionName The name of the function on the HandlerObject that will be called when the hyperlink is clicked.
	 *                     This function must be a UFUNCTION and should accept a single FString parameter for the payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor")
	static void RegisterLogHyperlinkAction(const TSubclassOf<UObject> HandlerClass, const FName ActionName, const FName FunctionName);

	/**
	 * Unregisters a previously registered hyperlink action, preventing it from being executed when hyperlinks with the same action name are clicked.
	 *
	 * @param ActionName The name of the action to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor")
	static void UnregisterLogHyperlinkAction(const FName ActionName);

	/**
	 * Registers a callable condition that determines if a hyperlink in the Message Log is enabled.
	 *
	 * @param HandlerClass The class that will handle the condition check.
	 * @param ConditionName A unique name for the condition to be registered.
	 * @param FunctionName The name of the function on the HandlerObject that will be called.
	 *                     This function must be a UFUNCTION, accept a single FString parameter (payload), and return a bool.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor")
	static void RegisterLogHyperlinkCondition(const TSubclassOf<UObject> HandlerClass, const FName ConditionName, const FName FunctionName);

	/**
	 * Unregisters a previously registered hyperlink condition.
	 *
	 * @param ConditionName The name of the condition to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor")
	static void UnregisterLogHyperlinkCondition(const FName ConditionName);

	/**
	 * Emits a Message Log entry with a clickable hyperlink to an asset.
	 * If the asset is missing, a notification is shown.
	 *
	 * @param Message The message to log.
	 * @param LoggingKey A unique key to manage the log message.
	 * @param Importance The importance level of the log message.
	 * @param AssetPath The soft object path to the asset for the hyperlink.
	 * @param LinkText The text to display for the hyperlink.
	 * @param WorldContextObject The world context object.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor", meta = (WorldContext = "WorldContextObject"))
	static void LogMessageWithAssetHyperlink(const FString Message, const FString LoggingKey,
		const EGorgeousLoggingImportance Importance, const FSoftObjectPath& AssetPath,
		const FString LinkText, UObject* WorldContextObject = nullptr);

	/**
	 * Emits a Message Log entry with a clickable hyperlink that executes a registered action when clicked.
	 *
	 * @param Message The message to log.
	 * @param LoggingKey A unique key to manage the log message.
	 * @param Importance The importance level of the log message.
	 * @param ActionName The name of the registered action to execute when the hyperlink is clicked.
	 * @param ActionPayload An optional string payload that will be passed to the action handler when executed.
	 * @param LinkText The text to display for the hyperlink.
	 * @param bSingleUse If true, the hyperlink will be disabled after a single click.
	 * @param ConditionName Optional name of a registered condition to check if the link should be enabled.
	 * @param WorldContextObject The world context object.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging|Editor", meta = (WorldContext = "WorldContextObject"))
	static void LogMessageWithActionHyperlink(const FString Message, const FString LoggingKey,
		const EGorgeousLoggingImportance Importance, const FName ActionName, const FString ActionPayload,
		const FString LinkText, const bool bSingleUse = true, const FName ConditionName = NAME_None, UObject* WorldContextObject = nullptr);
};

using UGT_EditorLogging_FL = UGorgeousEditorLoggingBlueprintFunctionLibrary;