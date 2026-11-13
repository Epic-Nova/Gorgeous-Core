// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "InsightMatrix/GorgeousInsightTestVolume.h"
#include "InsightMatrix/GorgeousInsightFunctionalTest.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"

AGorgeousInsightTestVolume::AGorgeousInsightTestVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsRunningTests = false;
}

TArray<AGorgeousInsightFunctionalTest*> AGorgeousInsightTestVolume::GetContainedTests() const
{
	TArray<AGorgeousInsightFunctionalTest*> ContainedTests;
	
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AGorgeousInsightFunctionalTest> It(World); It; ++It)
		{
			AGorgeousInsightFunctionalTest* TestActor = *It;
			if (TestActor && EncompassesPoint(TestActor->GetActorLocation()))
			{
				ContainedTests.Add(TestActor);
			}
		}
	}
	
	return ContainedTests;
}

void AGorgeousInsightTestVolume::RunAllTests()
{
    if (bIsRunningTests || !GetWorld() || !GetWorld()->HasBegunPlay()) return;
    TArray<AGorgeousInsightFunctionalTest*> Tests = GetContainedTests();
    Tests.Sort([](const AGorgeousInsightFunctionalTest& A, const AGorgeousInsightFunctionalTest& B) { return A.GetPathName() < B.GetPathName(); });
    PendingTests.Reset();
    for (int32 Index = Tests.Num() - 1; Index >= 0; --Index) PendingTests.Add(Tests[Index]);
    bIsRunningTests = true;
    GetWorld()->GetTimerManager().SetTimer(QueueWatchdog, this, &ThisClass::PollContainedTest, 0.2f, true);
    RunNextTest();
}

void AGorgeousInsightTestVolume::RunNextTest()
{
    if (ActiveTest.IsValid() && ActiveTest->IsRunning()) return;
    while (!PendingTests.IsEmpty())
    {
        AGorgeousInsightFunctionalTest* Test = PendingTests.Pop().Get();
        if (!IsValid(Test)) continue;
        ActiveTest = Test;
        Test->OnTestFinished.AddUniqueDynamic(this, &ThisClass::HandleContainedTestFinished);
        if (Test->RunTest({})) return;
        Test->OnTestFinished.RemoveDynamic(this, &ThisClass::HandleContainedTestFinished);
        ActiveTest.Reset();
    }
    bIsRunningTests = false;
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(QueueWatchdog);
}

void AGorgeousInsightTestVolume::HandleContainedTestFinished()
{
    if (ActiveTest.IsValid()) ActiveTest->OnTestFinished.RemoveDynamic(this, &ThisClass::HandleContainedTestFinished);
    ActiveTest.Reset();
    if (GetWorld()) GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RunNextTest);
}

void AGorgeousInsightTestVolume::PollContainedTest()
{
    // Destroyed actors must not leave the queue waiting forever.
    if (!ActiveTest.IsValid() || !ActiveTest->IsRunning()) HandleContainedTestFinished();
}

void AGorgeousInsightTestVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ActiveTest.IsValid()) ActiveTest->OnTestFinished.RemoveDynamic(this, &ThisClass::HandleContainedTestFinished);
    if (GetWorld()) GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    ActiveTest.Reset(); PendingTests.Reset(); bIsRunningTests = false;
    Super::EndPlay(EndPlayReason);
}
