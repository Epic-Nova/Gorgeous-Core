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
#include "Libraries/GorgeousEditorLoggingUtility.h"

#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

struct FGorgeousHyperlinkActionBinding
{
	TWeakObjectPtr<UObject> Handler;
	FName FunctionName;
};

static TMap<FName, FGorgeousHyperlinkActionBinding> GHyperlinkActionBindings;

void UGorgeousEditorLoggingUtility::LogMessageWithAssetHyperlink(const FString Message, const FString LoggingKey,
	EGorgeousLoggingImportance Importance, const FSoftObjectPath& AssetPath,
	const FString LinkText, UObject* WorldContextObject)
{
	FGorgeousLogHyperlink Hyperlink;
	Hyperlink.LinkText = LinkText.IsEmpty() ? TEXT("Open Asset") : LinkText;
	Hyperlink.TargetAsset = AssetPath;

	GT_LOG_MESSAGE_FULL_EX(Importance, Message, LoggingKey, 5.0f, true, true, true, false, WorldContextObject, &Hyperlink);
}

void UGorgeousEditorLoggingUtility::RegisterLogHyperlinkAction(UObject* HandlerObject, FName ActionName, FName FunctionName)
{
	if (!HandlerObject || ActionName.IsNone() || FunctionName.IsNone())
	{
		return;
	}

	GHyperlinkActionBindings.Add(ActionName, { HandlerObject, FunctionName });
}

void UGorgeousEditorLoggingUtility::LogMessageWithActionHyperlink(const FString Message, const FString LoggingKey,
	EGorgeousLoggingImportance Importance, FName ActionName, const FString ActionPayload,
	const FString LinkText, UObject* WorldContextObject)
{
	FGorgeousLogHyperlink Hyperlink;
	Hyperlink.LinkText = LinkText.IsEmpty() ? TEXT("Run Action") : LinkText;
	Hyperlink.ActionName = ActionName;
	Hyperlink.ActionPayload = ActionPayload;

	GT_LOG_MESSAGE_FULL_EX(Importance, Message, LoggingKey, 5.0f, true, true, true, false, WorldContextObject, &Hyperlink);
}

static void ExecuteHyperlinkAction(const FGorgeousLogHyperlink& Hyperlink)
{
	if (Hyperlink.ActionName.IsNone())
		return;

	const FGorgeousHyperlinkActionBinding* Binding = GHyperlinkActionBindings.Find(Hyperlink.ActionName);
	if (!Binding || !Binding->Handler.IsValid())
		return;

	UObject* HandlerObject = Binding->Handler.Get();
	UFunction* Function = HandlerObject ? HandlerObject->FindFunction(Binding->FunctionName) : nullptr;
	if (!Function)
		return;

	struct FActionParams
	{
		FString Payload;
	};

	FActionParams Params{ Hyperlink.ActionPayload };
	HandlerObject->ProcessEvent(Function, &Params);
}

// Expose for editor module.
namespace GorgeousEditorLogging
{
	void ExecuteLogHyperlinkAction(const FGorgeousLogHyperlink& Hyperlink)
	{
		ExecuteHyperlinkAction(Hyperlink);
	}
}
