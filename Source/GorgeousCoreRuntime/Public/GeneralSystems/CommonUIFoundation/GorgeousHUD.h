#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousInputConsumer_I.h"
#include "CommonActivatableWidget.h"
#include "GorgeousHUD.generated.h"

//@TODO: Show Notification calls, as the hud is the primary owner for ui, this is a great place to show notifications

struct FInputActionInstance;
/**
 * Centralized manager for the Gorgeous UI layer and input dispatching.
 * Acts as the bridge between Enhanced Input/CommonUI and IGorgeousInputConsumer_I.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API AGorgeousHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGorgeousHUD();

	// AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor Interface
	
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Input")
	bool IsInputConsumerRegistered(UObject* Object);

	/** 
	 * Registers a consumer to receive tag-based input. 
	 * @param Consumer The object implementing IGorgeousInputConsumer_I.
	 * @param Context A tag representing the context (e.g., UI.Context.Inventory). Used for conflict detection.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Input")
	void RegisterInputConsumer(UObject* Consumer, FGameplayTag Context);

	/** Unregisters a consumer. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Input")
	void UnregisterInputConsumer(UObject* Consumer);

	/** Rebuilds the action bar hints based on current bindings and theme. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|UI")
	void RefreshActionBar();

	/** Dispatches a tag-based input to all registered consumers. */
	bool DispatchGorgeousInput(FGameplayTag ActionTag, const FInputActionInstance& Instance, bool bConsumeInput);

	/** Helper to retrieve the primary game layout from the local player. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|UI")
	class UGorgeousPrimaryGameLayout* GetPrimaryLayout() const;

	/** Pushes a widget to a specific layout layer via the primary layout. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|UI")
	UCommonActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

protected:
	/** Fired when the Primary Game Layout is finally available for this player. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous|UI")
	void OnPrimaryLayoutReady(class UGorgeousPrimaryGameLayout* Layout);

	/** Fired when the Action Bar widget has been successfully created and added to the UI. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous|UI")
	void OnActionBarCreated(class UGorgeousActionBar_CAW* ActionBar);

protected:
	/** Tracks active consumers and their contexts. */
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

	/** Checks for input conflicts within the same context and triggers a toast if found. */
	void CheckForInputConflicts(FGameplayTag ActionTag, FGameplayTag Context, int32 Priority, UObject* FirstConsumer, UObject* SecondConsumer) const;

	/** The class of the action bar widget to spawn. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous|UI")
	TSubclassOf<class UGorgeousActionBar_CAW> ActionBarClass;

	/** The layer to push the action bar onto. If None, it will be added to viewport. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous|UI", meta = (Categories = "UI.Layer"))
	FGameplayTag ActionBarLayerTag;

	/** The instantiated action bar widget. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|UI")
	TObjectPtr<class UGorgeousActionBar_CAW> ActionBarWidget;

private:
	UFUNCTION()
	void OnLayoutCreated(ULocalPlayer* LocalPlayer, UGorgeousPrimaryGameLayout* Layout);
};
