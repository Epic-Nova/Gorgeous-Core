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
#include "GameFramework/HUD.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousInputConsumer_I.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "CommonActivatableWidget.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousHUD.generated.h"
//<-------------------------------------------------------------------------->

//@TODO: Show Notification calls, as the hud is the primary owner for ui, this is a great place to show notifications

struct FInputActionInstance;

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous HUD
| Functional Name: AGorgeousHUD
| Parent Class: AHUD
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Centralized manager for the Gorgeous UI layer and input dispatching. Acts
| as the bridge between Enhanced Input/CommonUI and
| IGorgeousInputConsumer_I.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/AGorgeousHUD",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API AGorgeousHUD : public AHUD
{
	GENERATED_BODY()

public:

	// Initializes the HUD's input-consumer and action-bar state.
	AGorgeousHUD();


	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	//<----------------------=== Actor Overrides ===---------------------------->

	// Initializes UI services after the actor enters play.
	virtual void BeginPlay() override;

	// Releases UI services before the actor leaves play.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//<----------------------=== End Actor Overrides ===------------------------>
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Checks whether an object is registered as an input consumer.
	 *
	 * @param Object The object to inspect.
	 * @return True when the object is registered as an input consumer.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Input")
	bool IsInputConsumerRegistered(UObject* Object);

	/**
	 * Registers a consumer to receive tag-based input.
	 * @param Consumer The object implementing IGorgeousInputConsumer_I.
	 * @param Context A tag representing the context (e.g., UI.Context.Inventory). Used for conflict detection.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Input")
	void RegisterInputConsumer(UObject* Consumer, FGameplayTag Context);

	/**
	 * Unregisters an input consumer.
	 *
	 * @param Consumer The consumer to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Input")
	void UnregisterInputConsumer(UObject* Consumer);

	// Rebuilds action-bar hints for the current bindings and theme.
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|UI")
	void RefreshActionBar();

	/**
	 * Dispatches tag-based input to registered consumers.
	 *
	 * @param ActionTag The tag that identifies the input action.
	 * @param Instance The Enhanced Input action instance.
	 * @param bConsumeInput Whether a receiving consumer may consume the input.
	 * @return True when a consumer handles the input.
	 */
	bool DispatchGorgeousInput(FGameplayTag ActionTag, const FInputActionInstance& Instance, bool bConsumeInput);

	/**
	 * Retrieves the primary game layout for the local player.
	 *
	 * @return The primary layout, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|UI")
	class UGorgeousPrimaryGameLayout* GetPrimaryLayout() const;

	/**
	 * Pushes a widget onto a layer of the primary layout.
	 *
	 * @param LayerTag The layer that receives the widget.
	 * @param WidgetClass The widget class to create.
	 * @return The pushed widget, or null when it cannot be created.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|UI")
	UCommonActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
protected:
	/**
	 * Fires when the primary layout becomes available for this player.
	 *
	 * @param Layout The created primary layout.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous|UI")
	void OnPrimaryLayoutReady(class UGorgeousPrimaryGameLayout* Layout);

	/**
	 * Fires after the action bar is created and added to the UI.
	 *
	 * @param ActionBar The created action bar widget.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous|UI")
	void OnActionBarCreated(class UGorgeousActionBar_CAW* ActionBar);
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks


	//<============================--- Variables ---============================>
	#pragma region Variables
protected:

	// Tracks an input consumer, its context, and its dispatch priority.
	struct FInputConsumerEntry
	{
		TScriptInterface<IGorgeousInputConsumer_I> Consumer;
		FGameplayTag Context;
		int32 Priority = 0;

		bool operator<(const FInputConsumerEntry& Other) const
		{
			return Priority > Other.Priority; // Descending order
		}
	};

	UPROPERTY()
	TArray<UObject*> ConsumerReferences; // Keep references for GC

	TArray<FInputConsumerEntry> ActiveConsumers;

	/**
	 * Checks a context for conflicting input consumers and reports conflicts.
	 *
	 * @param ActionTag The input action being registered.
	 * @param Context The context in which to search for conflicts.
	 * @param Priority The priority assigned to the incoming consumer.
	 * @param FirstConsumer The first consumer involved in the conflict.
	 * @param SecondConsumer The second consumer involved in the conflict.
	 */
	void CheckForInputConflicts(FGameplayTag ActionTag, FGameplayTag Context, int32 Priority, UObject* FirstConsumer, UObject* SecondConsumer) const;

	// The class of the action bar widget to spawn.
	UPROPERTY(EditAnywhere, Category = "Gorgeous|UI")
	TSubclassOf<class UGorgeousActionBar_CAW> ActionBarClass;

	// The layer to push the action bar onto. If None, it will be added to viewport.
	UPROPERTY(EditAnywhere, Category = "Gorgeous|UI", meta = (Categories = "UI.Layer"))
	FGameplayTag ActionBarLayerTag;

	// The instantiated action bar widget.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|UI")
	TObjectPtr<class UGorgeousActionBar_CAW> ActionBarWidget;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:
	/**
	 * Handles creation of a primary layout for a local player.
	 *
	 * @param LocalPlayer The player that owns the layout.
	 * @param Layout The created layout.
	 */
	UFUNCTION()
	void OnLayoutCreated(ULocalPlayer* LocalPlayer, UGorgeousPrimaryGameLayout* Layout);


	// Restores focus to the game viewport.
	void ResetFocusToGame();
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};