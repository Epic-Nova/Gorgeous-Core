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

#include "InsightMatrix/GorgeousInsightGauntletController.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/World.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"


void UGorgeousInsightGauntletController::OnInit()
{
	FParse::Value(FCommandLine::Get(), TEXT("GorgeousInsightParameters="), MatrixParameters, false);
	FParse::Value(FCommandLine::Get(), TEXT("GorgeousInsightWarmup="), WarmupSeconds);
	FParse::Value(FCommandLine::Get(), TEXT("GorgeousInsightTimeout="), TimeoutSeconds);
	WarmupSeconds = FMath::Max(0.0f, WarmupSeconds);
	TimeoutSeconds = FMath::Max(1.0f, TimeoutSeconds);
	TestWorld = GetWorld();
	MarkHeartbeatActive(TEXT("Waiting for Gorgeous Insight test world"));
}

void UGorgeousInsightGauntletController::OnPostMapChange(UWorld* World)
{
	TestWorld = World;
	ReadySeconds = 0.0f;
}

void UGorgeousInsightGauntletController::OnTick(float TimeDelta)
{
	if (bFinished) return;
	ElapsedSeconds += TimeDelta;
	if (ElapsedSeconds >= TimeoutSeconds)
	{
		bFinished = true;
		GT_E_LOG(TEXT("GT.InsightMatrix.Gauntlet.WorldTimeout"), TEXT("Insight suite timed out waiting for a ready game world."));
		EndTest(3);
		return;
	}
	UWorld* World = TestWorld.IsValid() ? TestWorld.Get() : GetWorld();
	if (!World || !World->IsGameWorld() || !World->HasBegunPlay()) return;
	ReadySeconds += TimeDelta;
	if (ReadySeconds < WarmupSeconds) return;
	UGorgeousInsightMatrixSubsystem* Matrix = UGorgeousInsightMatrixSubsystem::Get();
	if (!Matrix) return;
	bFinished = true;
	MarkHeartbeatActive(TEXT("Running Gorgeous Insight scenarios"));
	const TArray<FGorgeousInsightScenarioRunResult> Results = Matrix->RunMatrix(MatrixParameters, World);
	if (Results.IsEmpty())
	{
		GT_E_LOG(TEXT("GT.InsightMatrix.Gauntlet.NoScenariosExecuted"), TEXT("No scenarios executed. Use a Development build with automation tests and a matching scenario filter."));
		EndTest(2);
		return;
	}
	int32 Failed = 0;
	for (const FGorgeousInsightScenarioRunResult& Result : Results)
	{
		if (!Result.Result.bSuccess) ++Failed;
		GT_D_LOG(TEXT("GT.InsightMatrix.Gauntlet.ScenarioResult"), TEXT("%s: %s"), *Result.Descriptor.ScenarioName.ToString(), Result.Result.bSuccess ? TEXT("PASS") : TEXT("FAIL"));
	}
	GT_D_LOG(TEXT("GT.InsightMatrix.Gauntlet.SuiteFinished"), TEXT("Insight Gauntlet finished: %d executed, %d failed."), Results.Num(), Failed);
	EndTest(Failed == 0 ? 0 : 1);
}
