#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateUser.h"
#include "UObject/UnrealType.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStats.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidget.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIPolicy.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/World.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/UserWidget.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSignalForwarder.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousHUD.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings.h"
#include "EnhancedInputComponent.h"
#include "UITag.h"
#include "CommonInputSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonTextBlock.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonProgressBar.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
DECLARE_CYCLE_STAT(TEXT("Set UI Theme"), STAT_GUI_SetTheme, STATGROUP_GorgeousUIFoundation);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Total Theme Swaps"), STAT_GUI_ThemeSwaps, STATGROUP_GorgeousUIFoundation);

void UGorgeousUIFoundationSubsystem::Tick(float DeltaTime)
{
	APlayerController* CurrentController = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr;
	if (bSignalsRegistered && (SignalController.Get() != CurrentController || SignalWorld.Get() != GetWorld())) DetachSignalListeners();
	// Lazy registration also retries after controller replacement or map travel.
	if (!bSignalsRegistered)
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (APlayerController* GPC = LP->GetPlayerController(GetWorld()))
			{
				FGorgeousSignalBridgeAccessRules_S Rules;
				Rules.bNetworked = false;
				SignalController = GPC;
				SignalWorld = GetWorld();

				// Focus Routing
				const FGameplayTag FocusRequestTag = TAG_Gorgeous_UI_Focus_Request;
				FocusRequestDelegate.BindDynamic(this, &UGorgeousUIFoundationSubsystem::OnFocusRequestReceived);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), FocusRequestTag, Rules, nullptr);
				USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), FocusRequestTag, nullptr, FocusRequestDelegate);

				// Input Action Routing
				const FGameplayTag InputActionTag = TAG_Gorgeous_UI_Input_Action;
				InputActionDelegate.BindDynamic(this, &UGorgeousUIFoundationSubsystem::OnInputActionReceived);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), InputActionTag, Rules, nullptr);
				USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), InputActionTag, nullptr, InputActionDelegate);

				// Message Routing
				const FGameplayTag MessagePushTag = TAG_Gorgeous_UI_System_Message_Push;
				const FGameplayTag MessageResultTag = TAG_Gorgeous_UI_System_Message_Result;
				MessageRequestDelegate.BindDynamic(this, &UGorgeousUIFoundationSubsystem::OnMessageRequestReceived);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), MessagePushTag, Rules, nullptr);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), MessageResultTag, Rules, nullptr);
				USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), MessagePushTag, nullptr, MessageRequestDelegate);
				MessageResultDelegate.BindDynamic(this, &UGorgeousUIFoundationSubsystem::OnMessageResultReceived);
				USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), MessageResultTag, nullptr, MessageResultDelegate);

				for (const auto& Entry : TagForwarders) if (Entry.Value)
				{
					FSignalBridgeEventDelegate Delegate; Delegate.BindDynamic(Entry.Value.Get(), &UGorgeousSignalBridgeTagForwarder::OnBridgePayload);
					USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), Entry.Key, Rules, nullptr);
					USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), Entry.Key, nullptr, Delegate);
				}
				bSignalsRegistered = true;
				GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Registered system-wide signals for LocalPlayer via Tick lazy-init."));
			}
		}
	}

	// Self-healing check for input bridge setup (in case PlayerController/InputComponent was null on first call)
	if (!ActiveInputBindings.IsEmpty() && (BridgedBindingHandles.IsEmpty() || !CurrentController || BridgedInputComponent.Get() != CurrentController->InputComponent))
	{
		SetupInputBridgeOnHUD();
	}

	// Callbacks can unregister widgets; iterate a snapshot instead of the live array.
	RegisteredWidgets.RemoveAll([](const auto& Entry) { return !IsValid(Entry.GetObject()); });
	const auto TickWidgets = RegisteredWidgets;
	for (const auto& Entry : TickWidgets)
	{
		if (IsValid(Entry.GetObject()) && RegisteredWidgets.Contains(Entry)) IGorgeousUIWidget_I::Execute_TickInterpolation(Entry.GetObject(), DeltaTime);
	}
	PendingFocusRequests.RemoveAll([](const auto& Request) { return Request.ExpirationTime < FPlatformTime::Seconds(); });
	if (bStateTransitionPending)
	{
		for (auto It = TransitioningWidgets.CreateIterator(); It; ++It)
		{
			UObject* Object = It->Get();
			UUserWidget* Widget = Cast<UUserWidget>(Object);
			IGorgeousUIWidget_I* Interface = IsValid(Object) ? Cast<IGorgeousUIWidget_I>(Object) : nullptr;
			if (!IsValid(Widget) || !Widget->IsVisible() || (!Widget->IsAnyAnimationPlaying() && (!Interface || (!Interface->IsInterpOpacity() && !Interface->IsInterpTheme())))) It.RemoveCurrent();
		}
		if (TransitioningWidgets.IsEmpty() || FPlatformTime::Seconds() >= TransitionDeadline) ExecuteStateSwap();
	}

}

TStatId UGorgeousUIFoundationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGorgeousUIFoundationSubsystem, STATGROUP_Tickables);
}

void UGorgeousUIFoundationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 0. Auto-Populate Default Processors (AAA Zero-Config)
	// We gather all processor classes and register them based on their CDO's TargetWidgetClass
	TArray<UClass*> ProcessorDerivedClasses;
	GetDerivedClasses(UGorgeousUIProcessor::StaticClass(), ProcessorDerivedClasses);

	for (UClass* Class : ProcessorDerivedClasses)
	{
		if (!Class->HasAnyClassFlags(CLASS_Abstract))
		{
			if (const UGorgeousUIProcessor* ProcessorCDO = Class->GetDefaultObject<UGorgeousUIProcessor>())
			{
				if (ProcessorCDO->TargetWidgetClass)
				{
					ProcessorClasses.FindOrAdd(ProcessorCDO->TargetWidgetClass) = Class;
				}
			}
		}
	}

	// 1. Listen for Input Method changes for Dynamic Icon Swapping
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UCommonInputSubsystem* InputSubsystem = LP->GetSubsystem<UCommonInputSubsystem>())
		{
			InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &UGorgeousUIFoundationSubsystem::OnInputMethodChanged);
		}
	}

	if (FSlateApplication::IsInitialized()) SlateFocusHandle = FSlateApplication::Get().OnFocusChanging().AddUObject(this, &UGorgeousUIFoundationSubsystem::HandleGlobalFocusChanging);
}


#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"

