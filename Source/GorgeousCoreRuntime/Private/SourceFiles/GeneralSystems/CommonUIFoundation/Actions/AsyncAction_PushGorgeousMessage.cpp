// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Actions/AsyncAction_PushGorgeousMessage.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"

#include "UObject/Stack.h"

UAsyncAction_PushGorgeousMessage* UAsyncAction_PushGorgeousMessage::PushGorgeousMessage(
	UObject* InWorldContextObject, 
	FText Title, 
	FText Message, 
	const UGorgeousUIMessageConfig_DA* Config)
{
	if (!Config)
	{
		FFrame::KismetExecutionMessage(TEXT("PushGorgeousMessage: Config is null! A message template is required."), ELogVerbosity::Error);
		return nullptr;
	}

	UAsyncAction_PushGorgeousMessage* Action = NewObject<UAsyncAction_PushGorgeousMessage>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Request.Title = Title;
	Action->Request.Message = Message;
	Action->Request.Config = Config;
	Action->RegisterWithGameInstance(InWorldContextObject);
	return Action;
}

void UAsyncAction_PushGorgeousMessage::Activate()
{
	if (!WorldContextObject || !Request.Config)
	{
		SetReadyToDestroy();
		return;
	}

	FGameplayTag PushTag = TAG_Gorgeous_UI_System_Message_Push;
	FGameplayTag ResultTag = TAG_Gorgeous_UI_System_Message_Result;
	
	// 1. Register local signal rules for these tags
	FGorgeousSignalBridgeAccessRules_S Rules;
	Rules.bNetworked = false; // local-only

	USignalBridgeBlueprintFunctionLibrary::RegisterSignal(WorldContextObject, PushTag, Rules, nullptr);
	USignalBridgeBlueprintFunctionLibrary::RegisterSignal(WorldContextObject, ResultTag, Rules, nullptr);

	// 2. Listen for the result (use the member delegate declared in the header)
	ResultDelegate.BindDynamic(this, &UAsyncAction_PushGorgeousMessage::HandleMessageResult);
	USignalBridgeBlueprintFunctionLibrary::Listen(WorldContextObject, ResultTag, nullptr, ResultDelegate);

	// 3. Dispatch the message
	USignalBridgeBlueprintFunctionLibrary::Dispatch(WorldContextObject, PushTag, FInstancedStruct::Make(Request));
}

void UAsyncAction_PushGorgeousMessage::HandleMessageResult(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	// Clean up listeners
	FGameplayTag ResultTag = TAG_Gorgeous_UI_System_Message_Result;
	USignalBridgeBlueprintFunctionLibrary::Clear(WorldContextObject, ResultTag, nullptr);

	const FGorgeousUIMessageResult* Result = Payload.GetPtr<FGorgeousUIMessageResult>();
    
	// Ensure this result is actually for OUR request
	if (!Result || Result->RequestID != Request.RequestID) return;

	// Fire the appropriate pin
	const FName& Res = Result->ResultTag;
	if (Res == FName("Confirm") || Res == FName("Yes") || Res == FName("Ok"))
	{
		OnConfirmed.Broadcast(Res);
	}
	else if (Res == FName("Cancel") || Res == FName("No"))
	{
		OnCancelled.Broadcast(Res);
	}
	else
	{
		OnCustomResult.Broadcast(Res);
	}

	// Clean up
	SetReadyToDestroy();
}
