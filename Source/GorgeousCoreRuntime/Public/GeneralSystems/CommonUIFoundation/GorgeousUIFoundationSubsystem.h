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
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSignalForwarder.h"
#include "Subsystems/LocalPlayerSubsystem.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CommonInputSubsystem.h"
#include "CommonInputSubsystem.h"
#include "Tickable.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIFoundationSubsystem.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousUIState_DA;
class UGorgeousInputBinding_DA;
class UGorgeousUIProcessor;
class UGorgeousCommonWidget;
class UGorgeousSignalBridgeTagForwarder;
//<------------------------------------------------------------->
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateTransitionStarted, UGorgeousUIState_DA*, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateTransitionFinished, UGorgeousUIState_DA*, NewState);

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIFoundation Subsystem
| Functional Name: UGorgeousUIFoundationSubsystem
| Parent Class: ULocalPlayerSubsystem
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Manager for the Gorgeous Core|Common UI Foundation Foundation system.
| Handles state switching, processor lifecycle, and Signal Bridge
| integration.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIFoundationSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	friend class UGorgeousSignalBridgeTagForwarder;


	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	//<----------------------=== FTickableGameObject Overrides ===---------------------->

	// Updates UI Foundation state each frame.
	virtual void Tick(float DeltaTime) override;

	// Returns the tick policy for this subsystem.
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }

	// Returns the stat identifier used for tick profiling.
	virtual TStatId GetStatId() const override;

	// Keeps the subsystem ticking while the game is paused.
	virtual bool IsTickableWhenPaused() const override { return true; }
	//<----------------------=== End FTickableGameObject Overrides ===------------------>

	//<----------------------=== USubsystem Overrides ===---------------------->

	// Initializes state, processors, and Signal Bridge listeners.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//<----------------------=== End USubsystem Overrides ===------------------>
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Pushes a state onto the active UI state stack.
	 *
	 * @param NewState The state to activate.
	 * @param bImmediate Whether to bypass transition animations.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation", DisplayName = "Push UI State")
	void PushUIState(UGorgeousUIState_DA* NewState, bool bImmediate = false);

	/**
	 * Removes the most recent matching state from the active stack.
	 *
	 * @param StateClass The state class to remove.
	 * @param bImmediate Whether to bypass transition animations.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation", DisplayName = "Remove UI State")
	void RemoveUIState(TSubclassOf<UGorgeousUIState_DA> StateClass, bool bImmediate = false);

	/**
	 * Records completion of a widget's outgoing transition.
	 *
	 * @param Widget The widget that completed its transition.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	void NotifyWidgetTransitionComplete(UObject* Widget);

	// Event fired when a UI state transition begins. Widgets should use this to trigger outro animations.
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|Common UI Foundation")
	FOnUIStateTransitionStarted OnTransitionStarted;

	// Event fired when a UI state transition completes and the new state is applied. Widgets should use this for intro animations.
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|Common UI Foundation")
	FOnUIStateTransitionFinished OnTransitionFinished;

	/**
	 * Returns the currently active UI states.
	 *
	 * @return The active state stack.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	TArray<UGorgeousUIState_DA*> GetCurrentUIStates() const { return CurrentStates; }

	/**
	 * Returns the currently active UI themes.
	 *
	 * @return The active theme stack.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	TArray<UGorgeousUITheme_DA*> GetCurrentThemes() const;

	/**
	 * Returns the most recently activated UI state.
	 *
	 * @return The latest state, or null when no state is active.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	UGorgeousUIState_DA* GetMostRecentUIState() const { return CurrentStates.Num() > 0 ? CurrentStates.Last() : nullptr; }

	/**
	 * Returns the most recently activated UI theme.
	 *
	 * @return The latest theme, or null when no theme is active.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	UGorgeousUITheme_DA* GetMostRecentTheme() const;

	/**
	 * Finds an active UI state by object identity.
	 *
	 * @param InState The state to locate.
	 * @return The matching active state, or null when it is absent.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	UGorgeousUIState_DA* GetUIStateByObject(UGorgeousUIState_DA* InState) const;

	/**
	 * Finds an active UI theme by object identity.
	 *
	 * @param InTheme The theme to locate.
	 * @return The matching active theme, or null when it is absent.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	UGorgeousUITheme_DA* GetThemeByObject(UGorgeousUITheme_DA* InTheme) const;

	/**
	 * Sets and broadcasts a new theme to registered widgets.
	 *
	 * @param NewTheme The theme to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	void SetCurrentTheme(UGorgeousUITheme_DA* NewTheme);

	/** Track total theme swaps for the Insight Matrix. */
	static int32 GetTotalThemeSwapsTriggered();
	static void IncrementThemeSwapsTriggered();

	/**
	 * Applies a theme to one widget through its shared processor.
	 *
	 * @param Widget The widget that receives the theme.
	 * @param Theme The theme to apply.
	 * @param FallbackTheme The optional fallback theme.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* Theme, const UGorgeousUITheme_DA* FallbackTheme = nullptr);

	/** Broadcasts theme application to all registered widgets (without changing the stored theme). */
	void BroadcastThemeApplied(UGorgeousUITheme_DA* Theme);

	/** Broadcasts a state switch to all registered widgets. */
	void BroadcastStateSwitch(UGorgeousUIState_DA* NewState);

	/**
	 * Returns the current platform name for the active input method.
	 *
	 * @return The active platform name.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	FName GetCurrentPlatformName() const;

	/** Registers a widget and assigns a shared processor if a binding tag is present. */
	void RegisterWidget(IGorgeousUIWidget_I* Widget);

	/** Unregisters a widget and tears down its signal listener. */
	void UnregisterWidget(IGorgeousUIWidget_I* Widget);

	/**
	 * Returns the currently active input bindings.
	 *
	 * @return The active input-binding stack.
	 */
	/**
	 * Returns the most recently activated input bindings.
	 *
	 * @return The latest bindings, or null when none are active.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	TArray<UGorgeousInputBinding_DA*> GetActiveInputBindings() const { return ActiveInputBindings; }

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	UGorgeousInputBinding_DA* GetMostRecentInputBindings() const { return ActiveInputBindings.Last(); }

	/**
	 * Pushes input bindings onto the active stack.
	 *
	 * @param NewBindings The bindings to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	void PushInputBinding(UGorgeousInputBinding_DA* NewBindings);

	/** Pops the most recent input binding from the active stack. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	void PopInputBinding();

	/**
	 * Removes input bindings from the active stack.
	 *
	 * @param BindingToRemove The bindings to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	void RemoveInputBinding(UGorgeousInputBinding_DA* BindingToRemove);

	/**
	 * Redirects a hardware input tag to a simulated action tag.
	 *
	 * @param HardwareTag The physical input tag to redirect.
	 * @param SimulatedTag The action tag to simulate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Input")
	void SetInputRedirect(FGameplayTag HardwareTag, FGameplayTag SimulatedTag);

	/**
	 * Clears an input redirect.
	 *
	 * @param HardwareTag The physical input tag whose redirect is cleared.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Input")
	void ClearInputRedirect(FGameplayTag HardwareTag);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
protected:
	/**
	 * Processes a forwarded payload for a signal tag.
	 *
	 * @param Tag The tag associated with the payload.
	 * @param Payload The forwarded payload.
	 */
	void HandlePayloadForTag(FGameplayTag Tag, const FInstancedStruct& Payload);
	/**
	 * Handles a global focus request signal.
	 *
	 * @param SignalTag The received signal tag.
	 * @param Payload The received signal payload.
	 */
	UFUNCTION()
	void OnFocusRequestReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/**
	 * Handles a global input action signal.
	 *
	 * @param SignalTag The received signal tag.
	 * @param Payload The received signal payload.
	 */
	UFUNCTION()
	void OnInputActionReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/**
	 * Handles a global message request signal.
	 *
	 * @param SignalTag The received signal tag.
	 * @param Payload The received signal payload.
	 */
	UFUNCTION()
	void OnMessageRequestReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/** Responds when the active input method changes.
	 *
	 * @param NewInputType The newly active input method.
	 */
	void OnInputMethodChanged(ECommonInputType NewInputType);

	/** Delegate handle for the focus request listener. */
	FSignalBridgeEventDelegate FocusRequestDelegate;

	/** Slate delegate handle for global focus tracking. */
	FDelegateHandle SlateFocusHandle;

	/** Global hook to intercept focus changes from Slate. */
	void HandleGlobalFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget);

	// Delegate handle for the input action listener.
	UPROPERTY()
	FSignalBridgeEventDelegate InputActionDelegate;

	// Delegate handle for the message request listener.
	UPROPERTY()
	FSignalBridgeEventDelegate MessageRequestDelegate;
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks


	//<============================--- Variables ---============================>
	#pragma region Variables
