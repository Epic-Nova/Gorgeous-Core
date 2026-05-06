// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/CommonUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/CommonUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSignalForwarder.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "CommonInputSubsystem.h"
#include "GorgeousUIFoundationSubsystem.generated.h"

class UCommonUIState_DA;
class UGorgeousUIProcessor;
class UGorgeousCommonWidget;
class UGorgeousSignalBridgeTagForwarder;

/**
 * Manager for the Gorgeous UI Foundation system.
 * Handles state switching, processor lifecycle, and Signal Bridge integration.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousUIFoundationSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	friend class UGorgeousSignalBridgeTagForwarder;

public:
	// USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End USubsystem Interface

	/** Switches the active UI state. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void SwitchUIState(UCommonUIState_DA* NewState);

	/** Returns the currently active theme. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	UGorgeousUITheme_DA* GetCurrentTheme() const { return CurrentTheme; }

	/** Sets and broadcasts a new theme to all registered widgets. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void SetCurrentTheme(UGorgeousUITheme_DA* NewTheme);

	/** Broadcasts theme application to all registered widgets (without changing the stored theme). */
	void BroadcastThemeApplied(UGorgeousUITheme_DA* Theme);

	/** Broadcasts a state switch to all registered widgets. */
	void BroadcastStateSwitch(UCommonUIState_DA* NewState);

	/** Returns the current platform name based on the active input method. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	FName GetCurrentPlatformName() const;

	/** Registers a widget and assigns a shared processor if a binding tag is present. */
	void RegisterWidget(IGorgeousUIWidget_I* Widget);

	/** Unregisters a widget and tears down its signal listener. */
	void UnregisterWidget(IGorgeousUIWidget_I* Widget);

protected:
	/** Internal handler invoked by tag forwarders to process a payload for a specific tag. */
	void HandlePayloadForTag(FGameplayTag Tag, const FInstancedStruct& Payload);
	/** Called when a global focus request signal is received. */
	UFUNCTION()
	void OnFocusRequestReceived(const FInstancedStruct& Payload);

	/** Called when a global input action signal is received. */
	UFUNCTION()
	void OnInputActionReceived(const FInstancedStruct& Payload);

	/** Called when a global message request is received. */
	UFUNCTION()
	void OnMessageRequestReceived(const FInstancedStruct& Payload);

	/** Called when the input method (Gamepad/M&K) changes. */
	void OnInputMethodChanged(ECommonInputType NewInputType);

	/** Delegate handle for the focus request listener. */
	UPROPERTY()
	FSignalBridgeEventDelegate FocusRequestDelegate;

	/** Delegate handle for the input action listener. */
	UPROPERTY()
	FSignalBridgeEventDelegate InputActionDelegate;

	/** Delegate handle for the message request listener. */
	UPROPERTY()
	FSignalBridgeEventDelegate MessageRequestDelegate;



protected:
	/** Current active UI state. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous UI")
	UCommonUIState_DA* CurrentState = nullptr;

	/** Current active theme. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous UI")
	TObjectPtr<UGorgeousUITheme_DA> CurrentTheme;

	/** Mapping of widget/object classes to their corresponding processor classes. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous UI")
	TMap<TSubclassOf<UObject>, TSubclassOf<UGorgeousUIProcessor>> ProcessorClasses;

protected:
	/** Pool of shared processor instances (one per class). */
	UPROPERTY(Transient)
	TMap<UClass*, TObjectPtr<UGorgeousUIProcessor>> SharedProcessors;

	/** Finds or creates a shared processor instance for a specific widget/object class. */
	UGorgeousUIProcessor* GetSharedProcessorForWidget(UObject* Widget);

	/** List of all registered widgets. */
	UPROPERTY()
	TArray<TScriptInterface<IGorgeousUIWidget_I>> RegisteredWidgets;

	/** Forwarders created per SignalBridge tag to adapt dynamic delegates. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UGorgeousSignalBridgeTagForwarder>> TagForwarders;
};
