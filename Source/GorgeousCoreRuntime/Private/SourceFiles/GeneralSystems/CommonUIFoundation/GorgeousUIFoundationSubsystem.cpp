#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidget.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
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
#include "EnhancedInputComponent.h"

#include "CommonInputSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonTextBlock.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonProgressBar.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"

void UGorgeousUIFoundationSubsystem::Tick(float DeltaTime)
{
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

	// 2. Register system-wide signals
	FGorgeousSignalBridgeAccessRules_S Rules;

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (AGorgeousPlayerController* GPC = Cast<AGorgeousPlayerController>(LP->GetPlayerController(GetWorld())))
		{
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

			GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Registered system-wide signals for LocalPlayer."));
		}
	}
}

#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"

void UGorgeousUIFoundationSubsystem::SetCurrentTheme(UGorgeousUITheme_DA* NewTheme)
{
	if (CurrentTheme == NewTheme) return;
	CurrentTheme = NewTheme;

	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Applying Theme '%s'"), NewTheme ? *NewTheme->GetName() : TEXT("None"));

	BroadcastThemeApplied(CurrentTheme);

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

void UGorgeousUIFoundationSubsystem::BroadcastStateSwitch(UGorgeousUIState_DA* NewState)
{
	SwitchUIState(NewState);
}

void UGorgeousUIFoundationSubsystem::OnInputMethodChanged(ECommonInputType NewInputType)
{
	// When the input method changes, we re-broadcast the current theme.
	// Every widget will receive OnThemeApplied and re-fetch its Action Icons 
	// from the theme based on the new platform/input type.
	if (CurrentTheme)
	{
		BroadcastThemeApplied(CurrentTheme);
	}
}

void UGorgeousUIFoundationSubsystem::SetActiveInputBindings(UGorgeousInputBinding_DA* NewBindings)
{
	if (ActiveInputBindings == NewBindings) return;
	ActiveInputBindings = NewBindings;

	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Applying Input Bindings '%s'"), NewBindings ? *NewBindings->GetName() : TEXT("None"));

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
	ULocalPlayer* LP = GetLocalPlayer();
	APlayerController* PC = LP ? LP->GetPlayerController(GetWorld()) : nullptr;
	AGorgeousHUD* HUD = PC ? Cast<AGorgeousHUD>(PC->GetHUD()) : nullptr;

	if (!HUD || !ActiveInputBindings) return;

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EIC) return;

	// Note: In a production environment, we'd want to manage these bindings more carefully 
	// (e.g. removing old ones). For this prototype, we'll assume the HUD manages its own component.
	for (const auto& Pair : ActiveInputBindings->Bindings)
	{
		if (UInputAction* Action = Pair.Value.Action)
		{
			EIC->BindAction(Action, ETriggerEvent::Triggered, this, &UGorgeousUIFoundationSubsystem::HandleBridgedInputAction, Action);
		}
	}
}

