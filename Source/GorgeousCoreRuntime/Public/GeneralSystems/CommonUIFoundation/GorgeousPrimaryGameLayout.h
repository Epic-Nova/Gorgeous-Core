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
#include "GeneralSystems/SignalBridge/SignalBridgeStructures.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousPrimaryGameLayout.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UCommonActivatableWidgetContainerBase;
class UCommonActivatableWidget;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Primary Game Layout
| Functional Name: UGorgeousPrimaryGameLayout
| Parent Class: UCommonUserWidget
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| The primary layout root for the Gorgeous Core|Common UI Foundation system.
| --- SETUP (Blueprint side) --- 1. Create a Widget Blueprint that inherits
| from UGorgeousPrimaryGameLayout. 2. In the Blueprint's Graph, call
| RegisterLayer for each stack you placed in the designer. (Or use
| UGorgeousActivatableWidgetStack which auto-registers itself). 3. Assign
| the blueprint class to UGorgeousUIPolicy::DefaultLayoutClass. Done, the
| policy creates it for you automatically. --- LAYER TAGS (recommended) ---
| UI.Layer.Background, HUD, world elements UI.Layer.Game , In-game menus
| (pause, map) UI.Layer.Modal , Dialogs, confirmations UI.Layer.Loading ,
| Loading screens (always on top)
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Abstract,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	// Initializes the primary layout and its layer-management state.
	UGorgeousPrimaryGameLayout(const FObjectInitializer& ObjectInitializer);


	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	//<----------------------=== UUserWidget Overrides ===---------------------->

	// Registers layout services after the widget is constructed.
	virtual void NativeConstruct();

	// Releases layout services before the widget is destroyed.
	virtual void NativeDestruct();
	//<----------------------=== End UUserWidget Overrides ===------------------>
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Resolves the primary layout for the primary local player.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @return The primary player's layout, or null when it is unavailable.
	 */
	static UGorgeousPrimaryGameLayout* GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject);
	/**
	 * Resolves the primary layout for a player controller.
	 *
	 * @param PlayerController The controller that owns the target local player.
	 * @return The matching primary layout, or null when it is unavailable.
	 */
	static UGorgeousPrimaryGameLayout* GetPrimaryGameLayout(const APlayerController* PlayerController);
	/**
	 * Resolves the primary layout for a local player.
	 *
	 * @param LocalPlayer The player that owns the target layout.
	 * @return The matching primary layout, or null when it is unavailable.
	 */
	static UGorgeousPrimaryGameLayout* GetPrimaryGameLayout(const ULocalPlayer* LocalPlayer);

	/** Track active UI Overlays for the Insight Matrix. */
	static int32 GetTotalActiveOverlays();
	static void IncrementActiveOverlays();
	static void DecrementActiveOverlays();

	/**
	 * Synchronously pushes a widget class onto a named layer.
	 *
	 * @param LayerName The layer that receives the widget.
	 * @param ActivatableWidgetClass The widget class to create.
	 * @return The pushed widget, or null when the layer is unavailable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	UCommonActivatableWidget* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass);

	/**
	 * Asynchronously load + push a widget to a named layer.
	 * Input is suspended until the load completes (or is cancelled).
	 *
	 * @param LayerName The layer that receives the widget.
	 * @param bSuspendInputUntilComplete Whether input is suspended during loading.
	 * @param ActivatableWidgetClass The widget class to load and create.
	 * @param InitFunc Optional initialization invoked for the created widget.
	 * @return Streaming handle, cancel it to abort and resume input immediately.
	 */
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(
		FGameplayTag LayerName,
		bool bSuspendInputUntilComplete,
		TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
		TFunction<void(UCommonActivatableWidget&)> InitFunc = nullptr);

	/**
	 * Searches all layers and removes a widget.
	 *
	 * @param ActivatableWidget The widget to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/**
	 * Resolves the stack container registered for a layer.
	 *
	 * @param LayerName The layer to resolve.
	 * @return The registered container, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Layout")
	UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName);

	/**
	 * Sets whether the layout is dormant.
	 *
	 * @param bDormant Whether the layout collapses and accepts persistent input only.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void SetIsDormant(bool bDormant);
	bool IsDormant() const { return bIsDormant; }

	/**
	 * Hides a layer by setting its visibility to Collapsed.
	 *
	 * @param LayerName The layer to hide.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void HideLayer(FGameplayTag LayerName);

	/**
	 * Shows a layer by setting its visibility to SelfHitTestInvisible.
	 *
	 * @param LayerName The layer to show.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void ShowLayer(FGameplayTag LayerName);

	/**
	 * Checks whether a layer is visible.
	 *
	 * @param LayerName The layer to inspect.
	 * @return True when the layer is visible.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Layout")
	bool IsLayerVisible(FGameplayTag LayerName) const;
	/**
	 * Registers a layer container with the layout.
	 *
	 * @param LayerTag The tag that identifies the layer.
	 * @param LayerWidget The container registered for the layer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag,
	                   UCommonActivatableWidgetContainerBase* LayerWidget);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
protected:

	// Responds when the layout enters or leaves dormancy.
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|Common UI Foundation|Layout")
	void OnIsDormantChanged();
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:
	/**
	 * Handles transition changes for a registered layer.
	 *
	 * @param Widget The layer container whose transition changed.
	 * @param bIsTransitioning Whether the container is transitioning.
	 */
	void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

	/**
	 * Handles a Signal Bridge request to push a widget.
	 *
	 * @param SignalTag The received signal tag.
	 * @param Payload The signal payload.
	 */
	UFUNCTION()
	void OnPushWidgetSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/**
	 * Handles a Signal Bridge request to register a layer stack.
	 *
	 * @param SignalTag The received signal tag.
	 * @param Payload The signal payload.
	 */
	UFUNCTION()
	void OnRegisterLayerSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
private:

	// Registered layers indexed by their layer tag.
	UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

	// Stack of active input suspension tokens, one for each in-flight transition.
	TArray<FName> SuspendInputTokens;

	// Whether this layout is currently dormant.
	bool bIsDormant = false;

	// Delegate that receives signal-driven layout requests.
	UPROPERTY()
	FSignalBridgeEventDelegate LayoutSignalDelegate;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};