TArray<UGorgeousUITheme_DA*> UGorgeousUIFoundationSubsystem::GetCurrentThemes() const
{
	TArray<UGorgeousUITheme_DA*> Themes;
	UGorgeousUITheme_DA* Default = const_cast<UGorgeousUITheme_DA*>(UGorgeousUIProcessor::GetDefaultTheme());
	if (Default) Themes.Add(Default);
	const int32 EffectiveManualThemeDepth = FMath::Clamp(ManualThemeDepth, 0, CurrentStates.Num());
	if (EffectiveManualThemeDepth == 0) for (UGorgeousUITheme_DA* Theme : CurrentThemes) { if (IsValid(Theme)) Themes.AddUnique(Theme); }
	for (int32 Index = 0; Index < CurrentStates.Num(); ++Index)
	{
		UGorgeousUIState_DA* State = CurrentStates[Index];
		if (IsValid(State) && IsValid(State->Theme))
		{
			if (!State->bAdditiveToCurrentTheme) { Themes.Reset(); if (Default) Themes.Add(Default); }
			Themes.AddUnique(State->Theme);
		}
		if (Index + 1 == EffectiveManualThemeDepth) for (UGorgeousUITheme_DA* Theme : CurrentThemes) if (IsValid(Theme)) Themes.AddUnique(Theme);
	}
	for (const FGuid& Id : MessageOrder) { if (auto* Theme = MessageThemes.FindRef(Id).Get(); IsValid(Theme)) Themes.AddUnique(Theme); }
	return Themes;
}

UGorgeousUITheme_DA* UGorgeousUIFoundationSubsystem::GetMostRecentTheme() const
{	
	const TArray<UGorgeousUITheme_DA*> Themes = GetCurrentThemes();
	return Themes.IsEmpty() ? nullptr : Themes.Last();
}

void UGorgeousUIFoundationSubsystem::SetCurrentTheme(UGorgeousUITheme_DA* NewTheme)
{
	SCOPE_CYCLE_COUNTER(STAT_GUI_SetTheme);
	if (!IsValid(NewTheme)) return;
	if (const UGorgeousUITheme_DA* AffectedTheme = CurrentThemes.IsValidIndex(CurrentThemes.Num() - 1) ? CurrentThemes.Last() : nullptr; 
		AffectedTheme == NewTheme && ManualThemeDepth == CurrentStates.Num()) return;
	CurrentThemes.Reset();
	CurrentThemes.Add(NewTheme);
	ManualThemeDepth = CurrentStates.Num();
	IncrementThemeSwapsTriggered();

	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Applying Theme '%s'"), NewTheme ? *NewTheme->GetName() : TEXT("None"));

	BroadcastThemeApplied(CurrentThemes.Last());

	// Trigger HUD refresh so the Action Bar picks up new themed icons
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (APlayerController* PC = LP->GetPlayerController(GetWorld()))
		{
			if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
			{
				HUD->RefreshActionBar();
			}
		}
	}
}

static int32 GThemeSwapsTriggeredCount = 0;

int32 UGorgeousUIFoundationSubsystem::GetTotalThemeSwapsTriggered()
{
	return GThemeSwapsTriggeredCount;
}

void UGorgeousUIFoundationSubsystem::IncrementThemeSwapsTriggered()
{
	GThemeSwapsTriggeredCount++;
	INC_DWORD_STAT(STAT_GUI_ThemeSwaps);
}

void UGorgeousUIFoundationSubsystem::BroadcastThemeApplied(UGorgeousUITheme_DA* Theme)
{
	if (!Theme) return;
	const auto Widgets = RegisteredWidgets;
	for (const auto& Entry : Widgets)
	{
		if (IsValid(Entry.GetObject()) && Entry.GetInterface() && RegisteredWidgets.Contains(Entry)) Entry.GetInterface()->OnThemeApplied(Theme);
	}
}

void UGorgeousUIFoundationSubsystem::ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* Theme, const UGorgeousUITheme_DA* FallbackTheme)
{
	if (!Widget) return;

	if (UGorgeousUIProcessor* Processor = GetSharedProcessorForWidget(Widget))
	{
		Processor->ApplyThemeToWidget(Widget, Theme, FallbackTheme);
		return;
	}

	UGorgeousUIProcessor::ApplyThemeToWidgetInternal(Widget, Theme, FallbackTheme);
}

void UGorgeousUIFoundationSubsystem::BroadcastStateSwitch(UGorgeousUIState_DA* NewState)
{
	PushUIState(NewState);
}

void UGorgeousUIFoundationSubsystem::OnInputMethodChanged(ECommonInputType NewInputType)
{
	// When the input method changes, we re-broadcast the current theme.
	// Every widget will receive OnThemeApplied and re-fetch its Action Icons 
	// from the theme based on the new platform/input type.
	if (UGorgeousUITheme_DA* Theme = GetMostRecentTheme())
	{
		BroadcastThemeApplied(Theme);
	}
	RefreshLocalActionBar();
}

void UGorgeousUIFoundationSubsystem::PushInputBinding(UGorgeousInputBinding_DA* NewBindings)
{
	if (!NewBindings) return;

	if (const UGorgeousInputBinding_DA* AffectedBinding = ActiveInputBindings.IsValidIndex(ActiveInputBindings.Num() - 1) ? ActiveInputBindings.Last() : nullptr; 
		AffectedBinding == NewBindings) return;
	
	ActiveInputBindings.Remove(NewBindings);
	ActiveInputBindings.Add(NewBindings);

	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Pushed Input Bindings '%s'"), *NewBindings->GetName());

	// Add the IMCs associated with this binding
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FGorgeousInputMappingConfig_S& Config : NewBindings->InputMappingContexts)
			{
				if (UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
				{
					InputSubsystem->AddMappingContext(IMC, Config.Priority);
				}
			}
		}
	}

	SetupInputBridgeOnHUD();

	// Trigger HUD refresh
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (APlayerController* PC = LP->GetPlayerController(GetWorld()))
		{
			if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
			{
				HUD->RefreshActionBar();
			}
		}
	}
}

void UGorgeousUIFoundationSubsystem::PopInputBinding()
{
	if (ActiveInputBindings.IsEmpty())
	{
		return;
	}

	UGorgeousInputBinding_DA* PoppedBinding = ActiveInputBindings.Pop();
	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Popped Input Bindings '%s'"), PoppedBinding ? *PoppedBinding->GetName() : TEXT("None"));

	if (PoppedBinding)
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				for (const FGorgeousInputMappingConfig_S& Config : PoppedBinding->InputMappingContexts)
				{
					if (UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
					{
						bool bStillUsed = false;
					for (const UGorgeousInputBinding_DA* Remaining : ActiveInputBindings)
					{ if (Remaining) for (const auto& Other : Remaining->InputMappingContexts) if (Other.InputMapping.Get() == IMC) bStillUsed = true; }
					if (!bStillUsed) InputSubsystem->RemoveMappingContext(IMC);
					}
				}
			}
		}
	}

	SetupInputBridgeOnHUD();

	// Trigger HUD refresh
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (APlayerController* PC = LP->GetPlayerController(GetWorld()))
		{
			if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
			{
				HUD->RefreshActionBar();
			}
		}
	}
}