void UGorgeousUIFoundationSubsystem::HandleBridgedInputAction(const FInputActionValue& Value, UInputAction* Action)
{
	if (!ActiveInputBindings || !Action) return;

	ULocalPlayer* LP = GetLocalPlayer();
	APlayerController* PC = LP ? LP->GetPlayerController(GetWorld()) : nullptr;
	AGorgeousHUD* HUD = PC ? Cast<AGorgeousHUD>(PC->GetHUD()) : nullptr;

	if (!HUD) return;

	// Find the tag for this action
	for (const auto& Pair : ActiveInputBindings->Bindings)
	{
		if (Pair.Value.Action == Action)
		{
			HUD->DispatchGorgeousInput(Pair.Key, Value);
			break;
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
#if PLATFORM_WINDOWS || PLATFORM_XBOXONE || PLATFORM_XBOXSERIES
	return TEXT("Xbox");
#elif PLATFORM_PS4 || PLATFORM_PS5
	return TEXT("PlayStation");
#elif PLATFORM_SWITCH
	return TEXT("Switch");
#elif PLATFORM_ANDROID || PLATFORM_IOS
	return TEXT("Mobile");
#else
	return TEXT("Generic");
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
	FGameplayTag LayoutPushTag = FGameplayTag::RequestGameplayTag(FName("UI.Layout.PushWidget"));
	USignalBridgeBlueprintFunctionLibrary::Dispatch(GetWorld(), LayoutPushTag, Payload);
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
		static FGameplayTag BackTag = FGameplayTag::RequestGameplayTag("UI.Action.Back");
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
	for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
	{
		if (IGorgeousUIWidget_I* WidgetInterface = It->GetInterface())
		{
			if (WidgetInterface->GetBindingTag() == FocusPayload->TargetTag)
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
				break;
			}
		}
	}
}

void UGorgeousUIFoundationSubsystem::HandlePayloadForTag(FGameplayTag Tag, const FInstancedStruct& Payload)
{
	// Iterate registered widgets and dispatch payloads to shared processors for widgets
	// that have the matching binding tag.
	for (auto& Entry : RegisteredWidgets)
	{
		if (IGorgeousUIWidget_I* WidgetInterface = Entry.GetInterface())
		{
			if (WidgetInterface->GetBindingTag() == Tag)
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

void UGorgeousUIFoundationSubsystem::SwitchUIState(UGorgeousUIState_DA* NewState, bool bImmediate)
{
	if (!NewState || CurrentState == NewState) return;

	if (bImmediate)
	{
		PendingState = NewState;
		ExecuteStateSwap();
		return;
	}

	// 1. Start Transition Phase
	PendingState = NewState;
	TransitioningWidgets.Empty();

	// Gather all currently active widgets that want to participate in the transition
	for (auto It = RegisteredWidgets.CreateIterator(); It; ++It)
	{
		if (UObject* WidgetObj = It->GetObject())
		{
			// We only wait for widgets that are actually visible/active on screen
			if (UUserWidget* UserWidget = Cast<UUserWidget>(WidgetObj))
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
		// The system now waits for NotifyWidgetTransitionComplete calls from these widgets.
		// A fallback timer could be added here to prevent infinite hangs.
	}
	else
	{
		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Switching State to '%s' (Immediate)."), *NewState->GetName());
		ExecuteStateSwap();
	}
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
	TransitioningWidgets.Empty();

	ULocalPlayer* LP = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

	// 1. Remove old IMCs
	if (CurrentState && InputSubsystem)
	{
		for (const FGorgeousInputMappingConfig_S& Config : CurrentState->InputMappingContexts)
		{
			if (UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
			{
				InputSubsystem->RemoveMappingContext(IMC);
			}
		}
	}

	CurrentState = NewState;

	if (CurrentState)
	{
		// 2. Add new IMCs
		if (InputSubsystem)
		{
			for (const FGorgeousInputMappingConfig_S& Config : CurrentState->InputMappingContexts)
			{
				if (UInputMappingContext* IMC = Config.InputMapping.LoadSynchronous())
				{
					InputSubsystem->AddMappingContext(IMC, Config.Priority);
				}
			}
		}

		// 3. Apply Theme
		if (CurrentState->Theme)
		{
			SetCurrentTheme(CurrentState->Theme);
		}

		// 4. Apply Input Bindings
		if (CurrentState->InputBindings)
		{
			SetActiveInputBindings(CurrentState->InputBindings);
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
						CommonWidget->OnUIStateChanged(CurrentState);
					}
					else if (RawObj->IsA(UGorgeousActivatableWidget::StaticClass()))
					{
						UGorgeousActivatableWidget* Activatable = static_cast<UGorgeousActivatableWidget*>(RawObj);
						Activatable->OnUIStateChanged(CurrentState);
					}

					// Apply Overlay Configuration if defined for this widget's tag
					if (CurrentState->OverlayConfig)
					{
						const FGameplayTag BindingTag = WidgetInterface->GetBindingTag();
						if (BindingTag.IsValid())
						{
							if (const FGorgeousUIStateConfig* Config = CurrentState->OverlayConfig->WidgetConfigs.Find(BindingTag))
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

		OnTransitionFinished.Broadcast(CurrentState);

		GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: State Transition to '%s' Complete."), *CurrentState->GetName());
	}
}

void UGorgeousUIFoundationSubsystem::RegisterWidget(IGorgeousUIWidget_I* Widget)
{
	if (!Widget) return;

	UObject* WidgetObj = Widget->GetAsWidget();
	RegisteredWidgets.AddUnique(TScriptInterface<IGorgeousUIWidget_I>(WidgetObj));

	GT_I_LOG("GT.UI", TEXT("UI Foundation Subsystem: Registered Widget '%s' (Binding Tag: %s)"), *WidgetObj->GetName(), *Widget->GetBindingTag().ToString());

	// 1. Initial Aesthetic Application
	if (CurrentTheme)
	{
		Widget->OnThemeApplied(CurrentTheme);
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

	// 2. Find or create the shared instance
	TObjectPtr<UGorgeousUIProcessor>& SharedInstance = SharedProcessors.FindOrAdd(TargetProcessorClass);
	if (!SharedInstance)
	{
		SharedInstance = NewObject<UGorgeousUIProcessor>(this, TargetProcessorClass);
	}

	return SharedInstance;
}
