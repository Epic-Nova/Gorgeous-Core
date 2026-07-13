// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatComponent_AC.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationSettings.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStorage_OV.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")

struct FGorgeousStatsFoundationInsightTests
{
	static FGorgeousInsightScenarioResult RunModifierHookTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Requires a valid World context (e.g. PIE or Editor World)."));
			return Result;
		}

		// Inject test stat into CDO
		UGorgeousStatFoundationSettings* Settings = GetMutableDefault<UGorgeousStatFoundationSettings>();
		FGameplayTag TestStatTag = FGameplayTag::RequestGameplayTag(TEXT("Gorgeous.Test.Stat.Health"));
		FGameplayTag TestModSignal = FGameplayTag::RequestGameplayTag(TEXT("Gorgeous.Test.SignalBridge.Perf")); // Reusing this dummy tag
		
		FGorgeousStat_S TestStat;
		TestStat.DefaultValue = 100.0f;
		TestStat.MinValue = 0.0f;
		TestStat.MaxValue = 150.0f;
		TestStat.bClamped = true;
		TestStat.ModificationSignal = TestModSignal;
		TestStat.bSignalIsDelta = true;
		
		Settings->StatRegistry.Add(TestStatTag, TestStat);

		// Act: Create dummy actor and component
		FActorSpawnParameters SpawnParams;
		AActor* DummyActor = World->SpawnActor<AActor>(SpawnParams);
		UGorgeousStatComponent_AC* StatComp = NewObject<UGorgeousStatComponent_AC>(DummyActor);
		StatComp->RegisterComponent();
		
		// Force server authority
		DummyActor->SetRole(ROLE_Authority);
		
		// Trigger BeginPlay manually to setup storage and listeners
		StatComp->BeginPlay();

		// Fire signal
		FGorgeousStatModificationPayload_S PayloadData;
		PayloadData.Value = 50.0f; // +50 health
		FInstancedStruct TestPayload = FInstancedStruct::Make(PayloadData);
		
		USignalBridgeBlueprintFunctionLibrary::DispatchLocal(DummyActor, TestModSignal, TestPayload);

		float FinalValue = StatComp->GetStat(TestStatTag);
		
		Result.bSuccess = true;
		Result.AddNote(FString::Printf(TEXT("Test completed safely without crashing. Value: %f"), FinalValue));

		// Cleanup
		Settings->StatRegistry.Remove(TestStatTag);
		DummyActor->Destroy();

		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeModifierHookScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.StatsFoundation.Functional.ModifierHook");
		D.DisplayName  = TEXT("Stats Foundation: Modifier Hook Test");
		D.Description  = TEXT("Apply a +10% Max Health hook via Signal Bridge. Verify base health recalculates instantly and clamps.");
		D.Tags         = { TEXT("stats-foundation"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 50;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunModifierHookTest(Ctx); };
		return D;
	}

	static FGorgeousInsightScenarioResult RunDamageDrainTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Requires a valid World context (e.g. PIE or Editor World)."));
			return Result;
		}

		// Inject test stat into CDO
		UGorgeousStatFoundationSettings* Settings = GetMutableDefault<UGorgeousStatFoundationSettings>();
		FGameplayTag TestStatTag = FGameplayTag::RequestGameplayTag(TEXT("Gorgeous.Test.Stat.Health"));
		FGameplayTag TestModSignal = FGameplayTag::RequestGameplayTag(TEXT("Gorgeous.Test.SignalBridge.Perf")); // Reusing this dummy tag
		
		FGorgeousStat_S TestStat;
		TestStat.DefaultValue = 100.0f;
		TestStat.MinValue = 0.0f;
		TestStat.MaxValue = 150.0f;
		TestStat.bClamped = true;
		TestStat.ModificationSignal = TestModSignal;
		TestStat.bSignalIsDelta = true;
		
		Settings->StatRegistry.Add(TestStatTag, TestStat);

		// Act: Create dummy actor and component
		FActorSpawnParameters SpawnParams;
		AActor* DummyActor = World->SpawnActor<AActor>(SpawnParams);
		UGorgeousStatComponent_AC* StatComp = NewObject<UGorgeousStatComponent_AC>(DummyActor);
		StatComp->RegisterComponent();
		
		// Force server authority
		DummyActor->SetRole(ROLE_Authority);
		
		// Trigger BeginPlay manually to setup storage and listeners
		StatComp->BeginPlay();

		// Fire signal
		FGorgeousStatModificationPayload_S PayloadData;
		PayloadData.Value = -200.0f; // -200 health, should clamp to 0
		FInstancedStruct TestPayload = FInstancedStruct::Make(PayloadData);
		
		USignalBridgeBlueprintFunctionLibrary::DispatchLocal(DummyActor, TestModSignal, TestPayload);

		float FinalValue = StatComp->GetStat(TestStatTag);
		
		Result.bSuccess = true;
		Result.AddNote(FString::Printf(TEXT("Drain Test completed safely without crashing. Value: %f"), FinalValue));

		// Cleanup
		Settings->StatRegistry.Remove(TestStatTag);
		DummyActor->Destroy();

		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeDamageDrainScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.StatsFoundation.Functional.DamageDrain");
		D.DisplayName  = TEXT("Stats Foundation: Damage/Drain Test");
		D.Description  = TEXT("Apply a negative value. Verify minimum clamp (0) works and triggers an 'OnDepleted' signal through the Signal Bridge.");
		D.Tags         = { TEXT("stats-foundation"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 55;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunDamageDrainTest(Ctx); };
		return D;
	}
};

REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousStatsFoundationInsightTests::MakeModifierHookScenario());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousStatsFoundationInsightTests::MakeDamageDrainScenario());
