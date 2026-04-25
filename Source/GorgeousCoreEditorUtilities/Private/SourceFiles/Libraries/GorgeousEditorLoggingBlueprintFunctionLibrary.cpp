// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

// Binding struct for hyperlink actions. Maps an action name to a handler class and function.
struct FGorgeousHyperlinkActionBinding
{
	TSubclassOf<UObject> Handler;
	FName FunctionName;
};

// Global map of action name to handler binding. This is populated by calls to RegisterLogHyperlinkAction.
static TMap<FName, FGorgeousHyperlinkActionBinding> GHyperlinkActionBindings;

// Global map of condition name to handler binding. This is populated by calls to RegisterLogHyperlinkCondition.
static TMap<FName, FGorgeousHyperlinkActionBinding> GHyperlinkConditionBindings;

// Helper function to instantiate a handler object from a given class. Logs errors if instantiation fails.
UObject* InstantiateHandler(const TSubclassOf<UObject> HandlerClass)
{
	if (!HandlerClass)
	{
		GT_W_LOG("GT.Logging.Hyperlink", TEXT("InstantiateHandler failed: HandlerClass is null"));
		return nullptr;
	}
	
	UObject* NewInstance = NewObject<UObject>(GetTransientPackage(), HandlerClass);

	if (!NewInstance)
	{
		GT_E_LOG("GT.Logging.Hyperlink", TEXT("InstantiateHandler failed: NewObject failed for %s"), *HandlerClass->GetName());
		return nullptr;
	}

	return NewInstance;
}

/**
 * Executes the action associated with a log hyperlink. This looks up the action name in the global bindings map,
 * instantiates the handler object, and calls the specified function with the action payload as a parameter.
 */
static void ExecuteHyperlinkAction(const FGorgeousLogHyperlink& Hyperlink)
{
	if (Hyperlink.ActionName.IsNone())
		return;

	const FGorgeousHyperlinkActionBinding* Binding = GHyperlinkActionBindings.Find(Hyperlink.ActionName);
	if (!Binding || !Binding->Handler)
	{
		GT_W_LOG("GT.Logging.Hyperlink", TEXT("No valid handler found for action: %s"), *Hyperlink.ActionName.ToString());
		return;
	}

	UObject* HandlerObject = InstantiateHandler(Binding->Handler);
	
	UFunction* Function = HandlerObject->GetClass()->FindFunctionByName(Binding->FunctionName);
	if (!Function)
	{
		GT_W_LOG("GT.Logging.Hyperlink", TEXT("No valid function '%s' found on handler for action: %s"), *Binding->FunctionName.ToString(), *Hyperlink.ActionName.ToString());
		GT_E_LOG_FULL_EX("GT.Logging.Hyperlink", TEXT("Failed to execute action '%s': Function '%s' not found on handler class '%s'"),
			3.f, true, true, true, true, nullptr, nullptr, *Hyperlink.ActionName.ToString(), *Binding->FunctionName.ToString(), *Binding->Handler->GetName());
		return;
	}
	
	struct FActionParams
	{
		FString Payload;
	};

	FActionParams Params{ Hyperlink.ActionPayload };
	HandlerObject->ProcessEvent(Function, &Params);
}

/**
 * Evaluates a hyperlink condition. Returns true if the condition is met (or if no condition is specified).
 */
static bool EvaluateHyperlinkCondition(const FName& ConditionName, const FString& ActionPayload)
{
	if (ConditionName.IsNone())
	{
		return true;
	}

	const FGorgeousHyperlinkActionBinding* Binding = GHyperlinkConditionBindings.Find(ConditionName);
	if (!Binding || !Binding->Handler)
	{
		return true;
	}

	UObject* HandlerObject = InstantiateHandler(Binding->Handler);
	if (!HandlerObject)
	{
		return true;
	}

	UFunction* Function = HandlerObject->GetClass()->FindFunctionByName(Binding->FunctionName);
	if (!Function)
	{
		return true;
	}

	struct FConditionParams
	{
		FString Payload;
		bool bResult;
	};

	FConditionParams Params{ ActionPayload, true };
	HandlerObject->ProcessEvent(Function, &Params);

	return Params.bResult;
}