void UGorgeousUIFoundationSubsystem::RemoveInputBinding(UGorgeousInputBinding_DA* BindingToRemove)
{
	if (!BindingToRemove || !ActiveInputBindings.Contains(BindingToRemove))
	{
		return;
	}

	ActiveInputBindings.Remove(BindingToRemove);
	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Removed Input Bindings '%s'"), *BindingToRemove->GetName());

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FGorgeousInputMappingConfig_S& Config : BindingToRemove->InputMappingContexts)
			{
				if (UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
				{
					bool bStillUsed = false;
					for (const UGorgeousInputBinding_DA* Remaining : ActiveInputBindings)
					{ if (Remaining) for (const auto& Other : Remaining->InputMappingContexts) if (Other.InputMapping.Get() == IMC) bStillUsed = true; }
					if (!bStillUsed) InputSubsystem->RemoveMappingContext(IMC);
				}
			}
		}
	}

	SetupInputBridgeOnHUD();

	// Trigger HUD refresh
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (APlayerController* PC = LP->GetPlayerController(GetWorld()))
		{
			if (AGorgeousHUD* HUD = Cast<AGorgeousHUD>(PC->GetHUD()))
			{
				HUD->RefreshActionBar();
			}
		}
	}
}

void UGorgeousUIFoundationSubsystem::SetupInputBridgeOnHUD()
{
	if (UEnhancedInputComponent* Old = BridgedInputComponent.Get()) for (uint32 Handle : BridgedBindingHandles) Old->RemoveBindingByHandle(Handle);
	BridgedBindingHandles.Reset();
	BridgedInputComponent.Reset();
	if (ActiveInputBindings.IsEmpty()) return;
	ULocalPlayer* Player = GetLocalPlayer();
	APlayerController* Controller = Player ? Player->GetPlayerController(GetWorld()) : nullptr;
	UEnhancedInputComponent* Input = Controller ? Cast<UEnhancedInputComponent>(Controller->InputComponent) : nullptr;
	if (!Input) return; // Tick retries after controller/input creation.
	BridgedInputComponent = Input;
	TMap<UInputAction*, TSet<ETriggerEvent>> BoundEvents;
	for (UGorgeousInputBinding_DA* Asset : ActiveInputBindings)
	{
		if (!Asset) continue;
		for (const auto& Pair : Asset->Bindings)
		{
			UInputAction* Action = Pair.Value.Action;
			if (!Action) continue;
			TArray<ETriggerEvent> Events = Pair.Value.TriggerEventsToBind;
			if (Events.IsEmpty()) Events = {ETriggerEvent::Started, ETriggerEvent::Triggered, ETriggerEvent::Completed, ETriggerEvent::Canceled};
			for (ETriggerEvent Event : Events)
			{
				if (BoundEvents.FindOrAdd(Action).Contains(Event)) continue;
				BoundEvents.FindOrAdd(Action).Add(Event);
				BridgedBindingHandles.Add(Input->BindAction(Action, Event, this, &UGorgeousUIFoundationSubsystem::HandleBridgedInputAction).GetHandle());
			}
		}
	}
}
// ReSharper disable once CppMemberFunctionMayBeConst (If we would make it const, then the BindAction is not satisfied)
void UGorgeousUIFoundationSubsystem::HandleBridgedInputAction(const FInputActionInstance& Instance)
{
	if (ActiveInputBindings.IsEmpty()) return;

	ULocalPlayer* LP = GetLocalPlayer();
	APlayerController* PC = LP ? LP->GetPlayerController(GetWorld()) : nullptr;
	AGorgeousHUD* HUD = PC ? Cast<AGorgeousHUD>(PC->GetHUD()) : nullptr;

	if (!HUD) return;

	const UInputAction* Action = Instance.GetSourceAction();
	if (!Action) return;

	bool bInputWasConsumed = false;
	TSet<FGameplayTag> DispatchedTags;

	// Iterate in reverse priority order (state-specific bindings first, then default mappings)
	for (int32 i = ActiveInputBindings.Num() - 1; i >= 0; --i)
	{
		if (bInputWasConsumed) break;

		const UGorgeousInputBinding_DA* ActiveInputBinding = ActiveInputBindings[i];
		if (!ActiveInputBinding) continue;

		for (const auto& Pair : ActiveInputBinding->Bindings)
		{
			if (Pair.Value.Action == Action && (Pair.Value.TriggerEventsToBind.IsEmpty() || Pair.Value.TriggerEventsToBind.Contains(Instance.GetTriggerEvent())))
			{
				FGameplayTag DispatchedTag = Pair.Key;
				if (const FGameplayTag* RedirectedTag = ActiveInputRedirects.Find(DispatchedTag))
				{
					DispatchedTag = *RedirectedTag;
				}

				if (DispatchedTags.Contains(DispatchedTag)) continue;
				DispatchedTags.Add(DispatchedTag);
				// Per-player consumption applies within Gorgeous routing, without mutating the shared action asset.
				bool bHandled = HUD->DispatchGorgeousInput(DispatchedTag, Instance, Pair.Value.bConsumeInput);
				
				// If a consumer handled the action AND it's configured to consume, we kill further routing
				if (bHandled && Pair.Value.bConsumeInput)
				{
					bInputWasConsumed = true;
					break; 
				}
			}
		}
	}
}

FName UGorgeousUIFoundationSubsystem::GetCurrentPlatformName() const
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return TEXT("Generic");

	UCommonInputSubsystem* InputSubsystem = LP->GetSubsystem<UCommonInputSubsystem>();
	if (!InputSubsystem) return TEXT("Generic");

	// 1. Check for Mouse/Keyboard first
	if (InputSubsystem->GetCurrentInputType() == ECommonInputType::MouseAndKeyboard)
	{
		return TEXT("Keyboard");
	}

	// 2. It's a Gamepad - Let's get the ACTUAL hardware brand
	const FName GamepadName = InputSubsystem->GetCurrentGamepadName();
	
	// Map CommonUI Hardware Names to our Theme Keys
	FString GamepadStr = GamepadName.ToString().ToLower();
	
	if (GamepadStr.Contains(TEXT("xbox"))) return TEXT("Xbox");
	if (GamepadStr.Contains(TEXT("playstation")) || GamepadStr.Contains(TEXT("ps4")) || GamepadStr.Contains(TEXT("ps5"))) return TEXT("PlayStation");
	if (GamepadStr.Contains(TEXT("nintendo")) || GamepadStr.Contains(TEXT("switch"))) return TEXT("Switch");

	// 3. Fallback to Platform Macros if hardware ID is generic/unknown
#if WITH_EDITOR || PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC
	return FName(TEXT("Xbox")); // PC builds treat generic gamepads as XInput/Xbox layout
