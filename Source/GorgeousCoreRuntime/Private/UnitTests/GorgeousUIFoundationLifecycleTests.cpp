// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonBorder.h"
#include "SharedTests/GorgeousUIAuditWidget.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousCommonBorderProcessor.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIExtensions.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIPolicy.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "EnhancedPlayerInput.h"
#include "Misc/ScopeExit.h"
#include "Blueprint/WidgetNavigation.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousUIStateLifecycleRegressionTest,
    "GorgeousCore.Limitations.CommonUI.StateThemeMessageLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousUIStateLifecycleRegressionTest::RunTest(const FString&)
{
    auto* Player = NewObject<ULocalPlayer>(GEngine);
    auto* UI = NewObject<UGorgeousUIFoundationSubsystem>(Player);
    auto* Manual = NewObject<UGorgeousUITheme_DA>();
    auto* A = NewObject<UGorgeousUIState_DA>();
    auto* B = NewObject<UGorgeousUIState_DA>();
    A->Theme = NewObject<UGorgeousUITheme_DA>(); A->InputBindings = NewObject<UGorgeousInputBinding_DA>();
    B->Theme = NewObject<UGorgeousUITheme_DA>(); B->InputBindings = NewObject<UGorgeousInputBinding_DA>();
    B->bAdditiveToCurrentState = true; B->bAdditiveToCurrentTheme = true;
    UI->SetCurrentTheme(Manual); UI->PushUIState(A, true); UI->PushUIState(B, true);
    TestTrue(TEXT("Additive state retains preceding state theme"), UI->GetCurrentThemes().Contains(A->Theme));
    TestEqual(TEXT("Top state theme wins"), UI->GetMostRecentTheme(), B->Theme);
    TestEqual(TEXT("Both additive state bindings are active"), UI->GetActiveInputBindings().Num(), 2);
    UI->StateFocusHistory[0] = TAG_Gorgeous_UI_Layer_HUD;
    UI->StateFocusHistory[1] = TAG_Gorgeous_UI_Layer_ActionBar;
    UI->RemoveUIStateByObject(A, true);
    TestEqual(TEXT("Identity removal preserves same-class upper asset"), UI->GetMostRecentUIState(), B);
    TestFalse(TEXT("Removed state's input binding is released"), UI->GetActiveInputBindings().Contains(A->InputBindings));
    TestFalse(TEXT("Removed state's theme is released"), UI->GetCurrentThemes().Contains(A->Theme));
    TestEqual(TEXT("Focus memory remains aligned with surviving state"), UI->StateFocusHistory[0], TAG_Gorgeous_UI_Layer_ActionBar.GetTag());
    UI->RemoveUIStateByObject(B, true);
    TestTrue(TEXT("Last state removal releases owned bindings"), UI->GetActiveInputBindings().IsEmpty());
    TestEqual(TEXT("Manual theme returns after states end"), UI->GetMostRecentTheme(), Manual);
    for (int32 Index = 0; Index < 100; ++Index) UI->SetCurrentTheme(Index % 2 ? Manual : A->Theme);
    TestEqual(TEXT("Explicit theme replacement remains bounded"), UI->CurrentThemes.Num(), 1);
    auto* Config = NewObject<UGorgeousUIMessageConfig_DA>(); Config->State = A; Config->MessageTheme = B->Theme;
    FGorgeousUIMessageRequest Request; Request.RequestID = FGuid::NewGuid(); Request.Config = Config;
    UI->OnMessageRequestReceived(TAG_Gorgeous_UI_System_Message_Push, FInstancedStruct::Make(Request));
    UI->OnMessageRequestReceived(TAG_Gorgeous_UI_System_Message_Push, FInstancedStruct::Make(Request));
    TestEqual(TEXT("Repeated delivery only presents a request once"), UI->ActiveMessageRequests.Num(), 1);
    TestEqual(TEXT("Message theme participates in resource lookups"), UI->GetMostRecentTheme(), B->Theme);
    FGorgeousUIMessageRequest Other = Request; Other.RequestID = FGuid::NewGuid();
    UI->OnMessageRequestReceived(TAG_Gorgeous_UI_System_Message_Push, FInstancedStruct::Make(Other));
    UI->CompleteMessagePresentation(Request.RequestID);
    TestEqual(TEXT("Shared dialog state survives first completion"), UI->GetMostRecentUIState(), A);
    UI->CompleteMessagePresentation(Other.RequestID);
    TestNull(TEXT("Last completion removes message-owned state"), UI->GetMostRecentUIState());
    TestTrue(TEXT("Message themes are removed after completion"), UI->MessageThemes.IsEmpty());
    UI->PushUIState(A, true);
    Request.RequestID = FGuid::NewGuid();
    UI->OnMessageRequestReceived(TAG_Gorgeous_UI_System_Message_Push, FInstancedStruct::Make(Request));
    UI->CompleteMessagePresentation(Request.RequestID);
    TestEqual(TEXT("Dialog does not remove a pre-existing state"), UI->GetMostRecentUIState(), A);
    UI->RemoveUIStateByObject(A, true);
    // Preserve the public SetCurrentTheme contract across stack changes: a later
    // nonadditive state may mask it, but removing states must restore its ordering.
    UI->PushUIState(A, true); UI->SetCurrentTheme(Manual);
    B->bAdditiveToCurrentTheme = false; UI->PushUIState(B, true);
    TestEqual(TEXT("Later nonadditive state masks explicit earlier theme"), UI->GetMostRecentTheme(), B->Theme);
    UI->RemoveUIStateByObject(A, true);
    TestEqual(TEXT("Removing lower state cannot move explicit theme above upper state"), UI->GetMostRecentTheme(), B->Theme);
    UI->RemoveUIStateByObject(B, true);
    TestEqual(TEXT("Popping all states restores explicit earlier theme"), UI->GetMostRecentTheme(), Manual);
    UI->PushUIState(A, true); UI->PushUIState(B, true); UI->SetCurrentTheme(Manual);
    TestEqual(TEXT("Explicit theme after two states overrides them"), UI->GetMostRecentTheme(), Manual);
    UI->RemoveUIStateByObject(B, true);
    TestEqual(TEXT("Explicit later theme survives upper removal"), UI->GetMostRecentTheme(), Manual);
    UI->RemoveUIStateByObject(A, true);
    TestEqual(TEXT("Explicit later theme survives final removal"), UI->GetMostRecentTheme(), Manual);
    auto* Waiter = NewObject<UGorgeousUIAuditWidget>();
    const TSharedRef<SWidget> WaiterSlate = Waiter->TakeWidget();
    Waiter->SetVisibility(ESlateVisibility::Visible);
    UI->RegisteredWidgets.Add(TScriptInterface<IGorgeousUIWidget_I>(Waiter));
    UI->PushUIState(B, false);
    TestTrue(TEXT("Visible widget starts deferred swap"), UI->bStateTransitionPending);
    UI->Tick(0.f);
    TestFalse(TEXT("Widget without outgoing animation completes automatically"), UI->bStateTransitionPending);
    UI->PushUIState(A, false);
    UI->UnregisterWidget(Waiter);
    TestFalse(TEXT("Destroy/unregister removes final transition waiter"), UI->bStateTransitionPending);
    TestEqual(TEXT("Unregister advances pending target"), UI->GetMostRecentUIState(), A);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousUIFocusAndInputRegressionTest,
    "GorgeousCore.Limitations.CommonUI.FocusAndInputRouting",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousUIFocusAndInputRegressionTest::RunTest(const FString&)
{
    auto* Player = NewObject<ULocalPlayer>(GEngine);
    auto* UI = NewObject<UGorgeousUIFoundationSubsystem>(Player);
    auto* Border = NewObject<UGorgeousCommonBorder>(); Border->BindingTag = TAG_Gorgeous_UI_Layer_HUD;
    auto* Target = NewObject<UGorgeousUIAuditWidget>(); Target->BindingTag = TAG_Gorgeous_UI_Layer_ActionBar;
    UI->RegisteredWidgets.Add(TScriptInterface<IGorgeousUIWidget_I>(Border));
    UI->RegisteredWidgets.Add(TScriptInterface<IGorgeousUIWidget_I>(Target));
    TestTrue(TEXT("Focus request accepts native border without UserWidget cast"), UI->TryFocusWidget(Border->BindingTag, NAME_None));
    auto* ColorTheme = NewObject<UGorgeousUITheme_DA>();
    ColorTheme->StyleProperties.Add(TEXT("BorderColor"), FInstancedStruct::Make(FLinearColor::Red));
    Border->ThemeInterpSpeed = 2.f;
    NewObject<UGorgeousCommonBorderProcessor>()->ApplyThemeToWidget(Border, ColorTheme);
    TestEqual(TEXT("Border interpolation speed controls actual duration"), Border->GetThemeInterpConfig().Duration, .5f);
    FGorgeousPendingFocusRequest Expired; Expired.ExpirationTime = FPlatformTime::Seconds() - 1.;
    UI->PendingFocusRequests.Add(Expired); UI->Tick(0.f);
    TestTrue(TEXT("Deferred focus requests expire"), UI->PendingFocusRequests.IsEmpty());
    auto* State = NewObject<UGorgeousUIState_DA>();
    FGorgeousFocusRoute_S Route; Route.SourceTag = Border->BindingTag; Route.TargetTag = Target->BindingTag; Route.Direction = EUINavigation::Right;
    Border->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Wrap);
    State->FocusRoutes.Add(Route); UI->PushUIState(State, true);
    if (TestNotNull(TEXT("State creates actual UMG navigation"), Border->Navigation.Get()))
    {
        TestEqual(TEXT("Navigation uses explicit route"), Border->Navigation->Right.Rule, EUINavigationRule::Explicit);
        TestEqual(TEXT("Route targets registered widget"), Border->Navigation->Right.Widget.Get(), static_cast<UWidget*>(Target));
    }
    FGorgeousInputActionPayload Input; Input.ActionTag = TAG_Gorgeous_UI_Layer_HUD; Input.TargetWidgetTag = Target->BindingTag;
    UI->OnInputActionReceived(TAG_Gorgeous_UI_Input_Action, FInstancedStruct::Make(Input));
    TestEqual(TEXT("Named reflected handler is actually invoked"), Target->Handled, 1);
    Input.LocalPlayerIndex = 1; UI->OnInputActionReceived(TAG_Gorgeous_UI_Input_Action, FInstancedStruct::Make(Input));
    TestEqual(TEXT("Other local player's input is ignored"), Target->Handled, 1);
    UI->RemoveUIStateByObject(State, true);
    TestEqual(TEXT("Popped state restores authored navigation rule"), Border->Navigation->Right.Rule, EUINavigationRule::Wrap);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto* PC = World->SpawnActor<APlayerController>();
    Player->PlayerController = PC;
    PC->InputComponent = NewObject<UEnhancedInputComponent>(PC);
    auto* Component = CastChecked<UEnhancedInputComponent>(PC->InputComponent);
    auto* Action = NewObject<UInputAction>(); Action->bConsumeInput = true;
    auto* First = NewObject<UGorgeousInputBinding_DA>(); auto* Second = NewObject<UGorgeousInputBinding_DA>();
    FGorgeousInputBindingInfo_S Info; Info.Action = Action; Info.bConsumeInput = false;
    Info.TriggerEventsToBind = { ETriggerEvent::Triggered, ETriggerEvent::Triggered };
    First->Bindings.Add(TAG_Gorgeous_UI_Layer_HUD, Info); Second->Bindings.Add(TAG_Gorgeous_UI_Layer_HUD, Info);
    UI->PushInputBinding(First); UI->PushInputBinding(Second);
    TestEqual(TEXT("Shared action/event is bound only once"), Component->GetActionEventBindings().Num(), 1);
    TestTrue(TEXT("Per-binding consume does not modify shared asset"), Action->bConsumeInput);
    UI->PopInputBinding(); UI->PopInputBinding();
    TestTrue(TEXT("Last removal clears Enhanced Input bridge"), Component->GetActionEventBindings().IsEmpty());
    const FName FirstToken = UGorgeousUIExtensions::SuspendGorgeousInput(PC, TEXT("Audit"));
    const FName SecondToken = UGorgeousUIExtensions::SuspendGorgeousInput(PC, TEXT("Audit"));
    UGorgeousUIExtensions::ResumeGorgeousInput(PC, FirstToken);
    UGorgeousUIExtensions::ResumeGorgeousInput(PC, FirstToken);
    TestTrue(TEXT("Repeated completion cannot release another suspension"), PC->IsMoveInputIgnored());
    UGorgeousUIExtensions::ResumeGorgeousInput(PC, SecondToken);
    TestFalse(TEXT("Last owned token restores input"), PC->IsMoveInputIgnored());
    Player->PlayerController = nullptr;
    World->DestroyWorld(false);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousUIPolicyInputReadinessRegression,
    "GorgeousCore.Limitations.CommonUI.LayoutInputReadiness",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousUIPolicyInputReadinessRegression::RunTest(const FString&)
{
    UClass* PreviousInputClass = UInputSettings::GetDefaultPlayerInputClass();
    ON_SCOPE_EXIT { UInputSettings::SetDefaultPlayerInputClass(PreviousInputClass); };
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, false);
    if (!TestNotNull(TEXT("Actual game world"), World)) return false;
    ON_SCOPE_EXIT { World->DestroyWorld(false); };
    APlayerController* Controller = World->SpawnActor<APlayerController>();
    if (!TestNotNull(TEXT("Actual local controller"), Controller)) return false;
    auto* Player = NewObject<ULocalPlayer>(GEngine);
    Player->PlayerController = Controller;
    Controller->Player = Player;
    ON_SCOPE_EXIT { Player->PlayerController = nullptr; Controller->Player = nullptr; };
    UInputSettings::SetDefaultPlayerInputClass(UEnhancedPlayerInput::StaticClass());
    Controller->PlayerInput = nullptr;
    TestFalse(TEXT("Layout cannot activate before InitInputSystem supplies player input"), UGorgeousUIPolicy::IsPlayerInputReadyForLayout(Player, Controller));
    Controller->PlayerInput = NewObject<UEnhancedPlayerInput>(Controller);
    TestTrue(TEXT("Deferred layout becomes ready with actual EnhancedPlayerInput"), UGorgeousUIPolicy::IsPlayerInputReadyForLayout(Player, Controller));
    Controller->PlayerInput = NewObject<UPlayerInput>(Controller);
    TestFalse(TEXT("Enhanced input configuration cannot activate on a stale legacy input object"), UGorgeousUIPolicy::IsPlayerInputReadyForLayout(Player, Controller));
    UInputSettings::SetDefaultPlayerInputClass(UPlayerInput::StaticClass());
    TestTrue(TEXT("Projects deliberately using legacy input remain supported"), UGorgeousUIPolicy::IsPlayerInputReadyForLayout(Player, Controller));
    TestTrue(TEXT("Worldless previews preserve their existing construction semantics"), UGorgeousUIPolicy::IsPlayerInputReadyForLayout(Player, GetDefault<APlayerController>()));
    return true;
}
#endif