namespace GorgeousEditorLogging
{
	// Expose for editor module.
	void ExecuteLogHyperlinkAction(const FGorgeousLogHyperlink& Hyperlink)
	{
		ExecuteHyperlinkAction(Hyperlink);
	}

	bool EvaluateLogHyperlinkCondition(const FName& ConditionName, const FString& ActionPayload)
	{
		return EvaluateHyperlinkCondition(ConditionName, ActionPayload);
	}
}

//=============================================================================
// UGorgeousEditorLoggingBlueprintFunctionLibrary Implementation
//=============================================================================

void UGorgeousEditorLoggingBlueprintFunctionLibrary::LogMessageWithAssetHyperlink(const FString Message, const FString LoggingKey,
	const EGorgeousLoggingImportance Importance, const FSoftObjectPath& AssetPath,
	const FString LinkText, UObject* WorldContextObject)
{
	FGorgeousLogHyperlink Hyperlink;
	Hyperlink.LinkText = LinkText.IsEmpty() ? TEXT("Open Asset") : LinkText;
	Hyperlink.TargetAsset = AssetPath;

	GT_LOG_MESSAGE_FULL_EX(Importance, Message, LoggingKey, 5.0f, true, true, true, false, WorldContextObject, &Hyperlink);
}

void UGorgeousEditorLoggingBlueprintFunctionLibrary::RegisterLogHyperlinkAction(const TSubclassOf<UObject> HandlerClass, const FName ActionName, const FName FunctionName)
{
	if (!HandlerClass || ActionName.IsNone() || FunctionName.IsNone())
	{
		return;
	}

	GHyperlinkActionBindings.Add(ActionName, { HandlerClass, FunctionName });
}

void UGorgeousEditorLoggingBlueprintFunctionLibrary::UnregisterLogHyperlinkAction(const FName ActionName)
{
	if (ActionName.IsNone()) 
		return;
	
	GHyperlinkActionBindings.Remove(ActionName);
}

void UGorgeousEditorLoggingBlueprintFunctionLibrary::RegisterLogHyperlinkCondition(const TSubclassOf<UObject> HandlerClass, const FName ConditionName, const FName FunctionName)
{
	if (!HandlerClass || ConditionName.IsNone() || FunctionName.IsNone())
	{
		return;
	}

	GHyperlinkConditionBindings.Add(ConditionName, { HandlerClass, FunctionName });
}

void UGorgeousEditorLoggingBlueprintFunctionLibrary::UnregisterLogHyperlinkCondition(const FName ConditionName)
{
	if (ConditionName.IsNone())
		return;

	GHyperlinkConditionBindings.Remove(ConditionName);
}

void UGorgeousEditorLoggingBlueprintFunctionLibrary::LogMessageWithActionHyperlink(const FString Message, const FString LoggingKey,
                                                                                    const EGorgeousLoggingImportance Importance, const FName ActionName, const FString ActionPayload,
                                                                                    const FString LinkText, const bool bSingleUse, const FName ConditionName, UObject* WorldContextObject)
{
	FGorgeousLogHyperlink Hyperlink;
	Hyperlink.LinkText = LinkText.IsEmpty() ? TEXT("Run Action") : LinkText;
	Hyperlink.ActionName = ActionName;
	Hyperlink.ActionPayload = ActionPayload;
	Hyperlink.bSingleUse = bSingleUse;
	Hyperlink.ConditionName = ConditionName;

	GT_LOG_MESSAGE_FULL_EX(Importance, Message, LoggingKey, 5.0f, true, true, true, false, WorldContextObject, &Hyperlink);
}