#else
	// Fallback using Unreal's runtime platform string name for consoles/mobile
	FString PlatformName = FPlatformProperties::PlatformName();
	if (PlatformName.Contains(TEXT("Xbox"))) return FName(TEXT("Xbox"));
	if (PlatformName.Contains(TEXT("PS")) || PlatformName.Contains(TEXT("PlayStation"))) return FName(TEXT("PlayStation"));
	if (PlatformName.Contains(TEXT("Switch"))) return FName(TEXT("Switch"));
    
	return FName(TEXT("Generic"));
#endif
}

void UGorgeousUIFoundationSubsystem::OnMessageRequestReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const auto* Request = Payload.GetPtr<FGorgeousUIMessageRequest>();
	if (!Request || !Request->Config || (Request->LocalPlayerIndex == INDEX_NONE ? 0 : Request->LocalPlayerIndex) != GetLocalPlayerIndex() || ActiveMessageRequests.Contains(Request->RequestID)) return;
	ActiveMessageRequests.Add(Request->RequestID);
	MessageOrder.Add(Request->RequestID);
	if (Request->Config->State)
	{
		MessageStates.Add(Request->RequestID, Request->Config->State);
		if (!CurrentStates.Contains(Request->Config->State)) { MessageOwnedStates.Add(Request->Config->State); PushUIState(Request->Config->State, true); }
	}
	if (Request->Config->MessageTheme) MessageThemes.Add(Request->RequestID, Request->Config->MessageTheme);
	BroadcastThemeApplied(GetMostRecentTheme());
	USignalBridgeBlueprintFunctionLibrary::DispatchLocal(this, TAG_Gorgeous_UI_Layout_PushWidget, Payload);
}

void UGorgeousUIFoundationSubsystem::CompleteMessagePresentation(FGuid RequestID)
{
	if (!ActiveMessageRequests.Remove(RequestID)) return;
	UGorgeousUIState_DA* State = MessageStates.FindRef(RequestID);
	MessageStates.Remove(RequestID); MessageThemes.Remove(RequestID); MessageOrder.Remove(RequestID);
	if (State && MessageOwnedStates.Contains(State))
	{
		bool bStillUsed = false;
		for (const auto& Entry : MessageStates) if (Entry.Value == State) bStillUsed = true;
		if (!bStillUsed) { MessageOwnedStates.Remove(State); RemoveUIStateByObject(State, true); }
	}
	BroadcastThemeApplied(GetMostRecentTheme());
	RefreshLocalActionBar();
}

void UGorgeousUIFoundationSubsystem::OnMessageResultReceived(FGameplayTag, const FInstancedStruct& Payload)
{
	if (const auto* Result = Payload.GetPtr<FGorgeousUIMessageResult>()) CompleteMessagePresentation(Result->RequestID);
}

#include "CommonButtonBase.h"

void UGorgeousUIFoundationSubsystem::OnInputActionReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const FGorgeousInputActionPayload* ActionPayload = Payload.GetPtr<FGorgeousInputActionPayload>();
	if (!ActionPayload || !ActionPayload->ActionTag.IsValid() || (ActionPayload->LocalPlayerIndex == INDEX_NONE ? 0 : ActionPayload->LocalPlayerIndex) != GetLocalPlayerIndex()) return;

	UUserWidget* TargetWidget = nullptr;

	// 1. Find the target widget (either specified or top-most active)
	if (ActionPayload->TargetWidgetTag.IsValid())
	{
		for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
		{
			if (IGorgeousUIWidget_I* WidgetInterface = It->GetInterface())
			{
				if (WidgetInterface->GetBindingTag() == ActionPayload->TargetWidgetTag)
				{
					TargetWidget = Cast<UUserWidget>(It->GetObject());
					break;
				}
			}
		}
	}
	else
	{
		// Global Routing: Find the top-most active activatable widget
		for (int32 i = RegisteredWidgets.Num() - 1; i >= 0; --i)
		{
			UUserWidget* WidgetObj = Cast<UUserWidget>(RegisteredWidgets[i].GetObject());
			if (UCommonActivatableWidget* Activatable = Cast<UCommonActivatableWidget>(WidgetObj))
			{
				if (Activatable->IsActivated())
				{
					TargetWidget = Activatable;
					break;
				}
			}
		}
	}

	if (!TargetWidget) return;

	// 2. Route the action
	
	// Case A: It's a Button -> Remote Click
	if (UCommonButtonBase* Button = Cast<UCommonButtonBase>(TargetWidget))
	{
		UFunction* ClickFunc = nullptr;
		ClickFunc = Button->FindFunction(FName("NativeClick"));
		if (!ClickFunc) ClickFunc = Button->FindFunction(FName("Click"));
		if (!ClickFunc) ClickFunc = Button->FindFunction(FName("OnClicked"));
		if (ClickFunc)
		{
			Button->ProcessEvent(ClickFunc, nullptr);
			return;
		}
	}

	// Case B: It's an Activatable Widget -> Handle Standard Actions
	if (UCommonActivatableWidget* Activatable = Cast<UCommonActivatableWidget>(TargetWidget))
	{
		static FGameplayTag BackTag = FGlobalUITags::Get().UIAction_Cancel; // Use Unreal's Default UI Tags
		if (ActionPayload->ActionTag == BackTag)
		{
			if (UFunction* HandleFunc = Activatable->FindFunction(FName("HandleBackAction")))
			{
				Activatable->ProcessEvent(HandleFunc, nullptr);
				return;
			}
		}
	}

	// Case C: Universal Reflection Magic -> Look for "On[ActionName]" or "Handle[ActionName]"
	FString ActionName = ActionPayload->ActionTag.GetTagName().ToString();
	ActionName.RemoveFromStart("UI.Action."); // Cleanup the tag name
	
	// We'll use the reflection helper from the Processor class
	// Note: We'll assume a dummy payload for the function call if needed, 
	// or just call it as a parameterless function.
	FString ShortName;
	if (ActionName.Split(TEXT("."), nullptr, &ShortName, ESearchCase::CaseSensitive, ESearchDir::FromEnd)) ActionName = ShortName;
	for (const FString& Prefix : { FString(TEXT("Handle")), FString(TEXT("On")) })
	{
		if (UFunction* Function = TargetWidget->FindFunction(FName(*(Prefix + ActionName))); Function && Function->ParmsSize == 0 && !Function->HasAnyFunctionFlags(FUNC_Net | FUNC_Static))
		{ TargetWidget->ProcessEvent(Function, nullptr); break; }
	}
}

