// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIPolicy.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIExtensions.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/LocalPlayer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousHUD.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"

UGorgeousPrimaryGameLayout::UGorgeousPrimaryGameLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/*static*/ UGorgeousPrimaryGameLayout* UGorgeousPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	return UGorgeousUIPolicy::GetPrimaryPlayerLayout(WorldContextObject);
}

/*static*/ UGorgeousPrimaryGameLayout* UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(const APlayerController* PlayerController)
{
	if (const UGorgeousUIPolicy* Policy = UGorgeousUIPolicy::GetCurrent(PlayerController))
	{
		return Policy->GetRootLayoutForController(PlayerController);
	}
	return nullptr;
}

/*static*/ UGorgeousPrimaryGameLayout* UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(const ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		if (const UGorgeousUIPolicy* Policy = UGorgeousUIPolicy::GetCurrent(LocalPlayer->GetWorld()))
		{
			return Policy->GetRootLayout(LocalPlayer);
		}
	}
	return nullptr;
}

void UGorgeousPrimaryGameLayout::NativeConstruct()
{
	Super::NativeConstruct();

	// Listen for signal-driven widget pushes
	const FGameplayTag PushTag = TAG_Gorgeous_UI_Layout_PushWidget;
	LayoutSignalDelegate.BindDynamic(this, &UGorgeousPrimaryGameLayout::OnPushWidgetSignalReceived);
	
	// Listen for signal-driven layer registration
	const FGameplayTag RegisterTag = TAG_Gorgeous_UI_Layout_RegisterLayer;
	FSignalBridgeEventDelegate RegisterDelegate;
	RegisterDelegate.BindDynamic(this, &UGorgeousPrimaryGameLayout::OnRegisterLayerSignalReceived);

	FGorgeousSignalBridgeAccessRules_S Rules;
	AGorgeousPlayerController* Controller = nullptr;
	if (UWorld* __World = GetWorld())
	{
		if (APlayerController* PC = __World->GetFirstPlayerController())
		{
			Controller = Cast<AGorgeousPlayerController>(PC);
		}
	}
	USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), PushTag, Rules, Controller);
	USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), PushTag, Controller, LayoutSignalDelegate);

	USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), RegisterTag, Rules, Controller);
	USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), RegisterTag, Controller, RegisterDelegate);
}

void UGorgeousPrimaryGameLayout::NativeDestruct()
{
	AGorgeousPlayerController* Controller = nullptr;
	if (UWorld* __World = GetWorld())
	{
		if (APlayerController* PC = __World->GetFirstPlayerController())
		{
			Controller = Cast<AGorgeousPlayerController>(PC);
		}
	}
	USignalBridgeBlueprintFunctionLibrary::Clear(
		GetWorld(), TAG_Gorgeous_UI_Layout_PushWidget, Controller);
	USignalBridgeBlueprintFunctionLibrary::Clear(
		GetWorld(), TAG_Gorgeous_UI_Layout_RegisterLayer, Controller);
	Super::NativeDestruct();
}

void UGorgeousPrimaryGameLayout::SetIsDormant(bool bDormant)
{
	if (bIsDormant != bDormant)
	{
		bIsDormant = bDormant;
		OnIsDormantChanged();
	}
}

void UGorgeousPrimaryGameLayout::OnIsDormantChanged_Implementation()
{
	SetVisibility(bIsDormant ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

UCommonActivatableWidget* UGorgeousPrimaryGameLayout::PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
{
	if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
	{
		return Layer->AddWidget(ActivatableWidgetClass);
	}
	GT_W_LOG("GT.UI.Layout", TEXT("GorgeousPrimaryGameLayout: Layer '%s' not registered. Did you call RegisterLayer?"), *LayerName.ToString());
	return nullptr;
}

TSharedPtr<FStreamableHandle> UGorgeousPrimaryGameLayout::PushWidgetToLayerStackAsync(
	FGameplayTag LayerName,
	bool bSuspendInputUntilComplete,
	TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
	TFunction<void(UCommonActivatableWidget&)> InitFunc)
{
	static FName NAME_AsyncPush(TEXT("AsyncWidgetPush"));
	const FName SuspendToken = bSuspendInputUntilComplete
		? UGorgeousUIExtensions::SuspendGorgeousInput(GetOwningPlayer(), NAME_AsyncPush)
		: NAME_None;

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	TSharedPtr<FStreamableHandle> Handle = Streamable.RequestAsyncLoad(
		ActivatableWidgetClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, LayerName, ActivatableWidgetClass, SuspendToken, InitFunc]()
		{
			if (SuspendToken != NAME_None)
			{
				UGorgeousUIExtensions::ResumeGorgeousInput(GetOwningPlayer(), SuspendToken);
			}

			if (UClass* LoadedClass = ActivatableWidgetClass.Get())
			{
				if (UCommonActivatableWidget* Widget = PushWidgetToLayerStack(LayerName, LoadedClass))
				{
					if (InitFunc)
					{
						InitFunc(*Widget);
					}
				}
			}
		}));

	// Bind cancel delegate, always resume input even if the async load is aborted
	if (Handle.IsValid())
	{
		Handle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this, [this, SuspendToken]()
		{
			if (SuspendToken != NAME_None)
			{
				UGorgeousUIExtensions::ResumeGorgeousInput(GetOwningPlayer(), SuspendToken);
			}
		}));
	}

	return Handle;
}

void UGorgeousPrimaryGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget) return;
	for (auto& Pair : Layers)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveWidget(*ActivatableWidget);
		}
	}
}

UCommonActivatableWidgetContainerBase* UGorgeousPrimaryGameLayout::GetLayerWidget(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}

void UGorgeousPrimaryGameLayout::HideLayer(FGameplayTag LayerName)
{
	if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
	{
		Layer->SetVisibility(ESlateVisibility::Collapsed);
		
		// Refresh action bar when layer visibility changes
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
			{
				HUD->RefreshActionBar();
			}
		}
	}
}

void UGorgeousPrimaryGameLayout::ShowLayer(FGameplayTag LayerName)
{
	if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
	{
		Layer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		
		// Refresh action bar when layer visibility changes
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
			{
				HUD->RefreshActionBar();
			}
		}
	}
}

bool UGorgeousPrimaryGameLayout::IsLayerVisible(FGameplayTag LayerName) const
{
	if (const UCommonActivatableWidgetContainerBase* Layer = Layers.FindRef(LayerName))
	{
		return Layer->GetVisibility() != ESlateVisibility::Collapsed && Layer->GetVisibility() != ESlateVisibility::Hidden;
	}
	return false;
}

void UGorgeousPrimaryGameLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!LayerTag.IsValid() || !LayerWidget) return;
	if (IsDesignTime()) return;

	Layers.Add(LayerTag, LayerWidget);
	// FIX: Use correct API, OnTransitioningChanged, not OnWillRefresh
	LayerWidget->OnTransitioningChanged.AddUObject(this, &UGorgeousPrimaryGameLayout::OnWidgetStackTransitioning);
	// Zero transition duration to prevent gamepad focus issues (matches Lyra)
	LayerWidget->SetTransitionDuration(0.0);
}

void UGorgeousPrimaryGameLayout::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	FGameplayTag LayerTag;
	for (const auto& Pair : Layers)
	{
		if (Pair.Value == Widget)
		{
			LayerTag = Pair.Key;
			break;
		}
	}

	// Do not suspend gameplay input for HUD, Action Bar, or Message/Notification layers
	if (LayerTag == TAG_Gorgeous_UI_Layer_HUD || 
		LayerTag == TAG_Gorgeous_UI_Layer_ActionBar || 
		LayerTag == TAG_Gorgeous_UI_Layer_Message)
	{
		// Refresh action bar when transition completes
		if (!bIsTransitioning)
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
				{
					HUD->RefreshActionBar();
				}
			}
		}
		return;
	}

	if (bIsTransitioning)
	{
		static FName NAME_Transition(TEXT("StackTransition"));
		const FName SuspendToken = UGorgeousUIExtensions::SuspendGorgeousInput(GetOwningPlayer(), NAME_Transition);
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UGorgeousUIExtensions::ResumeGorgeousInput(GetOwningPlayer(), SuspendToken);
		}

		// Refresh action bar when a layer transition finishes (active widget changes)
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
			{
				HUD->RefreshActionBar();
			}
		}
	}
}

void UGorgeousPrimaryGameLayout::OnPushWidgetSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const FGorgeousUIMessageRequest* Request = Payload.GetPtr<FGorgeousUIMessageRequest>();
	if (!Request || !Request->Config) return;

	PushWidgetToLayerStackAsync(Request->Config->LayerTag, true, Request->Config->MessageWidgetClass);
}

void UGorgeousPrimaryGameLayout::OnRegisterLayerSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const FGorgeousRegisterLayerPayload* RegisterData = Payload.GetPtr<FGorgeousRegisterLayerPayload>();
	if (!RegisterData || !RegisterData->LayerTag.IsValid()) return;

	if (UCommonActivatableWidgetContainerBase* Container = Cast<UCommonActivatableWidgetContainerBase>(RegisterData->LayerWidget))
	{
		RegisterLayer(RegisterData->LayerTag, Container);
	}
}
