// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
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
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStats.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Active UI Overlays"), STAT_GUI_ActiveOverlays, STATGROUP_GorgeousUIFoundation);
DECLARE_CYCLE_STAT(TEXT("Push Widget To Layer"), STAT_GUI_PushWidget, STATGROUP_GorgeousUIFoundation);
DECLARE_CYCLE_STAT(TEXT("Register UI Layer"), STAT_GUI_RegisterLayer, STATGROUP_GorgeousUIFoundation);

static int32 GActiveUIOverlaysCount = 0;

int32 UGorgeousPrimaryGameLayout::GetTotalActiveOverlays()
{
	return GActiveUIOverlaysCount;
}

void UGorgeousPrimaryGameLayout::IncrementActiveOverlays()
{
	GActiveUIOverlaysCount++;
	INC_DWORD_STAT(STAT_GUI_ActiveOverlays);
}

void UGorgeousPrimaryGameLayout::DecrementActiveOverlays()
{
	GActiveUIOverlaysCount--;
	DEC_DWORD_STAT(STAT_GUI_ActiveOverlays);
}

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
	RegisterLayerDelegate.BindDynamic(this, &UGorgeousPrimaryGameLayout::OnRegisterLayerSignalReceived);

	FGorgeousSignalBridgeAccessRules_S Rules;
	Rules.bNetworked = false;
	AGorgeousPlayerController* Controller = nullptr;
	if (UWorld* __World = GetWorld())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			Controller = Cast<AGorgeousPlayerController>(PC);
		}
	}
	USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), PushTag, Rules, nullptr);
	USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), PushTag, nullptr, LayoutSignalDelegate);

	USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), RegisterTag, Rules, nullptr);
	USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), RegisterTag, nullptr, RegisterLayerDelegate);
}

void UGorgeousPrimaryGameLayout::NativeDestruct()
{
	AGorgeousPlayerController* Controller = nullptr;
	if (UWorld* __World = GetWorld())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			Controller = Cast<AGorgeousPlayerController>(PC);
		}
	}
	USignalBridgeBlueprintFunctionLibrary::Unlisten(this, TAG_Gorgeous_UI_Layout_PushWidget, nullptr, LayoutSignalDelegate);
	USignalBridgeBlueprintFunctionLibrary::Unlisten(this, TAG_Gorgeous_UI_Layout_RegisterLayer, nullptr, RegisterLayerDelegate);
	for (auto& Entry : TrackedOverlays) { if (Entry.IsValid()) Entry->OnDeactivated().RemoveAll(this); DecrementActiveOverlays(); }
	TrackedOverlays.Reset();
	for (FName Token : SuspendInputTokens) UGorgeousUIExtensions::ResumeGorgeousInput(GetOwningPlayer(), Token);
	SuspendInputTokens.Reset();
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
	SCOPE_CYCLE_COUNTER(STAT_GUI_PushWidget);
	if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
	{
		return Layer->AddWidget<UCommonActivatableWidget>(ActivatableWidgetClass, [this](UCommonActivatableWidget& Widget) { TrackOverlay(&Widget); });
	}
	GT_W_LOG("GT.UI.Layout", TEXT("GorgeousPrimaryGameLayout: Layer '%s' not registered. Did you call RegisterLayer?"), *LayerName.ToString());
	return nullptr;
}