void UGorgeousUIFoundationSubsystem::OnFocusRequestReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const auto* Request = Payload.GetPtr<FGorgeousFocusRequestPayload>();
	if (!Request || !Request->TargetTag.IsValid() || (Request->LocalPlayerIndex == INDEX_NONE ? 0 : Request->LocalPlayerIndex) != GetLocalPlayerIndex()) return;
	if (TryFocusWidget(Request->TargetTag, Request->RoutingID)) return;
	FGorgeousPendingFocusRequest Pending;
	Pending.TargetTag = Request->TargetTag; Pending.RoutingID = Request->RoutingID; Pending.ExpirationTime = FPlatformTime::Seconds() + 1.;
	PendingFocusRequests.RemoveAll([&](const auto& Entry) { return Entry.TargetTag == Pending.TargetTag && Entry.RoutingID == Pending.RoutingID; });
	PendingFocusRequests.Add(Pending);
}

void UGorgeousUIFoundationSubsystem::HandleGlobalFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget)
{
	if (!NewFocusedWidget.IsValid()) return;
	if (GetLocalPlayer() && GetLocalPlayer()->GetSlateUser().IsValid() && FocusEvent.GetUser() != GetLocalPlayer()->GetSlateUser()->GetUserIndex()) return;

	// Check if this was a navigation event for sound ticks
	if (FocusEvent.GetCause() == EFocusCause::Navigation)
	{
		if (GetWorld())
		{
			USignalBridgeBlueprintFunctionLibrary::DispatchLocal(GetWorld(), TAG_Gorgeous_UI_Focus_Navigated, FInstancedStruct());
		}
	}

	// Find if the new focused SWidget belongs to one of our registered IGorgeousUIWidget_I objects
	for (auto& Entry : RegisteredWidgets)
	{
		UWidget* UserWidget = Cast<UWidget>(Entry.GetObject());
		if (UserWidget && UserWidget->GetCachedWidget().IsValid() && (UserWidget->GetCachedWidget() == NewFocusedWidget || NewFocusedWidgetPath.ContainsWidget(UserWidget->GetCachedWidget().Get())))
		{
			// Enforce ScrollBox visibility automatically
			UWidget* CurrentParent = UserWidget->GetParent();
			while (CurrentParent)
			{
				if (UScrollBox* ScrollBox = Cast<UScrollBox>(CurrentParent))
				{
					ScrollBox->ScrollWidgetIntoView(UserWidget, false, EDescendantScrollDestination::IntoView, 0.0f);
					break;
				}
				CurrentParent = CurrentParent->GetParent();
			}

			if (IGorgeousUIWidget_I* WidgetInterface = Entry.GetInterface())
			{
				const FGameplayTag Tag = WidgetInterface->GetBindingTag();
				if (Tag.IsValid() && GetWorld())
				{
					FGorgeousFocusChangedPayload_S PayloadOut;
					PayloadOut.FocusedTag = Tag;
					PayloadOut.RoutingID = WidgetInterface->GetRoutingID();
					
					if (!StateFocusHistory.IsEmpty()) StateFocusHistory.Last() = Tag;
					USignalBridgeBlueprintFunctionLibrary::DispatchLocal(GetWorld(), TAG_Gorgeous_UI_Focus_Changed, FInstancedStruct::Make(PayloadOut));
				}
			}
			break;
		}
	}
}

void UGorgeousUIFoundationSubsystem::HandlePayloadForTag(FGameplayTag Tag, const FInstancedStruct& Payload)
{
	const FGorgeousUIUpdatePayload* UpdatePayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	const FName TargetRoutingID = UpdatePayload ? UpdatePayload->RoutingID : NAME_None;

	// Iterate registered widgets and dispatch payloads to shared processors for widgets
	// that have the matching binding tag and routing ID (if specified).
	for (auto& Entry : RegisteredWidgets)
	{
		if (IGorgeousUIWidget_I* WidgetInterface = Entry.GetInterface())
		{
			if (WidgetInterface->GetBindingTag() == Tag)
			{
				// If a RoutingID is specified in the payload, only dispatch to the widget with a matching ID.
				if (TargetRoutingID.IsNone() || WidgetInterface->GetRoutingID() == TargetRoutingID)
				{
					UObject* Obj = Entry.GetObject();
					if (UpdatePayload && UpdatePayload->InventoryID.IsValid())
					{
						const FStructProperty* IdProperty = FindFProperty<FStructProperty>(Obj->GetClass(), TEXT("InventoryID"));
						if (!IdProperty || IdProperty->Struct != TBaseStructure<FGuid>::Get() || *IdProperty->ContainerPtrToValuePtr<FGuid>(Obj) != UpdatePayload->InventoryID) continue;
					}
					if (UpdatePayload && UpdatePayload->AnimationTag.IsValid()) PlayNamedWidgetAnimation(Obj, UpdatePayload->AnimationTag.GetTagName());
					if (UGorgeousUIProcessor* SharedProcessor = GetSharedProcessorForWidget(Obj))
					{
						SharedProcessor->OnSignalReceived(Obj, Tag, Payload);
					}
				}
			}
		}
	}
}

void UGorgeousUIFoundationSubsystem::PushUIState(UGorgeousUIState_DA* NewState, bool bImmediate)
{
	if (!IsValid(NewState) || GetMostRecentUIState() == NewState) return;
	BeginStateTransition(NewState, true, bImmediate);
}

void UGorgeousUIFoundationSubsystem::RemoveUIState(TSubclassOf<UGorgeousUIState_DA> StateClass, bool bImmediate)
{
	if (!StateClass) return;
	for (int32 Index = CurrentStates.Num() - 1; Index >= 0; --Index)
	{ if (CurrentStates[Index] && CurrentStates[Index]->IsA(StateClass)) { RemoveUIStateByObject(CurrentStates[Index], bImmediate); return; } }
}

void UGorgeousUIFoundationSubsystem::RemoveUIStateByObject(UGorgeousUIState_DA* State, bool bImmediate)
{
	if (bStateTransitionPending && bPendingStateIsPush && PendingState == State)
	{ bStateTransitionPending = false; PendingState = nullptr; TransitioningWidgets.Reset(); }
	const int32 Index = CurrentStates.FindLast(State);
	if (Index == INDEX_NONE) return;
	const bool bWasTop = Index == CurrentStates.Num() - 1;
	CurrentStates.RemoveAt(Index);
	if (Index < ManualThemeDepth) --ManualThemeDepth;
	if (StateFocusHistory.IsValidIndex(Index)) StateFocusHistory.RemoveAt(Index);
	if (bWasTop) BeginStateTransition(GetMostRecentUIState(), false, bImmediate);
	else { ReconcileStateInputBindings(); BroadcastThemeApplied(GetMostRecentTheme()); }
}

