// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousHUD.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIPolicy.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActionBar_CAW.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Algo/Sort.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "EnhancedInputSubsystems.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "InputMappingContext.h"

AGorgeousHUD::AGorgeousHUD()
{
	ActionBarLayerTag = TAG_Gorgeous_UI_Layer_ActionBar;
}

void AGorgeousHUD::BeginPlay()
{
	Super::BeginPlay();

	if (ActionBarClass)
	{
		if (UGorgeousPrimaryGameLayout* Layout = GetPrimaryLayout())
		{
			// Layout already exists, push immediately
			OnPrimaryLayoutReady(Layout);

			if (ActionBarLayerTag.IsValid())
			{
				ActionBarWidget = Cast<UGorgeousActionBar_CAW>(PushWidgetToLayer(ActionBarLayerTag, ActionBarClass));
			}
			else
			{
				ActionBarWidget = CreateWidget<UGorgeousActionBar_CAW>(GetOwningPlayerController(), ActionBarClass);
				if (ActionBarWidget)
				{
					ActionBarWidget->AddToViewport(100);
				}
			}

			if (ActionBarWidget)
			{
				OnActionBarCreated(ActionBarWidget);
			}
		}
		else
		{
			// Layout doesn't exist yet, wait for it
			if (UGorgeousUIPolicy* Policy = UGorgeousUIPolicy::GetCurrent(this))
			{
				Policy->OnLayoutCreatedDelegate.AddDynamic(this, &AGorgeousHUD::OnLayoutCreated);
			}
		}
	}
	
	// Push Default IMC Bindings
	const ULocalPlayer* LP = GetOwningPlayerController()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	GT_E_LOG_ENSURE(InputSubsystem, "GT.Core.CommonUIFoundation.HUD.DefaultInputMapping", TEXT("Could not find EnhancedInputLocalPlayerSubsystem as InputSubsystem"));

	const auto FoundationSettings = GetDefault<UGorgeousUIFoundationSettings>();
	
	if (FoundationSettings->DefaultInputBindings.LoadSynchronous())
	{
		for (const FGorgeousInputMappingConfig_S& Config : FoundationSettings->DefaultInputBindings->InputMappingContexts)
		{
			if (const UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
			{
				InputSubsystem->AddMappingContext(IMC, Config.Priority);
			}
		}	
	}
	
	LP->GetSubsystem<UGorgeousUIFoundationSubsystem>()->SetActiveInputBindings(FoundationSettings->DefaultInputBindings.Get());
}

void AGorgeousHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ActiveConsumers.Empty();
	ConsumerReferences.Empty();

	if (UGorgeousUIPolicy* Policy = UGorgeousUIPolicy::GetCurrent(this))
	{
		Policy->OnLayoutCreatedDelegate.RemoveDynamic(this, &AGorgeousHUD::OnLayoutCreated);
	}

	Super::EndPlay(EndPlayReason);
}

bool AGorgeousHUD::IsInputConsumerRegistered(UObject* Object)
{
	return ActiveConsumers.ContainsByPredicate([Object](const FInputConsumerEntry& Entry) {
		return Entry.Consumer.GetObject() == Object;
	});
}

void AGorgeousHUD::RegisterInputConsumer(UObject* Consumer, FGameplayTag Context)
{
	if (!Consumer || !Consumer->Implements<UGorgeousInputConsumer_I>())
	{
		return;
	}

	// Check if already registered
	for (const auto& Entry : ActiveConsumers)
	{
		if (Entry.Consumer.GetObject() == Consumer)
		{
			return;
		}
	}

	FInputConsumerEntry NewEntry;
	NewEntry.Consumer = Consumer;
	NewEntry.Context = Context;
	NewEntry.Priority = IGorgeousInputConsumer_I::Execute_GetInputConsumerPriority(Consumer);

	ActiveConsumers.Add(NewEntry);
	ConsumerReferences.Add(Consumer);

	// Sort by priority
	Algo::Sort(ActiveConsumers);

	RefreshActionBar();
}

void AGorgeousHUD::UnregisterInputConsumer(UObject* Consumer)
{
	ActiveConsumers.RemoveAll([Consumer](const FInputConsumerEntry& Entry) {
		return Entry.Consumer.GetObject() == Consumer;
	});
	ConsumerReferences.Remove(Consumer);

	RefreshActionBar();
}

