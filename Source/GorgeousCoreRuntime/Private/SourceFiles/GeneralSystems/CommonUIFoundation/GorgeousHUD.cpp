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
#include "CommonActivatableWidget.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"

AGorgeousHUD::AGorgeousHUD()
{
	ActionBarLayerTag = TAG_Gorgeous_UI_Layer_HUD;
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

	UGorgeousInputBinding_DA* Bindings = Subsystem->GetActiveInputBindings();
	UGorgeousUITheme_DA* Theme = Subsystem->GetCurrentTheme();
	FName PlatformName = Subsystem->GetCurrentPlatformName();

	if (!Bindings || !Theme)
	{
		GT_W_LOG("GT.HUD", TEXT("RefreshActionBar: Skipping refresh. Bindings: %s, Theme: %s"), 
			Bindings ? TEXT("Valid") : TEXT("NULL"), 
			Theme ? TEXT("Valid") : TEXT("NULL"));
		return;
	}

	GT_I_LOG("GT.HUD", TEXT("RefreshActionBar: Refreshing Action Bar with %d bindings for platform '%s'"), 
		Bindings->Bindings.Num(), *PlatformName.ToString());

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

	for (const auto& Pair : Bindings->Bindings)
	{
		const FGameplayTag& ActionTag = Pair.Key;
		const FGorgeousInputBindingInfo_S& Info = Pair.Value;

		if (Info.bShouldDisplayInActionBar)
		{
			// Check if this action belongs to an active context or is global
			// For simplicity, we currently show all enabled bindings in the active BindingAsset.
			// The user can swap BindingAssets to change what's shown.
			
			FGorgeousActionBarEntry_S Entry;
			Entry.Icon = Theme->GetActionIcon(ActionTag, PlatformName);
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

bool AGorgeousHUD::DispatchGorgeousInput(FGameplayTag ActionTag, const FInputActionValue& Value)
{
	TArray<FGameplayTag> HandlingContexts;
	bool bHandled = false;

	for (const auto& Entry : ActiveConsumers)
	{
		if (Entry.Consumer->Execute_HandleGorgeousInput(Entry.Consumer.GetObject(), ActionTag, Value))
		{
			bHandled = true;
			HandlingContexts.AddUnique(Entry.Context);
			
			// Conflict Detection: If multiple consumers in the SAME context would have handled this,
			// or if multiple active contexts are competing without a clear priority chain.
			// For now, we warn if multiple consumers are handling the same tag in the same context.
		}
	}

	return bHandled;
}

void AGorgeousHUD::CheckForInputConflicts(FGameplayTag ActionTag, FGameplayTag Context)
{
	int32 HandlersInContext = 0;
	for (const auto& Entry : ActiveConsumers)
	{
		if (Entry.Context == Context)
		{
			HandlersInContext++;
		}
	}

	if (HandlersInContext > 1)
	{
		GT_W_LOG_FULL_EX(
			*FString::Printf(TEXT("InputConflict.%s"), *ActionTag.ToString()),
			TEXT("Input Conflict Detected! Multiple consumers in context '%s' are responding to Action '%s'."),
			GT_DURATION,
			true,  // Screen
			true,  // Log
			true,  // Override
			true,  // TOAST
			this,
			nullptr,
			*Context.ToString(),
			*ActionTag.ToString()
		);
	}
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