void UGorgeousUIFoundationSubsystem::BeginStateTransition(UGorgeousUIState_DA* State, bool bPush, bool bImmediate)
{
	PendingState = State;
	bStateTransitionPending = true;
	bPendingStateIsPush = bPush;
	TransitionDeadline = FPlatformTime::Seconds() + 10.;
	TransitioningWidgets.Reset();
	if (!bImmediate)
	{
		for (const auto& Entry : RegisteredWidgets)
		{ if (const UUserWidget* Widget = Cast<UUserWidget>(Entry.GetObject()); IsValid(Widget) && Widget->IsVisible()) TransitioningWidgets.Add(Entry.GetObject()); }
	}
	if (TransitioningWidgets.IsEmpty()) ExecuteStateSwap();
	else OnTransitionStarted.Broadcast(State);
}

UGorgeousUIState_DA* UGorgeousUIFoundationSubsystem::GetUIStateByObject(
	UGorgeousUIState_DA* InState) const
{
	if (!GetCurrentUIStates().Contains(InState)) return nullptr;
	
	return GetCurrentUIStates()[GetCurrentUIStates().Find(InState)];
}

UGorgeousUITheme_DA* UGorgeousUIFoundationSubsystem::GetThemeByObject(UGorgeousUITheme_DA* InTheme) const
{
	if (!GetCurrentThemes().Contains(InTheme)) return nullptr;
	
	return GetCurrentThemes()[GetCurrentThemes().Find(InTheme)];
}

void UGorgeousUIFoundationSubsystem::NotifyWidgetTransitionComplete(UObject* Widget)
{
	if (!bStateTransitionPending || !Widget) return;
	TransitioningWidgets.Remove(Widget);
	if (TransitioningWidgets.IsEmpty()) ExecuteStateSwap();
}

void UGorgeousUIFoundationSubsystem::ExecuteStateSwap()
{
	if (!bStateTransitionPending) return;
	UGorgeousUIState_DA* State = PendingState;
	PendingState = nullptr;
	bStateTransitionPending = false;
	TransitioningWidgets.Reset();
	if (bPendingStateIsPush && IsValid(State))
	{
		if (StateFocusHistory.IsValidIndex(CurrentStates.Num() - 1))
		{
			for (const auto& Entry : RegisteredWidgets)
			{ if (const UWidget* Widget = Cast<UWidget>(Entry.GetObject()); Widget && Widget->HasAnyUserFocus() && Entry.GetInterface()) { StateFocusHistory.Last() = Entry.GetInterface()->GetBindingTag(); break; } }
		}
		CurrentStates.Add(State);
		StateFocusHistory.Add(FGameplayTag());
	}
	ReconcileStateInputBindings();
	BroadcastThemeApplied(GetMostRecentTheme());
	IncrementThemeSwapsTriggered();
	const auto Widgets = RegisteredWidgets;
	for (const auto& Entry : Widgets)
	{
		UObject* Object = Entry.GetObject();
		if (!IsValid(Object) || !Entry.GetInterface()) continue;
		if (auto* Common = Cast<UGorgeousCommonWidget>(Object)) Common->OnUIStateChanged(State);
		else if (auto* Activatable = Cast<UGorgeousActivatableWidget>(Object)) Activatable->OnUIStateChanged(State);
		if (State && State->OverlayConfig)
		{ if (const auto* Config = State->OverlayConfig->WidgetConfigs.Find(Entry.GetInterface()->GetBindingTag()); Config && (!Config->State || Config->State == State)) Entry.GetInterface()->ApplyOverlayConfig(*Config); }
		if (State) PlayNamedWidgetAnimation(Object, State->TransitionInAnimation);
	}
	ApplyFocusRoutes();
	FGameplayTag FocusTag = StateFocusHistory.IsEmpty() ? FGameplayTag() : StateFocusHistory.Last();
	if (!FocusTag.IsValid() && State && State->FocusFallback.Strategy == EGorgeousFocusFallbackStrategy_E::FocusSpecificWidget) FocusTag = State->FocusFallback.TargetTag;
	if (FocusTag.IsValid())
	{ FGorgeousFocusRequestPayload Request; Request.TargetTag = FocusTag; Request.LocalPlayerIndex = GetLocalPlayerIndex(); OnFocusRequestReceived(TAG_Gorgeous_UI_Focus_Request, FInstancedStruct::Make(Request)); }
	RefreshLocalActionBar();
	OnTransitionFinished.Broadcast(State);
}

void UGorgeousUIFoundationSubsystem::ReconcileStateInputBindings()
{
	TArray<UGorgeousInputBinding_DA*> Desired;
	int32 Start = 0;
	for (int32 Index = CurrentStates.Num() - 1; Index >= 0; --Index) { if (CurrentStates[Index] && !CurrentStates[Index]->bAdditiveToCurrentState) { Start = Index; break; } }
	for (int32 Index = Start; Index < CurrentStates.Num(); ++Index) { if (CurrentStates[Index] && CurrentStates[Index]->InputBindings) Desired.AddUnique(CurrentStates[Index]->InputBindings); }
	const auto Old = StateOwnedInputBindings;
	for (UGorgeousInputBinding_DA* Binding : Old) { if (!Desired.Contains(Binding)) { StateOwnedInputBindings.Remove(Binding); RemoveInputBinding(Binding); } }
	for (UGorgeousInputBinding_DA* Binding : Desired) { if (!ActiveInputBindings.Contains(Binding)) { PushInputBinding(Binding); StateOwnedInputBindings.AddUnique(Binding); } }
}

