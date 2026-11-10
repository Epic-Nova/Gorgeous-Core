// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "SharedTests/GorgeousFeedbackAuditEffect.h"
#include "FeedbackEffects/GorgeousFeedbackDispatcher.h"
#include "FeedbackEffects/GorgeousCompositeFeedbackEffect.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousFeedbackSchedulingRegressionTest, "GorgeousCore.FeedbackEffects.FiniteScheduling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousFeedbackSchedulingRegressionTest::RunTest(const FString&)
{
    TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto Advance = [World](float Seconds) { ++GFrameCounter; World->GetTimerManager().Tick(Seconds); };
    Advance(0.f);
    UGorgeousFeedbackAuditEffect* Effect = NewObject<UGorgeousFeedbackAuditEffect>(World);
    Effect->SetFallbackOwner(World);
    Effect->Delay = .05f; Effect->RepeatCount = 3; Effect->RepeatInterval = .1f;
    Effect->PlayFeedback(FGorgeousFeedbackContext{});
    Advance(.06f); TestEqual(TEXT("First delayed play"), Effect->Calls, 1);
    Advance(.1f); Advance(.1f); TestEqual(TEXT("Exactly requested repeats"), Effect->Calls, 3);
    Advance(2.f); TestEqual(TEXT("Repeat timer stops"), Effect->Calls, 3);
    Effect->RepeatInterval = 0.f;
    Effect->PlayFeedback(FGorgeousFeedbackContext{});
    Advance(.06f); TestEqual(TEXT("Delayed zero-interval repeats all execute"), Effect->Calls, 6);
    Effect->PlayFeedback(FGorgeousFeedbackContext{}); Effect->CancelPendingExecutions();
    Advance(1.f); TestEqual(TEXT("Explicit cancellation stops every pending play"), Effect->Calls, 6);
    Effect->Chance = 0.f; Effect->PlayFeedback(FGorgeousFeedbackContext{});
    Advance(1.f); TestEqual(TEXT("Zero chance never plays"), Effect->Calls, 6);
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousFeedbackSequentialRegressionTest, "GorgeousCore.FeedbackEffects.SequentialAndOverride",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousFeedbackSequentialRegressionTest::RunTest(const FString&)
{
    TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto Advance = [World](float Seconds) { ++GFrameCounter; World->GetTimerManager().Tick(Seconds); };
    Advance(0.f);
    UGorgeousFeedbackAuditEffect* First = NewObject<UGorgeousFeedbackAuditEffect>(World); First->SetFallbackOwner(World);
    UGorgeousFeedbackAuditOverrideEffect* Second = NewObject<UGorgeousFeedbackAuditOverrideEffect>(World); Second->SetFallbackOwner(World);
    First->Delay = .1f; First->RepeatCount = 2; First->RepeatInterval = .1f;
    Second->Delay = .1f; Second->RepeatCount = 2;
    TArray<int32> Order;
    First->OnPerformed = [&Order] { Order.Add(1); }; Second->OnPerformed = [&Order] { Order.Add(2); };
    UGameInstance* GI = NewObject<UGameInstance>();
    UGorgeousFeedbackDispatcher* Dispatcher = NewObject<UGorgeousFeedbackDispatcher>(GI);
    FGorgeousFeedbackDefinition Definition;
    Definition.SelectionMode = EGorgeousFeedbackSelectionMode::ESequential; Definition.Effects = { First, Second };
    Dispatcher->PlayDefinition(Definition, FGorgeousFeedbackContext{});
    Advance(.11f); TestEqual(TEXT("First effect starts after its delay"), First->Calls, 1);
    TestEqual(TEXT("Next effect waits for prior repeats"), Second->Calls, 0);
    Advance(.1f); TestEqual(TEXT("First effect completes its repeats"), First->Calls, 2);
    TestEqual(TEXT("Override's own delay retained"), Second->Calls, 0);
    Advance(.11f); TestEqual(TEXT("Override receives both scheduled plays"), Second->Calls, 2);
    TestTrue(TEXT("Sequence order is 1,1,2,2"), Order == TArray<int32>{1,1,2,2});
    First->OnPerformed = nullptr; Second->OnPerformed = nullptr;
    First->CancelPendingExecutions(); Second->CancelPendingExecutions();
    World->DestroyWorld(false);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousFeedbackAsyncCompletionRegressionTest, "GorgeousCore.FeedbackEffects.AsyncCompletion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousFeedbackAsyncCompletionRegressionTest::RunTest(const FString&)
{
    UGorgeousFeedbackAuditAsyncEffect* First = NewObject<UGorgeousFeedbackAuditAsyncEffect>();
    UGorgeousFeedbackAuditEffect* Second = NewObject<UGorgeousFeedbackAuditEffect>();
    UGorgeousFeedbackDispatcher* Dispatcher = NewObject<UGorgeousFeedbackDispatcher>(NewObject<UGameInstance>());
    FGorgeousFeedbackDefinition Definition;
    Definition.SelectionMode = EGorgeousFeedbackSelectionMode::ESequential;
    Definition.Effects = { First, Second };
    Dispatcher->PlayDefinition(Definition, FGorgeousFeedbackContext{});
    TestEqual(TEXT("Async effect starts immediately"), First->Calls, 1);
    TestEqual(TEXT("Following effect waits for actual playback completion"), Second->Calls, 0);
    First->FinishForTest();
    TestEqual(TEXT("Completion advances sequence"), Second->Calls, 1);
    First->FinishForTest();
    TestEqual(TEXT("Duplicate completion does not re-run sequence"), Second->Calls, 1);
    Dispatcher->PlayDefinition(Definition, FGorgeousFeedbackContext{});
    First->StopFeedback(); First->FinishForTest();
    TestEqual(TEXT("Cancel stops async sequence continuation"), Second->Calls, 1);
    return true;
}

#endif
