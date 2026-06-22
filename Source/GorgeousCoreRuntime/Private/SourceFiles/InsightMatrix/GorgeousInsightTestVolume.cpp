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
			// TODO: Hook into FFunctionalTestingManager for sequential execution
			// Currently a stub to prevent compilation errors if RunTest signature varies.
		}
	}
	
	bIsRunningTests = false;
}