void UGorgeousUIFoundationSubsystem::RegisterWidget(IGorgeousUIWidget_I* Widget)
{
	if (!Widget) return;

	UObject* WidgetObj = Widget->GetAsWidget();
	RegisteredWidgets.AddUnique(TScriptInterface<IGorgeousUIWidget_I>(WidgetObj));
	ApplyFocusRoutes();

	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Registered Widget '%s' (Binding Tag: %s)"), *WidgetObj->GetName(), *Widget->GetBindingTag().ToString());

	// Newly constructed widgets receive the effective state/message theme stack.
	for (UGorgeousUITheme_DA* Theme : GetCurrentThemes()) Widget->OnThemeApplied(Theme);

	// 2. Setup Shared Processor for Signal Handling
	const FGameplayTag BindingTag = Widget->GetBindingTag();
	if (BindingTag.IsValid())
	{
		// Ensure a single SignalBridge listener exists per tag. A forwarder object will
		// receive the raw payload and ask the subsystem to dispatch to the appropriate
		// shared processors for registered widgets that match the tag.
		FGorgeousSignalBridgeAccessRules_S Rules;
		Rules.bNetworked = false;
		AGorgeousPlayerController* Controller = nullptr;
		if (UWorld* __World = GetWorld())
		{
			if (APlayerController* PC = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(__World) : nullptr)
			{
				Controller = Cast<AGorgeousPlayerController>(PC);
			}
		}

		if (!TagForwarders.Contains(BindingTag))
		{
			UGorgeousSignalBridgeTagForwarder* Forwarder = NewObject<UGorgeousSignalBridgeTagForwarder>(this);
			Forwarder->Tag = BindingTag;
			Forwarder->OwningSubsystem = this;
			TagForwarders.Add(BindingTag, Forwarder);

			FSignalBridgeEventDelegate ListenDelegate;
			ListenDelegate.BindDynamic(Forwarder, &UGorgeousSignalBridgeTagForwarder::OnBridgePayload);

			USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), BindingTag, Rules, nullptr);
			USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), BindingTag, nullptr, ListenDelegate);
		}
	}

	// Check deferred focus requests
	if (IGorgeousUIWidget_I* WidgetInterface = Widget)
	{
		const FGameplayTag Tag = WidgetInterface->GetBindingTag();
		if (Tag.IsValid())
		{
			for (int32 i = PendingFocusRequests.Num() - 1; i >= 0; --i)
			{
				if (PendingFocusRequests[i].ExpirationTime < FPlatformTime::Seconds()) { PendingFocusRequests.RemoveAt(i); continue; }
				if (PendingFocusRequests[i].TargetTag == Tag)
				{
					if (PendingFocusRequests[i].RoutingID.IsNone() || PendingFocusRequests[i].RoutingID == WidgetInterface->GetRoutingID())
					{
						TryFocusWidget(Tag, PendingFocusRequests[i].RoutingID);
						PendingFocusRequests.RemoveAt(i);
						break;
					}
				}
			}
		}
	}
}

void UGorgeousUIFoundationSubsystem::UnregisterWidget(IGorgeousUIWidget_I* Widget)
{
	if (!Widget) return;

	UObject* WidgetObj = Widget->GetAsWidget();
	RegisteredWidgets.Remove(TScriptInterface<IGorgeousUIWidget_I>(WidgetObj));
	ApplyFocusRoutes();
	TransitioningWidgets.Remove(WidgetObj);
	if (bStateTransitionPending && TransitioningWidgets.IsEmpty()) ExecuteStateSwap();

	// Cleanup signal listeners for this specific widget
	if (Widget->GetBindingTag().IsValid())
	{
		AGorgeousPlayerController* Controller = nullptr;
		if (UWorld* __World = GetWorld())
		{
			if (APlayerController* PC = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(__World) : nullptr)
			{
				Controller = Cast<AGorgeousPlayerController>(PC);
			}
		}

		const FGameplayTag Tag = Widget->GetBindingTag();
		bool bAnyRemaining = false;
		for (auto& Entry : RegisteredWidgets)
		{
			if (IGorgeousUIWidget_I* Other = Entry.GetInterface())
			{
				if (Other->GetBindingTag() == Tag)
				{
					bAnyRemaining = true;
					break;
				}
			}
		}

		if (!bAnyRemaining)
		{
			if (auto* Forwarder = TagForwarders.FindRef(Tag).Get())
			{ FSignalBridgeEventDelegate Delegate; Delegate.BindDynamic(Forwarder, &UGorgeousSignalBridgeTagForwarder::OnBridgePayload); USignalBridgeBlueprintFunctionLibrary::Unlisten(GetWorld(), Tag, nullptr, Delegate); }
			TagForwarders.Remove(Tag);
		}
	}
}

