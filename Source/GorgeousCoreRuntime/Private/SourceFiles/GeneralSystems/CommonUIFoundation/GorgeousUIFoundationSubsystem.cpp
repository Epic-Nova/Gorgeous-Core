#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
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
#include "Stats/Stats.h"

// Note: STATGROUP_GorgeousUIFoundation is declared in GorgeousPrimaryGameLayout.cpp, but for linking ease across translation units, we can just declare the counter here.
extern TStatId GetStatId_STATGROUP_GorgeousUIFoundation();
DECLARE_STATS_GROUP(TEXT("Gorgeous UI Foundation"), STATGROUP_GorgeousUIFoundation, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Set UI Theme"), STAT_GUI_SetTheme, STATGROUP_GorgeousUIFoundation);
DECLARE_DWORD_COUNTER_STAT(TEXT("Total Theme Swaps"), STAT_GUI_ThemeSwaps, STATGROUP_GorgeousUIFoundation);

void UGorgeousUIFoundationSubsystem::Tick(float DeltaTime)
{
	// Lazy signal registration for system-wide Signal Bridge routing
	if (!bSignalsRegistered)
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (AGorgeousPlayerController* GPC = Cast<AGorgeousPlayerController>(LP->GetPlayerController(GetWorld())))
			{
				FGorgeousSignalBridgeAccessRules_S Rules;

				// Focus Routing
				const FGameplayTag FocusRequestTag = TAG_Gorgeous_UI_Focus_Request;
				FocusRequestDelegate.BindDynamic(this, &UGorgeousUIFoundationSubsystem::OnFocusRequestReceived);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), FocusRequestTag, Rules, GPC);
				USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), FocusRequestTag, GPC, FocusRequestDelegate);

				// Input Action Routing
				const FGameplayTag InputActionTag = TAG_Gorgeous_UI_Input_Action;
				InputActionDelegate.BindDynamic(this, &UGorgeousUIFoundationSubsystem::OnInputActionReceived);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), InputActionTag, Rules, GPC);
				USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), InputActionTag, GPC, InputActionDelegate);

				// Message Routing
				const FGameplayTag MessagePushTag = TAG_Gorgeous_UI_System_Message_Push;
				const FGameplayTag MessageResultTag = TAG_Gorgeous_UI_System_Message_Result;
				MessageRequestDelegate.BindDynamic(this, &UGorgeousUIFoundationSubsystem::OnMessageRequestReceived);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), MessagePushTag, Rules, GPC);
				USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), MessageResultTag, Rules, GPC);
				USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), MessagePushTag, GPC, MessageRequestDelegate);

				bSignalsRegistered = true;
				GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Registered system-wide signals for LocalPlayer via Tick lazy-init."));
			}
		}
	}

	// Self-healing check for input bridge setup (in case PlayerController/InputComponent was null on first call)
	if (BridgedBindingHandles.IsEmpty() && !ActiveInputBindings.IsEmpty())
	{
		SetupInputBridgeOnHUD();
	}

	// Process Juicy interpolation for all registered widgets
	for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
	{
		if (UObject* WidgetObject = It->GetObject())
		{
			IGorgeousUIWidget_I::Execute_TickInterpolation(WidgetObject, DeltaTime);
		}
		else
		{
			It.RemoveCurrent();
		}
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

	// 2. Register system-wide signals (moved to OnPlayerControllerAdded)
}


#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"

TArray<UGorgeousUITheme_DA*> UGorgeousUIFoundationSubsystem::GetCurrentThemes() const
{
	TArray<UGorgeousUITheme_DA*> AffectedThemes;
	
	if (CurrentThemes.IsValidIndex(CurrentThemes.Num() - 1))
	{
		AffectedThemes.Add(const_cast<TArray<UGorgeousUITheme_DA*>::ElementType>(UGorgeousUIProcessor::GetDefaultTheme()));
	}
	AffectedThemes.Append(CurrentThemes);
	
	return AffectedThemes;
}

