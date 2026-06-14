// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "UObject/SoftObjectPtr.h"
#include "AsyncAction_CreateGorgeousWidgetAsync.generated.h"

class UUserWidget;
class UCommonActivatableWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGorgeousWidgetAsyncDelegate, UUserWidget*, Widget);

/**
 * Async node to create a widget without frame hitches.
 * Automatically applies the current global theme upon creation.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UAsyncAction_CreateGorgeousWidgetAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Asynchronously loads and creates a Gorgeous widget.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_CreateGorgeousWidgetAsync* CreateGorgeousWidgetAsync(
		UObject* InWorldContextObject, 
		TSoftClassPtr<UUserWidget> WidgetClass,
		bool bSuspendInputUntilComplete = true
	);

	virtual void Activate() override;

public:
	/** Fired when the widget has been successfully loaded and created. */
	UPROPERTY(BlueprintAssignable)
	FGorgeousWidgetAsyncDelegate OnComplete;

private:
	void OnWidgetClassLoaded();

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TSoftClassPtr<UUserWidget> WidgetSoftClass;

	UPROPERTY(Transient)
	bool bSuspendInput = false;

	UPROPERTY(Transient)
	FName SuspendToken;
};