UGorgeousUIProcessor* UGorgeousUIFoundationSubsystem::GetSharedProcessorForWidget(UObject* Widget)
{
	if (!Widget) return nullptr;

	// 1. Determine the processor class for this object's class hierarchy
	UClass* CurrentClass = Widget->GetClass();
	TSubclassOf<UGorgeousUIProcessor> TargetProcessorClass = UGorgeousUIProcessor::StaticClass();

	while (CurrentClass)
	{
		if (TSubclassOf<UGorgeousUIProcessor>* FoundClass = ProcessorClasses.Find(CurrentClass))
		{
			TargetProcessorClass = *FoundClass;
			break;
		}
		CurrentClass = CurrentClass->GetSuperClass();
	}
	
	// 1.5: Check and early return if the processor class is abstract
	if (!TargetProcessorClass || TargetProcessorClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	// 2. Find or create the shared instance
	TObjectPtr<UGorgeousUIProcessor>& SharedInstance = SharedProcessors.FindOrAdd(TargetProcessorClass);
	if (!SharedInstance)
	{
		SharedInstance = NewObject<UGorgeousUIProcessor>(this, TargetProcessorClass);
	}

	return SharedInstance;
}

void UGorgeousUIFoundationSubsystem::SetInputRedirect(FGameplayTag HardwareTag, FGameplayTag SimulatedTag)
{
	if (!HardwareTag.IsValid() || !SimulatedTag.IsValid()) return;
	
	ActiveInputRedirects.Add(HardwareTag, SimulatedTag);
}

void UGorgeousUIFoundationSubsystem::ClearInputRedirect(FGameplayTag HardwareTag)
{
	if (!HardwareTag.IsValid()) return;
	
	ActiveInputRedirects.Remove(HardwareTag);
}

UGorgeousUIFoundationSubsystem* UGorgeousUIFoundationSubsystem::GetForContext(const UObject* Context)
{
	if (!IsValid(Context)) return nullptr;
	if (const auto* Subsystem = Cast<UGorgeousUIFoundationSubsystem>(Context)) return const_cast<UGorgeousUIFoundationSubsystem*>(Subsystem);
	const UUserWidget* UserWidget = Cast<UUserWidget>(Context);
	if (!UserWidget) UserWidget = Context->GetTypedOuter<UUserWidget>();
	ULocalPlayer* Player = UserWidget ? UserWidget->GetOwningLocalPlayer() : const_cast<ULocalPlayer*>(Cast<ULocalPlayer>(Context));
	if (!Player) Player = Context->GetTypedOuter<ULocalPlayer>();
	if (!Player) { if (const UWidget* Widget = Cast<UWidget>(Context)) if (auto* Controller = Widget->GetOwningPlayer()) Player = Controller->GetLocalPlayer(); }
	if (!Player) { if (const APlayerController* Controller = Cast<APlayerController>(Context)) Player = Controller->GetLocalPlayer(); }
	if (!Player) { if (const AHUD* HUD = Cast<AHUD>(Context)) { if (auto* Controller = HUD->GetOwningPlayerController()) Player = Controller->GetLocalPlayer(); } }
	if (!Player && !UserWidget) { if (UWorld* World = Context->GetWorld()) Player = World->GetFirstLocalPlayerFromController(); }
	return Player ? Player->GetSubsystem<UGorgeousUIFoundationSubsystem>() : nullptr;
}

int32 UGorgeousUIFoundationSubsystem::GetLocalPlayerIndex() const
{
	const ULocalPlayer* Player = GetLocalPlayer();
	return Player && Player->GetGameInstance() ? Player->GetGameInstance()->GetLocalPlayers().IndexOfByKey(Player) : (Player ? 0 : INDEX_NONE);
}

bool UGorgeousUIFoundationSubsystem::PlayNamedWidgetAnimation(UObject* Object, FName Name)
{
	UUserWidget* Widget = Cast<UUserWidget>(Object);
	if (!Widget || Name.IsNone()) return false;
	const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(Widget->GetClass(), Name);
	UWidgetAnimation* Animation = Property ? Cast<UWidgetAnimation>(Property->GetObjectPropertyValue_InContainer(Widget)) : nullptr;
	if (!Animation) Animation = FindObject<UWidgetAnimation>(Widget->GetClass(), *Name.ToString());
	if (!Animation)
	{
		FString ShortName;
		if (Name.ToString().Split(TEXT("."), nullptr, &ShortName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
		{
			const FObjectPropertyBase* ShortProperty = FindFProperty<FObjectPropertyBase>(Widget->GetClass(), FName(*ShortName));
			Animation = ShortProperty ? Cast<UWidgetAnimation>(ShortProperty->GetObjectPropertyValue_InContainer(Widget)) : nullptr;
			if (!Animation) Animation = FindObject<UWidgetAnimation>(Widget->GetClass(), *ShortName);
		}
	}
	if (!Animation) return false;
	Widget->PlayAnimation(Animation);
	return true;
}

bool UGorgeousUIFoundationSubsystem::TryFocusWidget(FGameplayTag Tag, FName Routing)
{
	for (const auto& Entry : RegisteredWidgets)
	{
		UWidget* Widget = Cast<UWidget>(Entry.GetObject());
		if (!IsValid(Widget) || !Entry.GetInterface() || Entry.GetInterface()->GetBindingTag() != Tag || (!Routing.IsNone() && Entry.GetInterface()->GetRoutingID() != Routing)) continue;
		if (auto* Activatable = Cast<UCommonActivatableWidget>(Widget)) Activatable->ActivateWidget();
		if (APlayerController* Controller = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr) Widget->SetUserFocus(Controller);
		else if (FSlateApplication::IsInitialized() && Widget->GetCachedWidget().IsValid()) Widget->SetFocus();
		return true;
	}
	return false;
}

void UGorgeousUIFoundationSubsystem::RefreshLocalActionBar()
{
	if (APlayerController* Controller = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr)
	{ if (auto* HUD = Cast<AGorgeousHUD>(Controller->GetHUD())) HUD->RefreshActionBar(); }
}

namespace
{
	FWidgetNavigationData* GorgeousNavigationData(UWidgetNavigation* Navigation, EUINavigation Direction)
	{
		if (!Navigation) return nullptr;
		switch (Direction)
		{
		case EUINavigation::Up: return &Navigation->Up;
		case EUINavigation::Down: return &Navigation->Down;
		case EUINavigation::Left: return &Navigation->Left;
		case EUINavigation::Right: return &Navigation->Right;
		case EUINavigation::Next: return &Navigation->Next;
		case EUINavigation::Previous: return &Navigation->Previous;
		default: return nullptr;
		}
	}
}

void UGorgeousUIFoundationSubsystem::ApplyFocusRoutes()
{
	// Reverse order also restores duplicate route overrides to their original rule.
	for (int32 Index = AppliedFocusRoutes.Num() - 1; Index >= 0; --Index)
	{
		const auto& Backup = AppliedFocusRoutes[Index];
		if (auto* Widget = Backup.Widget.Get()) if (auto* Data = GorgeousNavigationData(Widget->Navigation, Backup.Direction))
		{ *Data = Backup.Previous; Widget->BuildNavigation(); }
	}
	AppliedFocusRoutes.Reset();
	const UGorgeousUIState_DA* State = GetMostRecentUIState();
	if (!State) return;
	for (const auto& Route : State->FocusRoutes)
	{
		UWidget* Source = nullptr; UWidget* Target = nullptr;
		for (const auto& Entry : RegisteredWidgets) if (Entry.GetInterface())
		{ if (Entry.GetInterface()->GetBindingTag() == Route.SourceTag) Source = Cast<UWidget>(Entry.GetObject()); if (Entry.GetInterface()->GetBindingTag() == Route.TargetTag) Target = Cast<UWidget>(Entry.GetObject()); }
		if (Source && Target && Route.Direction != EUINavigation::Invalid)
		{
			const auto* Previous = GorgeousNavigationData(Source->Navigation, Route.Direction);
			AppliedFocusRoutes.Add({Source, Route.Direction, Previous ? *Previous : FWidgetNavigationData()});
			Source->SetNavigationRuleExplicit(Route.Direction, Target);
		}
	}
}

void UGorgeousUIFoundationSubsystem::DetachSignalListeners()
{
	if (UWorld* World = SignalWorld.Get())
	{
		USignalBridgeBlueprintFunctionLibrary::Unlisten(World, TAG_Gorgeous_UI_Focus_Request, nullptr, FocusRequestDelegate);
		USignalBridgeBlueprintFunctionLibrary::Unlisten(World, TAG_Gorgeous_UI_Input_Action, nullptr, InputActionDelegate);
		USignalBridgeBlueprintFunctionLibrary::Unlisten(World, TAG_Gorgeous_UI_System_Message_Push, nullptr, MessageRequestDelegate);
		USignalBridgeBlueprintFunctionLibrary::Unlisten(World, TAG_Gorgeous_UI_System_Message_Result, nullptr, MessageResultDelegate);
		for (const auto& Entry : TagForwarders) if (Entry.Value)
		{ FSignalBridgeEventDelegate Delegate; Delegate.BindDynamic(Entry.Value.Get(), &UGorgeousSignalBridgeTagForwarder::OnBridgePayload); USignalBridgeBlueprintFunctionLibrary::Unlisten(World, Entry.Key, nullptr, Delegate); }
	}
	bSignalsRegistered = false; SignalController.Reset(); SignalWorld.Reset();
}

void UGorgeousUIFoundationSubsystem::Deinitialize()
{
	if (FSlateApplication::IsInitialized()) FSlateApplication::Get().OnFocusChanging().Remove(SlateFocusHandle);
	if (GetLocalPlayer()) if (auto* Input = GetLocalPlayer()->GetSubsystem<UCommonInputSubsystem>()) Input->OnInputMethodChangedNative.RemoveAll(this);
	DetachSignalListeners();
	CurrentStates.Reset(); ApplyFocusRoutes();
	if (auto* Input = BridgedInputComponent.Get()) for (uint32 Handle : BridgedBindingHandles) Input->RemoveBindingByHandle(Handle);
	BridgedBindingHandles.Reset(); TagForwarders.Reset(); RegisteredWidgets.Reset();
	Super::Deinitialize();
}
