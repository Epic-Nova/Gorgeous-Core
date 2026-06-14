// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStructures.h"
#include "GorgeousPrimaryGameLayout.generated.h"

class UCommonActivatableWidgetContainerBase;
class UCommonActivatableWidget;

/**
 * The primary layout root for the Gorgeous Core|Common UI Foundation system.
 *
 * --- SETUP (Blueprint side) ---
 * 1. Create a Widget Blueprint that inherits from UGorgeousPrimaryGameLayout.
 * 2. In the Blueprint's Graph, call RegisterLayer for each stack you placed in the designer.
 *    (Or use UGorgeousActivatableWidgetStack which auto-registers itself).
 * 3. Assign the blueprint class to UGorgeousUIPolicy::DefaultLayoutClass.
 * Done — the policy creates it for you automatically.
 *
 * --- LAYER TAGS (recommended) ---
 *   UI.Layer.Background — HUD, world elements
 *   UI.Layer.Game       — In-game menus (pause, map)
 *   UI.Layer.Modal      — Dialogs, confirmations
 *   UI.Layer.Loading    — Loading screens (always on top)
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UGorgeousPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** O(1) lookup via UGorgeousUIPolicy. */
	static UGorgeousPrimaryGameLayout* GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject);
	static UGorgeousPrimaryGameLayout* GetPrimaryGameLayout(const APlayerController* PlayerController);
	static UGorgeousPrimaryGameLayout* GetPrimaryGameLayout(const ULocalPlayer* LocalPlayer);

	UGorgeousPrimaryGameLayout(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct();
	virtual void NativeDestruct();

	/** Synchronously push a widget class to a named layer. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	UCommonActivatableWidget* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass);

	/**
	 * Asynchronously load + push a widget to a named layer.
	 * Input is suspended until the load completes (or is cancelled).
	 * @return Streaming handle — cancel it to abort and resume input immediately.
	 */
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(
		FGameplayTag LayerName,
		bool bSuspendInputUntilComplete,
		TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
		TFunction<void(UCommonActivatableWidget&)> InitFunc = nullptr);

	/** Searches all layers and removes the widget. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** Returns the stack container for a layer tag, or null if not registered. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Layout")
	UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName);

	/** Whether the layout is collapsed and only responds to persistent input. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void SetIsDormant(bool bDormant);
	bool IsDormant() const { return bIsDormant; }

	/** Hides a layer by setting its visibility to Collapsed. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void HideLayer(FGameplayTag LayerName);

	/** Shows a layer by setting its visibility to SelfHitTestInvisible. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void ShowLayer(FGameplayTag LayerName);

	/** Checks if a layer is currently visible. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Layout")
	bool IsLayerVisible(FGameplayTag LayerName) const;

public:
	/** Call this in your Blueprint's construct (or use UGorgeousActivatableWidgetStack for auto-registration). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag,
	                   UCommonActivatableWidgetContainerBase* LayerWidget);

protected:
	/** Override to respond to dormancy changes. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void OnIsDormantChanged();

private:
	/** Called by RegisterLayer to wire up transition-based input suspension. */
	void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

	/** Registered layers (tag → stack). */
	UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

	/** Stack of active input suspension tokens. One per in-flight transition. */
	TArray<FName> SuspendInputTokens;

	bool bIsDormant = false;

	/** Delegate for signal-driven widget push. */
	UPROPERTY()
	FSignalBridgeEventDelegate LayoutSignalDelegate;
	UFUNCTION()
	void OnPushWidgetSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/** Handles the signal to register a layer stack. */
	UFUNCTION()
	void OnRegisterLayerSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);
};