void AGorgeousHUD::RefreshActionBar()
{
	if (!ActionBarWidget) return;

	UGorgeousUIFoundationSubsystem* Subsystem = GetOwningPlayerController() ? GetOwningPlayerController()->GetLocalPlayer()->GetSubsystem<UGorgeousUIFoundationSubsystem>() : nullptr;
	if (!Subsystem) return;

	TArray<UGorgeousInputBinding_DA*> Bindings = Subsystem->GetActiveInputBindings();
	TArray<UGorgeousUITheme_DA*> Themes = Subsystem->GetCurrentThemes();
	FName PlatformName = Subsystem->GetCurrentPlatformName();

	if (Bindings.IsEmpty())
	{
		GT_W_LOG("GT.HUD", TEXT("RefreshActionBar: Skipping refresh. Bindings: %s, Theme: %s"), 
			Bindings.IsEmpty() ? TEXT("Empty") : TEXT("Valid Data"), 
			Themes.IsEmpty() ? TEXT("Empty") : TEXT("Valid Data"));
		return;
	}
	
	//Flatten the map out for easier access
	TMap<FGameplayTag, FGorgeousInputBindingInfo_S> FlattenedBindings;
	for (auto Binding : Bindings)
	{
		FlattenedBindings.Append(Binding->Bindings);
	}
	
	GT_I_LOG("GT.HUD", TEXT("RefreshActionBar: Refreshing Action Bar with %d bindings for platform '%s'"), 
		FlattenedBindings.Num(), *PlatformName.ToString());

	TArray<FGorgeousActionBarEntry_S> Entries;

	// Get a set of tags that have at least one active consumer
	TSet<FGameplayTag> ActiveConsumerTags;
	for (const auto& Entry : ActiveConsumers)
	{
		ActiveConsumerTags.Add(Entry.Context); 
		// Wait, Entry.Context is the context tag (e.g. UI.Context.Inventory).
		// We want to know if the consumer handles specific ActionTags.
		// For now, let's assume if a consumer is registered in a context, 
		// we show all "Active" actions for that context.
	}
	
	UGorgeousPrimaryGameLayout* Layout = GetPrimaryLayout();

	for (const auto& Pair : FlattenedBindings)
	{
		const FGameplayTag& ActionTag = Pair.Key;
		const FGorgeousInputBindingInfo_S& Info = Pair.Value;

		if (Info.bShouldDisplayInActionBar)
		{
			// Check layer context-awareness if AllowedActionBarLayers is specified
			if (!Info.AllowedActionBarLayers.IsEmpty())
			{
				bool bLayerIsActiveAndVisible = false;
				if (Layout)
				{
					for (const FGameplayTag& LayerTag : Info.AllowedActionBarLayers)
					{
						if (Layout->IsLayerVisible(LayerTag))
						{
							if (const UCommonActivatableWidgetContainerBase* Container = Layout->GetLayerWidget(LayerTag))
							{
								if (Container->GetActiveWidget() != nullptr)
								{
									bLayerIsActiveAndVisible = true;
									break;
								}
							}
						}
					}
				}
				if (!bLayerIsActiveAndVisible)
				{
					GT_I_LOG("GT.HUD", TEXT("RefreshActionBar: Skipping Action '%s' (not allowed in current pushed layers)."), *ActionTag.ToString());
					continue; // Skip displaying this action in the action bar
				}
			}

			// Check if this action belongs to an active context or is global
			// For simplicity, we currently show all enabled bindings in the active BindingAsset.
			// The user can swap BindingAssets to change what's shown.
		
			FGorgeousActionBarEntry_S Entry;
			// Reverse iterate to fetch resources from the most recent theme first
			for (int32 i = Themes.Num() - 1; i >= 0; --i)
			{
				if (const FSlateBrush Brush = Themes[i]->GetActionIcon(ActionTag, PlatformName); 
					Brush.GetResourceName() != NAME_None)
				{
					Entry.Icon = Brush;
					break;
				}
			}
			Entry.ActionName = Info.DisplayName.IsEmpty() ? FText::FromName(ActionTag.GetTagName()) : Info.DisplayName;
			Entries.Add(Entry);

			GT_I_LOG("GT.HUD", TEXT("RefreshActionBar: Added Action '%s' to Bar."), *ActionTag.ToString());
		}
		else
		{
			GT_I_LOG("GT.HUD", TEXT("RefreshActionBar: Skipping Action '%s' (bShouldDisplayInActionBar is false)."), *ActionTag.ToString());
		}
	}

	GT_I_LOG("GT.HUD", TEXT("RefreshActionBar: Sending %d entries to Action Bar widget."), Entries.Num());
	ActionBarWidget->UpdateActionBar(Entries);
}

