// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "AsyncAction_PushGorgeousMessage.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGorgeousMessageResultDelegate, FName, ResultTag);

/**
 * Async node to push a Gorgeous Message and wait for the result.
 * This is the ultimate "Developer Experience" node for UI interactions.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UAsyncAction_PushGorgeousMessage : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Pushes a Gorgeous Message to the screen and waits for a result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Messaging", meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_PushGorgeousMessage* PushGorgeousMessage(
		UObject* InWorldContextObject, 
		FText Title, 
		FText Message, 
		const UGorgeousUIMessageConfig_DA* Config
	);

	virtual void Activate() override;

public:
	/** Fired when the user clicks a button that matches the "Confirm" tag. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousMessageResultDelegate OnConfirmed;

	/** Fired when the user clicks a button that matches the "Cancel" tag or closes the dialog. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousMessageResultDelegate OnCancelled;

	/** Fired for any other custom button tags defined in the config. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousMessageResultDelegate OnCustomResult;

private:
	UFUNCTION()
	void HandleMessageResult(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	FGorgeousUIMessageRequest Request;

	UPROPERTY()
	FSignalBridgeEventDelegate ResultDelegate;
};
