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
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Kismet/BlueprintAsyncActionBase.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "AsyncAction_PushGorgeousMessage.generated.h"
//<-------------------------------------------------------------------------->

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGorgeousMessageResultDelegate, FName, ResultTag);

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Async Action Push Gorgeous Message
| Functional Name: UAsyncAction_PushGorgeousMessage
| Parent Class: UBlueprintAsyncActionBase
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Async node to push a Gorgeous Message and wait for the result. This is the
| ultimate "Developer Experience" node for UI interactions.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Actions/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Actions/AsyncAction_PushGorgeousMessage",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Actions/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UAsyncAction_PushGorgeousMessage : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Pushes a Gorgeous Message to the screen and waits for a result.
	 *
	 * @param InWorldContextObject The world context that owns the request.
	 * @param Title The localized dialog title.
	 * @param Message The localized dialog message.
	 * @param Config The configuration that defines the dialog presentation.
	 * @return The asynchronous action that reports the dialog result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Messaging", meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_PushGorgeousMessage* PushGorgeousMessage(UObject* InWorldContextObject, FText Title, FText Message, const UGorgeousUIMessageConfig_DA* Config);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Starts the message request after Blueprint has configured the async action.
	virtual void Activate() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
public:
	// Fires when the user confirms the message.
	UPROPERTY(BlueprintAssignable)
	FGorgeousMessageResultDelegate OnConfirmed;
	// Fires when the user cancels or closes the message.
	UPROPERTY(BlueprintAssignable)
	FGorgeousMessageResultDelegate OnCancelled;
	// Fires for a custom result tag selected by the user.
	UPROPERTY(BlueprintAssignable)
	FGorgeousMessageResultDelegate OnCustomResult;
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:
	/**
	 * Routes the bridge result to the matching asynchronous action delegate.
	 *
	 * @param SignalTag The signal tag emitted by the message response.
	 * @param Payload The response payload emitted with the signal.
	 */
	UFUNCTION()
	void HandleMessageResult(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
private:
	// Stores the world context used to submit the request.
	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;
	// Stores the dialog request submitted to the message system.
	UPROPERTY(Transient)
	FGorgeousUIMessageRequest Request;
	// Stores the Signal Bridge delegate bound to the request result.
	UPROPERTY()
	FSignalBridgeEventDelegate ResultDelegate;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};