bool AGorgeousHUD::DispatchGorgeousInput(FGameplayTag ActionTag, const FInputActionInstance& Instance, bool bConsumeInput)
{
	bool bHandled = false;
	
	struct FFrameHandlerTracker
	{
		FGameplayTag Context;
		int32 Priority;
		UObject* Consumer;
	};
	TArray<FFrameHandlerTracker> HandlersThisFrame;

	for (const auto& Entry : ActiveConsumers)
	{
		// Conditional Pass-Through: If a higher priority layer already handled this input,
		// and the binding rules specify consumption, skip lower priority layers entirely.
		if (bHandled && bConsumeInput)
		{
			break;
		}

		UObject* ConsumerObj = Entry.Consumer.GetObject();
		if (!IsValid(ConsumerObj)) continue;

		// 1. Try Advanced Interface
		bool bConsumerHandled = Entry.Consumer->Execute_HandleGorgeousInputAdvanced(ConsumerObj, ActionTag, Instance);

		// 2. Fallback to Simple Interface
		if (!bConsumerHandled)
		{
			ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
			if (TriggerEvent == ETriggerEvent::Triggered)
			{
				bConsumerHandled = Entry.Consumer->Execute_HandleGorgeousInput(ConsumerObj, ActionTag, Instance.GetValue());
			}
		}

		// 3. Priority-Aware Conflict Detection
		if (bConsumerHandled)
		{
			bHandled = true;

			UObject* ConflictingConsumer = nullptr;
			for (const auto& Tracked : HandlersThisFrame)
			{
				if (Tracked.Context == Entry.Context && Tracked.Priority == Entry.Priority)
				{
					ConflictingConsumer = Tracked.Consumer;
					break;
				}
			}

			if (ConflictingConsumer)
			{
				CheckForInputConflicts(ActionTag, Entry.Context, Entry.Priority, ConflictingConsumer, ConsumerObj);
			}
			else
			{
				HandlersThisFrame.Add({ Entry.Context, Entry.Priority, ConsumerObj });
			}
		}
	}

	return bHandled;
}

void AGorgeousHUD::CheckForInputConflicts(FGameplayTag ActionTag, FGameplayTag Context, int32 Priority, UObject* FirstConsumer, UObject* SecondConsumer) const
{
	FString FirstConsumerName  = FirstConsumer  ? FirstConsumer->GetName()  : TEXT("Unknown");
	FString SecondConsumerName = SecondConsumer ? SecondConsumer->GetName() : TEXT("Unknown");

	/*GT_W_LOG_FULL_EX(
		*FString::Printf(TEXT("InputConflict.%s"), *ActionTag.ToString()),
		TEXT("Input Conflict! Both [%s] and [%s] in context '%s' are fighting for Priority Tier %d on Action '%s'."),
		4.f,
		true,  // Screen
		true,  // Log
		true,  // Override
		true,  // TOAST
		this,
		nullptr,
		*FirstConsumerName,
		*SecondConsumerName,
		*Context.ToString(),
		*ActionTag.ToString()
	);*/
}

UGorgeousPrimaryGameLayout* AGorgeousHUD::GetPrimaryLayout() const
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		return UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(PC);
	}
	return nullptr;
}

UCommonActivatableWidget* AGorgeousHUD::PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (UGorgeousPrimaryGameLayout* Layout = GetPrimaryLayout())
	{
		return Layout->PushWidgetToLayerStack(LayerTag, WidgetClass);
	}

	GT_W_LOG("GT.HUD", TEXT("AGorgeousHUD::PushWidgetToLayer: Failed to find PrimaryGameLayout."));
	return nullptr;
}

void AGorgeousHUD::OnLayoutCreated(ULocalPlayer* LocalPlayer, UGorgeousPrimaryGameLayout* Layout)
{
	if (LocalPlayer && LocalPlayer == (GetOwningPlayerController() ? GetOwningPlayerController()->GetLocalPlayer() : nullptr))
	{
		// Unsubscribe now that we found our layout
		if (UGorgeousUIPolicy* Policy = UGorgeousUIPolicy::GetCurrent(this))
		{
			Policy->OnLayoutCreatedDelegate.RemoveDynamic(this, &AGorgeousHUD::OnLayoutCreated);
		}

		// Notify BP that the layout is ready
		OnPrimaryLayoutReady(Layout);

		if (ActionBarClass && !ActionBarWidget)
		{
			if (ActionBarLayerTag.IsValid())
			{
				ActionBarWidget = Cast<UGorgeousActionBar_CAW>(PushWidgetToLayer(ActionBarLayerTag, ActionBarClass));
			}
			else
			{
				ActionBarWidget = CreateWidget<UGorgeousActionBar_CAW>(GetOwningPlayerController(), ActionBarClass);
				if (ActionBarWidget)
				{
					ActionBarWidget->AddToViewport(100);
				}
			}

			if (ActionBarWidget)
			{
				OnActionBarCreated(ActionBarWidget);
			}

			// Force a refresh now that the widget exists
			RefreshActionBar();
		}
	}
}