UGorgeousUITheme_DA* UGorgeousUIFoundationSubsystem::GetMostRecentTheme() const
{	
	return GetCurrentThemes().Last();
}

void UGorgeousUIFoundationSubsystem::SetCurrentTheme(UGorgeousUITheme_DA* NewTheme)
{
	SCOPE_CYCLE_COUNTER(STAT_GUI_SetTheme);
	if (const UGorgeousUITheme_DA* AffectedTheme = CurrentThemes.IsValidIndex(CurrentThemes.Num() - 1) ? CurrentThemes.Last() : nullptr; 
		AffectedTheme== NewTheme) return;
	CurrentThemes.Push(NewTheme);

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
	for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
	{
		if (IGorgeousUIWidget_I* WidgetInterface = It->GetInterface())
		{
			WidgetInterface->OnThemeApplied(Theme);
		}
		else
		{
			It.RemoveCurrent();
		}
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
	if (CurrentThemes.Last())
	{
		BroadcastThemeApplied(CurrentThemes.Last());
	}
}

void UGorgeousUIFoundationSubsystem::PushInputBinding(UGorgeousInputBinding_DA* NewBindings)
{
	if (!NewBindings) return;

	if (const UGorgeousInputBinding_DA* AffectedBinding = ActiveInputBindings.IsValidIndex(ActiveInputBindings.Num() - 1) ? ActiveInputBindings.Last() : nullptr; 
		AffectedBinding == NewBindings) return;
	
	ActiveInputBindings.Push(NewBindings);

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
					if (Config.bIsDefaultMapping)
					{
						continue;
					}

					if (UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
					{
						InputSubsystem->RemoveMappingContext(IMC);
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
				if (Config.bIsDefaultMapping)
				{
					continue;
				}

				if (UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
				{
					InputSubsystem->RemoveMappingContext(IMC);
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
	if (ActiveInputBindings.IsEmpty()) return;

	ULocalPlayer* LP = GetLocalPlayer();
	APlayerController* PC = LP ? LP->GetPlayerController(GetWorld()) : nullptr;
	AGorgeousHUD* HUD = PC ? Cast<AGorgeousHUD>(PC->GetHUD()) : nullptr;

	if (!HUD || !PC) 
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &UGorgeousUIFoundationSubsystem::SetupInputBridgeOnHUD);
		}
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EIC) 
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &UGorgeousUIFoundationSubsystem::SetupInputBridgeOnHUD);
		}
		return;
	}

	// Clear previously bridged action bindings using stored handles to prevent duplicate callbacks
	for (uint32 Handle : BridgedBindingHandles)
	{
		EIC->RemoveBindingByHandle(Handle);
	}
	BridgedBindingHandles.Empty();

	for (auto ActiveInputBinding : ActiveInputBindings)
	{
		if (!ActiveInputBinding) continue;

		for (const auto& Pair : ActiveInputBinding->Bindings)
		{
			if (UInputAction* Action = Pair.Value.Action)
			{
				// CRITICAL: Tells Enhanced Input whether to swallow the hardware key or let it pass through
				Action->bConsumeInput = Pair.Value.bConsumeInput;
				
				// Determine which trigger events to bind (custom configuration)
				TArray<ETriggerEvent> EventsToBind = Pair.Value.TriggerEventsToBind;
				if (EventsToBind.IsEmpty())
				{
					EventsToBind = { ETriggerEvent::Started, ETriggerEvent::Triggered, ETriggerEvent::Completed, ETriggerEvent::Canceled };
				}

				for (ETriggerEvent Event : EventsToBind)
				{
					FEnhancedInputActionEventBinding& Binding = EIC->BindAction(Action, Event, this, &UGorgeousUIFoundationSubsystem::HandleBridgedInputAction);
					BridgedBindingHandles.Add(Binding.GetHandle());
				}
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

	// Iterate in reverse priority order (state-specific bindings first, then default mappings)
	for (int32 i = ActiveInputBindings.Num() - 1; i >= 0; --i)
	{
		if (bInputWasConsumed) break;

		const UGorgeousInputBinding_DA* ActiveInputBinding = ActiveInputBindings[i];
		if (!ActiveInputBinding) continue;

		for (const auto& Pair : ActiveInputBinding->Bindings)
		{
			if (Pair.Value.Action == Action)
			{
				FGameplayTag DispatchedTag = Pair.Key;
				if (const FGameplayTag* RedirectedTag = ActiveInputRedirects.Find(DispatchedTag))
				{
					DispatchedTag = *RedirectedTag;
				}

				// Pass the custom bConsumeInput rule down to the HUD dispatcher
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
	const FGorgeousUIMessageRequest* Request = Payload.GetPtr<FGorgeousUIMessageRequest>();
	if (!Request || !Request->Config) return;

	// 1. WOW FACTOR: Environmental Shift
	// If the message has a state (e.g. Pause), apply it now
	if (Request->Config->State)
	{
		BroadcastStateSwitch(Request->Config->State);
	}

	// 2. THEME OVERRIDE
	// If the message has a specific theme (e.g. "Danger Mode"), apply it globally
	if (Request->Config->MessageTheme)
	{
		BroadcastThemeApplied(Request->Config->MessageTheme);
	}

	// 3. WIDGET SPAWNING (Deferred to the UI Layout System)
	// In a real implementation, we would push this to a CommonActivatableWidgetStack
	// For this foundation, we broadcast a signal that the UI Layout should handle.
	USignalBridgeBlueprintFunctionLibrary::Dispatch(GetWorld(), TAG_Gorgeous_UI_Layout_PushWidget, Payload);
}

#include "CommonButtonBase.h"

void UGorgeousUIFoundationSubsystem::OnInputActionReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const FGorgeousInputActionPayload* ActionPayload = Payload.GetPtr<FGorgeousInputActionPayload>();
	if (!ActionPayload || !ActionPayload->ActionTag.IsValid()) return;

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
	UGorgeousUIProcessor::ApplyPropertyToTarget(TargetWidget, *FString::Printf(TEXT("Handle%s"), *ActionName), FInstancedStruct());
	UGorgeousUIProcessor::ApplyPropertyToTarget(TargetWidget, *FString::Printf(TEXT("On%s"), *ActionName), FInstancedStruct());
}

void UGorgeousUIFoundationSubsystem::OnFocusRequestReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	const FGorgeousFocusRequestPayload* FocusPayload = Payload.GetPtr<FGorgeousFocusRequestPayload>();
	if (!FocusPayload || !FocusPayload->TargetTag.IsValid()) return;

	// Find the widget with the target tag
	bool bFound = false;
	for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
	{
		if (IGorgeousUIWidget_I* WidgetInterface = It->GetInterface())
		{
			if (WidgetInterface->GetBindingTag() == FocusPayload->TargetTag)
			{
				if (FocusPayload->RoutingID.IsNone() || WidgetInterface->GetRoutingID() == FocusPayload->RoutingID)
				{
					UUserWidget* Widget = Cast<UUserWidget>(It->GetObject());
					
					// If it's an activatable widget, activate it (which usually handles focus)
					if (UCommonActivatableWidget* Activatable = Cast<UCommonActivatableWidget>(Widget))
					{
						Activatable->ActivateWidget();
					}
					else
					{
						Widget->SetFocus();
					}
					bFound = true;
					break;
				}
			}
		}
	}

	// Defer focus request if the widget hasn't registered yet
	if (!bFound && GetWorld())
	{
		FGorgeousPendingFocusRequest PendingReq;
		PendingReq.TargetTag = FocusPayload->TargetTag;
		PendingReq.RoutingID = FocusPayload->RoutingID;
		PendingReq.ExpirationTime = GetWorld()->GetTimeSeconds() + 1.0; // 1 second expiry
		PendingFocusRequests.Add(PendingReq);
	}
}

void UGorgeousUIFoundationSubsystem::HandleGlobalFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget)
{
	if (!NewFocusedWidget.IsValid()) return;

	// Check if this was a navigation event for sound ticks
	if (FocusEvent.GetCause() == EFocusCause::Navigation)
	{
		if (GetWorld())
		{
			USignalBridgeBlueprintFunctionLibrary::Dispatch(GetWorld(), TAG_Gorgeous_UI_Focus_Navigated, FInstancedStruct());
		}
	}

	// Find if the new focused SWidget belongs to one of our registered IGorgeousUIWidget_I objects
	for (auto& Entry : RegisteredWidgets)
	{
		UUserWidget* UserWidget = Cast<UUserWidget>(Entry.GetObject());
		if (UserWidget && UserWidget->GetCachedWidget() == NewFocusedWidget)
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
					
					USignalBridgeBlueprintFunctionLibrary::Dispatch(GetWorld(), TAG_Gorgeous_UI_Focus_Changed, FInstancedStruct::Make(PayloadOut));
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
	const UGorgeousUIState_DA* AffectedState = CurrentStates.IsValidIndex(CurrentStates.Num() - 1) ? CurrentStates.Last() : nullptr;
	if (!NewState || AffectedState == NewState) return;

	if (bImmediate)
	{
		PendingState = NewState;
		ExecuteStateSwap();
		return;
	}

	PendingState = NewState;
	TransitioningWidgets.Empty();

	for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
	{
		if (UObject* WidgetObj = It->GetObject())
		{
			if (const UUserWidget* UserWidget = Cast<UUserWidget>(WidgetObj))
			{
				if (UserWidget->IsVisible())
				{
					TransitioningWidgets.Add(WidgetObj);
				}
			}
		}
	}

	if (TransitioningWidgets.Num() > 0)
	{
		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Starting State Transition to '%s'. Waiting for %d widgets..."), *NewState->GetName(), TransitioningWidgets.Num());
		OnTransitionStarted.Broadcast(NewState);
	}
	else
	{
		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Pushing State '%s' (Immediate)."), *NewState->GetName());
		ExecuteStateSwap();
	}
}

void UGorgeousUIFoundationSubsystem::RemoveUIState(TSubclassOf<UGorgeousUIState_DA> StateClass, bool bImmediate)
{
	if (!StateClass) return;

	int32 RemoveIndex = INDEX_NONE;
	for (int32 i = CurrentStates.Num() - 1; i >= 0; --i)
	{
		if (UGorgeousUIState_DA* State = CurrentStates[i])
		{
			if (State->IsA(StateClass))
			{
				RemoveIndex = i;
				break;
			}
		}
	}

	if (RemoveIndex == INDEX_NONE) return;

	const bool bWasTopState = (RemoveIndex == CurrentStates.Num() - 1);
	CurrentStates.RemoveAt(RemoveIndex);
	if (StateFocusHistory.IsValidIndex(RemoveIndex))
	{
		StateFocusHistory.RemoveAt(RemoveIndex);
	}

	if (!bWasTopState)
	{
		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Removed non-active state '%s' from stack."), *StateClass->GetName());
		return;
	}

	UGorgeousUIState_DA* NewTopState = CurrentStates.IsValidIndex(CurrentStates.Num() - 1) ? CurrentStates.Last() : nullptr;
	if (!NewTopState)
	{
		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Removed active state '%s'. No fallback state available."), *StateClass->GetName());
		return;
	}

	if (bImmediate)
	{
		PendingState = NewTopState;
		ExecuteStateSwap();
		return;
	}

	PendingState = NewTopState;
	TransitioningWidgets.Empty();

	for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
	{
		if (UObject* WidgetObj = It->GetObject())
		{
			if (const UUserWidget* UserWidget = Cast<UUserWidget>(WidgetObj))
			{
				if (UserWidget->IsVisible())
				{
					TransitioningWidgets.Add(WidgetObj);
				}
			}
		}
	}

	if (TransitioningWidgets.Num() > 0)
	{
		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Removing state '%s'. Waiting for %d widgets..."), *StateClass->GetName(), TransitioningWidgets.Num());
		OnTransitionStarted.Broadcast(NewTopState);
	}
	else
	{
		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Removing state '%s' (Immediate)."), *StateClass->GetName());
		ExecuteStateSwap();
	}
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
	if (!PendingState || !Widget) return;

	TransitioningWidgets.Remove(Widget);

	if (TransitioningWidgets.Num() == 0)
	{
		ExecuteStateSwap();
	}
}

void UGorgeousUIFoundationSubsystem::ExecuteStateSwap()
{
	if (!PendingState) return;

	UGorgeousUIState_DA* NewState = PendingState;
	PendingState = nullptr;
	
	// Restore focus memory if available
	bool bRestoredMemory = false;
	if (StateFocusHistory.IsValidIndex(StateFocusHistory.Num() - 1))
	{
		const FGameplayTag TagToRestore = StateFocusHistory.Last();
		if (TagToRestore.IsValid() && GetWorld())
		{
			FGorgeousFocusRequestPayload FocusReq;
			FocusReq.TargetTag = TagToRestore;
			USignalBridgeBlueprintFunctionLibrary::Dispatch(GetWorld(), TAG_Gorgeous_UI_Focus_Request, FInstancedStruct::Make(FocusReq));
			bRestoredMemory = true;
		}
	}

	// Apply Fallback focus strategy if memory restoration didn't occur
	if (!bRestoredMemory && NewState)
	{
		if (NewState->FocusFallback.Strategy == EGorgeousFocusFallbackStrategy_E::FocusSpecificWidget)
		{
			if (NewState->FocusFallback.TargetTag.IsValid() && GetWorld())
			{
				FGorgeousFocusRequestPayload FocusReq;
				FocusReq.TargetTag = NewState->FocusFallback.TargetTag;
				USignalBridgeBlueprintFunctionLibrary::Dispatch(GetWorld(), TAG_Gorgeous_UI_Focus_Request, FInstancedStruct::Make(FocusReq));
			}
		}
	}

	TransitioningWidgets.Empty();

	const UGorgeousUIState_DA* AffectedState = CurrentStates.IsValidIndex(CurrentStates.Num() - 1) ? CurrentStates.Last() : nullptr;
	
	// 1. Remove old Input Bindings
	if (AffectedState && !NewState->bAdditiveToCurrentState)
	{
		if (AffectedState->InputBindings)
		{
			RemoveInputBinding(AffectedState->InputBindings);
		}
	}

	if (!CurrentStates.IsValidIndex(CurrentStates.Num() - 1) || CurrentStates.Last() != NewState)
	{
		// Track the focus of the current state before we push the new one
		FGameplayTag LastFocusedTag = FGameplayTag::EmptyTag;
		for (auto& Entry : RegisteredWidgets)
		{
			if (UUserWidget* UserWidget = Cast<UUserWidget>(Entry.GetObject()))
			{
				if (UserWidget->HasAnyUserFocus())
				{
					if (IGorgeousUIWidget_I* WidgetInterface = Entry.GetInterface())
					{
						LastFocusedTag = WidgetInterface->GetBindingTag();
					}
					break;
				}
			}
		}
		StateFocusHistory.Push(LastFocusedTag);

		CurrentStates.Push(NewState);
	}

	if (CurrentStates.Last())
	{
		// 2. Apply Theme
		if (GetMostRecentUIState()->Theme)
		{
			SetCurrentTheme(GetMostRecentUIState()->Theme);
		}

		// 4. Apply Input Bindings
		if (CurrentStates.Last()->InputBindings)
		{
			PushInputBinding(CurrentStates.Last()->InputBindings);
		}

		// 5. Notify all registered widgets of the state change
		for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
		{
			if (IGorgeousUIWidget_I* WidgetInterface = It->GetInterface())
			{
				UObject* RawObj = It->GetObject();
				if (RawObj)
				{
					// Trigger state change notification
					if (RawObj->IsA(UGorgeousCommonWidget::StaticClass()))
					{
						UGorgeousCommonWidget* CommonWidget = static_cast<UGorgeousCommonWidget*>(RawObj);
						CommonWidget->OnUIStateChanged(CurrentStates.Last());
					}
					else if (RawObj->IsA(UGorgeousActivatableWidget::StaticClass()))
					{
						UGorgeousActivatableWidget* Activatable = static_cast<UGorgeousActivatableWidget*>(RawObj);
						Activatable->OnUIStateChanged(CurrentStates.Last());
					}

					// Apply Overlay Configuration if defined for this widget's tag
					if (CurrentStates.Last()->OverlayConfig)
					{
						const FGameplayTag BindingTag = WidgetInterface->GetBindingTag();
						if (BindingTag.IsValid())
						{
							if (const FGorgeousUIStateConfig* Config = CurrentStates.Last()->OverlayConfig->WidgetConfigs.Find(BindingTag))
							{
								WidgetInterface->ApplyOverlayConfig(*Config);
							}
						}
					}
				}
			}
			else
			{
				It.RemoveCurrent();
			}
		}

		OnTransitionFinished.Broadcast(CurrentStates.Last());

		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: State Transition to '%s' Complete."), *CurrentStates.Last()->GetName());
	}
}

void UGorgeousUIFoundationSubsystem::RegisterWidget(IGorgeousUIWidget_I* Widget)
{
	if (!Widget) return;

	UObject* WidgetObj = Widget->GetAsWidget();
	RegisteredWidgets.AddUnique(TScriptInterface<IGorgeousUIWidget_I>(WidgetObj));

	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Registered Widget '%s' (Binding Tag: %s)"), *WidgetObj->GetName(), *Widget->GetBindingTag().ToString());

	// 1. Initial Aesthetic Application
	if (!CurrentThemes.IsEmpty())
	{
		for (auto Theme : CurrentThemes)
		{
			Widget->OnThemeApplied(Theme);
		}
	}

	// 2. Setup Shared Processor for Signal Handling
	const FGameplayTag BindingTag = Widget->GetBindingTag();
	if (BindingTag.IsValid())
	{
		// Ensure a single SignalBridge listener exists per tag. A forwarder object will
		// receive the raw payload and ask the subsystem to dispatch to the appropriate
		// shared processors for registered widgets that match the tag.
		FGorgeousSignalBridgeAccessRules_S Rules;
		AGorgeousPlayerController* Controller = nullptr;
		if (UWorld* __World = GetWorld())
		{
			if (APlayerController* PC = __World->GetFirstPlayerController())
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

			USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), BindingTag, Rules, Controller);
			USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), BindingTag, Controller, ListenDelegate);
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
				if (PendingFocusRequests[i].TargetTag == Tag)
				{
					if (PendingFocusRequests[i].RoutingID.IsNone() || PendingFocusRequests[i].RoutingID == WidgetInterface->GetRoutingID())
					{
						UUserWidget* UserWidget = Cast<UUserWidget>(Widget->GetAsWidget());
						if (UCommonActivatableWidget* Activatable = Cast<UCommonActivatableWidget>(UserWidget))
						{
							Activatable->ActivateWidget();
						}
						else if (UserWidget)
						{
							UserWidget->SetFocus();
						}
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

	// Cleanup signal listeners for this specific widget
	if (Widget->GetBindingTag().IsValid())
	{
		AGorgeousPlayerController* Controller = nullptr;
		if (UWorld* __World = GetWorld())
		{
			if (APlayerController* PC = __World->GetFirstPlayerController())
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
			USignalBridgeBlueprintFunctionLibrary::Clear(GetWorld(), Tag, Controller);
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
	if (TargetProcessorClass.GetDefaultObject()->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) || TargetProcessorClass->HasAnyClassFlags(CLASS_Abstract))
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
