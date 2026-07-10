// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"

struct FGorgeousSelfReferenceInsightTests
{
	static FGorgeousInsightScenarioResult RunRegistrationPerformanceTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;

		const int32 NumIterations = 1000;
		TMap<FName, FGorgeousObjectVariableEntry> DummyData;
		TArray<UObject*> DummyObjects;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Invalid World Context"));
			return Result;
		}

		// Pre-allocate dummy objects so their creation time is not measured
		for (int32 i = 0; i < NumIterations; ++i)
		{
			DummyObjects.Add(NewObject<USignalBridgeStorage_OV>(World));
		}
		
		double StartTime = FPlatformTime::Seconds();
		
		for (int32 i = 0; i < NumIterations; ++i)
		{
			FGorgeousQualityOfLifeStatics::EnsureSelfReference(DummyObjects[i], DummyData, false);
		}
		
		double EndTime = FPlatformTime::Seconds();
		double TotalTimeMs = (EndTime - StartTime) * 1000.0;
		
		Result.AddNote(FString::Printf(TEXT("Successfully registered %d Self References into the QoL Registry."), NumIterations));
		Result.AddMetric(TEXT("Registered Items"), FString::FromInt(NumIterations));
		Result.AddMetric(TEXT("Total Registration Time"), TotalTimeMs, TEXT("ms"));
		Result.AddMetric(TEXT("Avg Time Per Registration"), TotalTimeMs / NumIterations, TEXT("ms"));
		
		return Result;
	}

	static FGorgeousInsightScenarioResult RunResolutionPerformanceTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;

		const int32 NumItems = 5000;
		const int32 NumResolutions = 1000;
		TMap<FName, FGorgeousObjectVariableEntry> DummyData;
		TArray<UObject*> DummyObjects;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;

		// Seed the registry with objects
		for (int32 i = 0; i < NumItems; ++i)
		{
			UObject* Dummy = NewObject<USignalBridgeStorage_OV>(World);
			DummyObjects.Add(Dummy);
			FGorgeousQualityOfLifeStatics::EnsureSelfReference(Dummy, DummyData, false);
		}
		
		double StartTime = FPlatformTime::Seconds();
		
		int32 FoundCount = 0;
		for (int32 i = 0; i < NumResolutions; ++i)
		{
			TArray<UObject*> Resolved = FGorgeousQualityOfLifeStatics::ResolveSelfReferences(World, USignalBridgeStorage_OV::StaticClass());
			FoundCount += Resolved.Num();
		}
		
		double EndTime = FPlatformTime::Seconds();
		double TotalTimeMs = (EndTime - StartTime) * 1000.0;
		
		Result.AddNote(FString::Printf(TEXT("Successfully resolved Self References %d times. Each resolution found %d objects."), NumResolutions, NumItems));
		Result.AddMetric(TEXT("Resolution Count"), FString::FromInt(NumResolutions));
		Result.AddMetric(TEXT("Objects Returned Per Call"), FString::FromInt(NumItems));
		Result.AddMetric(TEXT("Total Resolution Time"), TotalTimeMs, TEXT("ms"));
		Result.AddMetric(TEXT("Avg Time Per Resolution"), TotalTimeMs / NumResolutions, TEXT("ms"));
		
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeRegistrationScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.QoL.SelfReference.RegistrationPerf");
		D.DisplayName  = TEXT("Self Reference: Registration Performance");
		D.Description  = TEXT("Benchmarks the time it takes to register 1000 new objects into the Self Reference Object Variable registry.");
		D.Tags         = { TEXT("qol"), TEXT("self-reference"), TEXT("performance"), TEXT("GorgeousCore") };
		D.Priority     = 100;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunRegistrationPerformanceTest(Ctx); };
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakeResolutionScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.QoL.SelfReference.ResolutionPerf");
		D.DisplayName  = TEXT("Self Reference: Resolution Performance");
		D.Description  = TEXT("Benchmarks the time it takes to resolve all Self References of a specific class 1000 times, from a pool of 5000 registered objects.");
		D.Tags         = { TEXT("qol"), TEXT("self-reference"), TEXT("performance"), TEXT("GorgeousCore") };
		D.Priority     = 101;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunResolutionPerformanceTest(Ctx); };
		return D;
	}
};

REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousSelfReferenceInsightTests::MakeRegistrationScenario());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousSelfReferenceInsightTests::MakeResolutionScenario());