TSharedPtr<FStreamableHandle> UGorgeousPrimaryGameLayout::PushWidgetToLayerStackAsync(
	FGameplayTag LayerName, bool bSuspendInputUntilComplete,
	TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
	TFunction<void(UCommonActivatableWidget&)> InitFunc, TFunction<void()> FailureFunc)
{
	if (ActivatableWidgetClass.IsNull() || !GetLayerWidget(LayerName)) { if (FailureFunc) FailureFunc(); return nullptr; }
	const TWeakObjectPtr<APlayerController> Controller = GetOwningPlayer();
	const TWeakObjectPtr<UGorgeousPrimaryGameLayout> WeakThis(this);
	const FName SuspendToken = bSuspendInputUntilComplete ? UGorgeousUIExtensions::SuspendGorgeousInput(Controller.Get(), TEXT("AsyncWidgetPush")) : NAME_None;
	auto Resume = [Controller, SuspendToken] { UGorgeousUIExtensions::ResumeGorgeousInput(Controller.Get(), SuspendToken); };
	auto Complete = [WeakThis, LayerName, ActivatableWidgetClass, InitFunc, FailureFunc, Resume]
	{
		Resume();
		if (auto* Layout = WeakThis.Get())
		{
			if (UClass* Class = ActivatableWidgetClass.Get()) if (auto* Layer = Layout->GetLayerWidget(LayerName))
			{
				if (Layer->AddWidget<UCommonActivatableWidget>(Class, [Layout, &InitFunc](UCommonActivatableWidget& Widget)
				{ Layout->TrackOverlay(&Widget); if (InitFunc) InitFunc(Widget); })) return;
			}
		}
		if (FailureFunc) FailureFunc();
	};
	auto Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateLambda(Complete));
	if (Handle) Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([Resume, FailureFunc] { Resume(); if (FailureFunc) FailureFunc(); }));
	else { Resume(); if (FailureFunc) FailureFunc(); }
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
	SCOPE_CYCLE_COUNTER(STAT_GUI_RegisterLayer);
	if (!LayerTag.IsValid() || !LayerWidget) return;
	if (IsDesignTime()) return;

	Layers.Add(LayerTag, LayerWidget);
	// FIX: Use correct API, OnTransitioningChanged, not OnWillRefresh
	LayerWidget->OnTransitioningChanged.RemoveAll(this);
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
	if (auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(this); !Subsystem || (Request->LocalPlayerIndex == INDEX_NONE ? 0 : Request->LocalPlayerIndex) != Subsystem->GetLocalPlayerIndex()) return;

	const FGorgeousUIMessageRequest DialogRequest = *Request;
	PushWidgetToLayerStackAsync(Request->Config->LayerTag, true, Request->Config->MessageWidgetClass,
		[DialogRequest](UCommonActivatableWidget& Widget)
		{
			if (UGorgeousActivatableWidget* Dialog = Cast<UGorgeousActivatableWidget>(&Widget))
			{
				Dialog->MessageRequest = DialogRequest;
				Dialog->InitializeMessage(DialogRequest);
			}
		}, [Context = TWeakObjectPtr<UGorgeousUIFoundationSubsystem>(UGorgeousUIFoundationSubsystem::GetForContext(this)), DialogRequest]
		{
			if (auto* UI = Context.Get())
			{
				UI->CompleteMessagePresentation(DialogRequest.RequestID);
				FGorgeousUIMessageResult Result; Result.RequestID = DialogRequest.RequestID; Result.ResultTag = TEXT("Cancel");
				USignalBridgeBlueprintFunctionLibrary::DispatchLocal(UI, TAG_Gorgeous_UI_System_Message_Result, FInstancedStruct::Make(Result));
			}
		});
}

void UGorgeousPrimaryGameLayout::OnRegisterLayerSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const FGorgeousRegisterLayerPayload* RegisterData = Payload.GetPtr<FGorgeousRegisterLayerPayload>();
	if (!RegisterData || !RegisterData->LayerTag.IsValid()) return;

	if (UCommonActivatableWidgetContainerBase* Container = Cast<UCommonActivatableWidgetContainerBase>(RegisterData->LayerWidget))
	{
		if (UGorgeousUIFoundationSubsystem::GetForContext(Container) == UGorgeousUIFoundationSubsystem::GetForContext(this)) RegisterLayer(RegisterData->LayerTag, Container);
	}
}

void UGorgeousPrimaryGameLayout::TrackOverlay(UCommonActivatableWidget* Widget)
{
	if (!Widget || TrackedOverlays.Contains(Widget)) return;
	TrackedOverlays.Add(Widget);
	IncrementActiveOverlays();
	Widget->OnDeactivated().RemoveAll(this);
	Widget->OnDeactivated().AddWeakLambda(this, [this, Weak = TWeakObjectPtr<UCommonActivatableWidget>(Widget)]
	{ if (TrackedOverlays.Remove(Weak)) DecrementActiveOverlays(); });
}