protected:
	// Current active UI state.
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Gorgeous Core|Common UI Foundation")
	TArray<TObjectPtr<UGorgeousUIState_DA>> CurrentStates;

	// Parallel array tracking the last focused binding tag for each UI state.
	UPROPERTY(Transient)
	TArray<FGameplayTag> StateFocusHistory;

	// Current active theme.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Common UI Foundation")
	TArray<TObjectPtr<UGorgeousUITheme_DA>> CurrentThemes;

	// Mapping of widget/object classes to their corresponding processor classes.
	UPROPERTY(EditAnywhere, Category = "Gorgeous Core|Common UI Foundation")
	TMap<TSubclassOf<UObject>, TSubclassOf<UGorgeousUIProcessor>> ProcessorClasses;

protected:
	// Pool of shared processor instances (one per class).
	UPROPERTY(Transient)
	TMap<UClass*, TObjectPtr<UGorgeousUIProcessor>> SharedProcessors;

	/** Finds or creates a shared processor instance for a specific widget/object class. */
	UGorgeousUIProcessor* GetSharedProcessorForWidget(UObject* Widget);

	// List of all registered widgets.
	UPROPERTY()
	TArray<TScriptInterface<IGorgeousUIWidget_I>> RegisteredWidgets;

	// Forwarders created per SignalBridge tag to adapt dynamic delegates.
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UGorgeousSignalBridgeTagForwarder>> TagForwarders;

	// The currently active input-to-tag mapping.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGorgeousInputBinding_DA>> ActiveInputBindings;

	// Active redirects for hardware inputs to simulate different tags.
	UPROPERTY(Transient)
	TMap<FGameplayTag, FGameplayTag> ActiveInputRedirects;

	/** Active Enhanced Input action binding handles. Used to clean up previously bound actions on the HUD player controller. */
	TArray<uint32> BridgedBindingHandles;

	bool bSignalsRegistered = false;

	/** Internal helper to setup the input bridge on the HUD. */
	void SetupInputBridgeOnHUD();

	/** Actually performs the state swap after transitions are complete. */
	void ExecuteStateSwap();

	// Target state waiting for transition completion.
	UPROPERTY()
	TObjectPtr<UGorgeousUIState_DA> PendingState;

	UPROPERTY(Transient)
	TArray<FGorgeousPendingFocusRequest> PendingFocusRequests;

	// Set of widgets currently performing transition animations.
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<UObject>> TransitioningWidgets;

	/** Handles Enhanced Input actions bridged to tags.
	 *
	 * @param Instance The action instance received from Enhanced Input.
	 */
	void HandleBridgedInputAction(const FInputActionInstance& Instance);
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};