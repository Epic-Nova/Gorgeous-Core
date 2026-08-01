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
	if (bIsRunningTests)
	{
		return;
	}

	bIsRunningTests = true;

	TArray<AGorgeousInsightFunctionalTest*> Tests = GetContainedTests();
	for (AGorgeousInsightFunctionalTest* Test : Tests)
	{
		if (Test)
		{
			// TODO: Hook into FFunctionalTestingManager for proper sequential execution.
			// TODO: Hook into the Insight Matrix — each test result should be forwarded to
			//       UGorgeousInsightMatrixSubsystem so results appear under the Tests tab in
			//       the Insight Matrix Live view. Results should only be visible while a game
			//       session is active (i.e. surfaced exclusively in the Live tab, not the
			//       static/editor stats view).
		}
	}
	
	bIsRunningTests = false;
}
