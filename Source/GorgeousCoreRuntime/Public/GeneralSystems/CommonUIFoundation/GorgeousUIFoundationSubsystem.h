// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSignalForwarder.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "CommonInputSubsystem.h"
#include "CommonInputSubsystem.h"
#include "Tickable.h"
#include "GorgeousUIFoundationSubsystem.generated.h"

class UGorgeousUIState_DA;
class UGorgeousInputBinding_DA;
class UGorgeousUIProcessor;
class UGorgeousCommonWidget;
class UGorgeousSignalBridgeTagForwarder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateTransitionStarted, UGorgeousUIState_DA*, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateTransitionFinished, UGorgeousUIState_DA*, NewState);

/**
 * Manager for the Gorgeous UI Foundation system.
 * Handles state switching, processor lifecycle, and Signal Bridge integration.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousUIFoundationSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	friend class UGorgeousSignalBridgeTagForwarder;

public:
	// FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	// End FTickableGameObject Interface

public:
	// USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End USubsystem Interface

	/** Pushes a state on top of the active UI state stack. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI", DisplayName = "Push UI State")
	void PushUIState(UGorgeousUIState_DA* NewState, bool bImmediate = false);
	
	/** Removes the most recent matching state from the active UI state stack. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI", DisplayName = "Remove UI State")
	void RemoveUIState(TSubclassOf<UGorgeousUIState_DA> StateClass, bool bImmediate = false);

	/** Called by widgets when they have finished their outgoing transition animations. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void NotifyWidgetTransitionComplete(UObject* Widget);

	/** Event fired when a UI state transition begins. Widgets should use this to trigger outro animations. */
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous UI")
	FOnUIStateTransitionStarted OnTransitionStarted;

	/** Event fired when a UI state transition completes and the new state is applied. Widgets should use this for intro animations. */
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous UI")
	FOnUIStateTransitionFinished OnTransitionFinished;

	/** Returns the currently active UI state. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	TArray<UGorgeousUIState_DA*> GetCurrentUIStates() const { return CurrentStates; }

	/** Returns the currently active theme. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	TArray<UGorgeousUITheme_DA*> GetCurrentThemes() const;
	
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	UGorgeousUIState_DA* GetMostRecentUIState() const { return CurrentStates.Last(); }
	
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")	
	UGorgeousUITheme_DA* GetMostRecentTheme() const;
	
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	UGorgeousUIState_DA* GetUIStateByObject(UGorgeousUIState_DA* InState) const;
	
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	UGorgeousUITheme_DA* GetThemeByObject(UGorgeousUITheme_DA* InTheme) const;

	/** Sets and broadcasts a new theme to all registered widgets. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void SetCurrentTheme(UGorgeousUITheme_DA* NewTheme);

	/** Applies a theme to a single widget via its shared processor. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* Theme, const UGorgeousUITheme_DA* FallbackTheme = nullptr);

	/** Broadcasts theme application to all registered widgets (without changing the stored theme). */
	void BroadcastThemeApplied(UGorgeousUITheme_DA* Theme);

	/** Broadcasts a state switch to all registered widgets. */
	void BroadcastStateSwitch(UGorgeousUIState_DA* NewState);

	/** Returns the current platform name based on the active input method. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	FName GetCurrentPlatformName() const;

	/** Registers a widget and assigns a shared processor if a binding tag is present. */
	void RegisterWidget(IGorgeousUIWidget_I* Widget);

	/** Unregisters a widget and tears down its signal listener. */
	void UnregisterWidget(IGorgeousUIWidget_I* Widget);

	/** Returns the currently active input bindings. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	TArray<UGorgeousInputBinding_DA*> GetActiveInputBindings() const { return ActiveInputBindings; }
	
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")	
	UGorgeousInputBinding_DA* GetMostRecentInputBindings() const { return ActiveInputBindings.Last(); }

	/** Manually sets the active input bindings. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void SetActiveInputBindings(UGorgeousInputBinding_DA* NewBindings);

protected:
	/** Internal handler invoked by tag forwarders to process a payload for a specific tag. */
	void HandlePayloadForTag(FGameplayTag Tag, const FInstancedStruct& Payload);
	/** Called when a global focus request signal is received. */
	UFUNCTION()
	void OnFocusRequestReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/** Called when a global input action signal is received. */
	UFUNCTION()
	void OnInputActionReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/** Called when a global message request is received. */
	UFUNCTION()
	void OnMessageRequestReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

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
	TArray<TObjectPtr<UGorgeousUIState_DA>> CurrentStates;

	/** Current active theme. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous UI")
	TArray<TObjectPtr<UGorgeousUITheme_DA>> CurrentThemes;

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

	/** The currently active input-to-tag mapping. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGorgeousInputBinding_DA>> ActiveInputBindings;

	/** Internal helper to setup the input bridge on the HUD. */
	void SetupInputBridgeOnHUD();

	/** Actually performs the state swap after transitions are complete. */
	void ExecuteStateSwap();

	/** Target state waiting for transition completion. */
	UPROPERTY(Transient)
	TObjectPtr<UGorgeousUIState_DA> PendingState;

	/** Set of widgets currently performing transition animations. */
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<UObject>> TransitioningWidgets;

	/** Handler for Enhanced Input actions bridged to tags. */
	void HandleBridgedInputAction(const FInputActionValue& Value, UInputAction* Action);
};
