// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|      Gorgeous Core - PIE-Based Multi-Client Replication Test Harness      |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "UnitTests/Helpers/GorgeousMultiClientTestHarness.h"

#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "UnitTests/Helpers/GorgeousNetworkEmulationHelpers.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#endif

bool FGorgeousMultiClientTestOrchestrator::IsMultiPlayerPIEActive()
{
#if WITH_EDITOR
	if (!GEngine) return false;

	int32 WorldCount = 0;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			++WorldCount;
		}
	}
	return WorldCount > 1;
#else
	return false;
#endif
}

TArray<TPair<FString, UWorld*>> FGorgeousMultiClientTestOrchestrator::CollectPIEWorlds()
{
	TArray<TPair<FString, UWorld*>> Result;

#if WITH_EDITOR
	if (!GEngine) return Result;

	int32 ClientIndex = 0;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType != EWorldType::PIE || !Context.World()) continue;

		UWorld* World = Context.World();
		const ENetMode NetMode = World->GetNetMode();

		FString Label;
		if (NetMode == NM_DedicatedServer)
		{
			Label = TEXT("Server");
		}
		else if (NetMode == NM_ListenServer)
		{
			Label = TEXT("ListenServer");
		}
		else
		{
			Label = FString::Printf(TEXT("Client_%d"), ClientIndex++);
		}

		Result.Emplace(Label, World);
	}
#endif

	return Result;
}

FGorgeousMultiClientEndpointResult FGorgeousMultiClientTestOrchestrator::RunScenariosOnWorld(
	UWorld* World,
	const FString& EndpointLabel,
	bool bIsServer,
	int32 ClientIndex,
	const TArray<FGorgeousInsightScenarioDescriptor>& Scenarios,
	FAutomationTestBase* OptionalTest)
{
	FGorgeousMultiClientEndpointResult Result;
	Result.EndpointLabel = EndpointLabel;
	Result.bIsServer = bIsServer;
	Result.ClientIndex = ClientIndex;

	const double EPStartTime = FPlatformTime::Seconds();

	// ── Capture PIE environment fingerprint for this endpoint ──
	Result.EnvironmentFingerprint = FGorgeousPerfEnvironmentFingerprint::Capture(World);

	FGorgeousInsightMatrixRequest DummyRequest;
	DummyRequest.bEnableSharedHarness = false;

	for (const FGorgeousInsightScenarioDescriptor& Desc : Scenarios)
	{
		++Result.TotalScenarios;

		FGorgeousInsightScenarioContext Ctx(
			DummyRequest,
			FString(),
			0,
			OptionalTest,
			Desc,
			World);

		FGorgeousInsightScenarioResult ScenarioResult = FGorgeousInsightTestMatrix::ExecuteScenario(Desc, Ctx);

		// Inject endpoint provenance into every note
		ScenarioResult.AddNote(FString::Printf(TEXT("ENDPOINT: %s (IsServer=%s, PIE=%s, NetMode=%s)"),
			*EndpointLabel,
			bIsServer ? TEXT("true") : TEXT("false"),
			Result.EnvironmentFingerprint.bIsPlayInEditor ? TEXT("Yes") : TEXT("No"),
			*Result.EnvironmentFingerprint.NetModeName));

		// Count value verifications and PIE proofs from child results
		for (const FString& Note : ScenarioResult.Notes)
		{
			if (Note.Contains(TEXT("VERIFIED"))) ++Result.ValueVerificationCount;
			if (Note.Contains(TEXT("PIE-PROOF"))) ++Result.PIEProofCount;
		}

		FGorgeousInsightScenarioRunResult RunResult;
		RunResult.Descriptor = Desc;
		RunResult.VariantIndex = 0;
		RunResult.Result = ScenarioResult;

		if (ScenarioResult.bSuccess)
		{
			++Result.PassedScenarios;
		}
		else
		{
			++Result.FailedScenarios;
		}

		Result.ScenarioResults.Add(MoveTemp(RunResult));
	}

	Result.EndpointDurationSeconds = FPlatformTime::Seconds() - EPStartTime;
	return Result;
}

bool FGorgeousMultiClientTestOrchestrator::RunTests(const FGorgeousMultiClientTestConfig& Config, FGorgeousMultiClientTestReport& OutReport)
{
	OutReport = FGorgeousMultiClientTestReport();
	OutReport.Config = Config;

	const double StartTime = FPlatformTime::Seconds();

#if WITH_EDITOR
	// ── Step 1: Check if PIE is already running with multiple worlds ──
	if (!IsMultiPlayerPIEActive())
	{
		GT_W_LOG("GT.MultiClientTest", TEXT("[MultiClientHarness] PIE is not active with multiple worlds."));
		GT_W_LOG("GT.MultiClientTest", TEXT("[MultiClientHarness] Please start PIE with %d clients + dedicated server enabled."), Config.NumClients);
		GT_W_LOG("GT.MultiClientTest", TEXT("[MultiClientHarness] Use: Editor → Play → Multiplayer Options → Number of Players = %d, Run Dedicated Server = true"), Config.NumClients + 1);
		return false;
	}

	// ── Step 2: Apply net emulation if requested ──
	if (Config.NetEmulationPreset >= 0)
	{
		FGorgeousNetworkEmulation::SetSuiteDefaultPreset(Config.NetEmulationPreset);
		FGorgeousNetworkEmulation::EnsureSuitePresetApplied(nullptr);
	}

	// ── Step 3: Collect PIE worlds ──
	TArray<TPair<FString, UWorld*>> Worlds = CollectPIEWorlds();
	if (Worlds.Num() == 0)
	{
		GT_W_LOG("GT.MultiClientTest", TEXT("[MultiClientHarness] No PIE worlds found."));
		return false;
	}

	GT_I_LOG("GT.MultiClientTest", TEXT("[MultiClientHarness] Found %d PIE worlds:"), Worlds.Num());
	for (const auto& Pair : Worlds)
	{
		GT_I_LOG("GT.MultiClientTest", TEXT("  - %s (%s)"), *Pair.Key, *Pair.Value->GetName());
	}

	// ── Step 4: Gather scenarios to run ──
	TArray<FGorgeousInsightScenarioDescriptor> Scenarios;
	{
		const TArray<FGorgeousInsightScenarioDescriptor> All = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
		for (const FGorgeousInsightScenarioDescriptor& Desc : All)
		{
			// Skip meta/runall scenarios
			if (Desc.ScenarioName.ToString().Contains(TEXT("RunAll"))) continue;

			bool bMatchesTag = Config.TagFilter.Num() == 0; // empty filter = match all
			for (const FName& Tag : Desc.Tags)
			{
				if (Config.TagFilter.Contains(Tag))
				{
					bMatchesTag = true;
					break;
				}
				// Default: include "comprehensive" tagged scenarios
				if (Config.TagFilter.Num() == 0 && Tag == TEXT("comprehensive"))
				{
					bMatchesTag = true;
					break;
				}
			}
			// Also include the "local" scenarios for cross-validation
			if (!bMatchesTag)
			{
				for (const FName& Tag : Desc.Tags)
				{
					if (Tag == TEXT("local")) { bMatchesTag = true; break; }
				}
			}

			if (bMatchesTag)
			{
				Scenarios.Add(Desc);
			}
		}
	}

	GT_I_LOG("GT.MultiClientTest", TEXT("[MultiClientHarness] Running %d scenarios on %d endpoints."), Scenarios.Num(), Worlds.Num());

	// ── Step 5: Run scenarios on each endpoint ──
	int32 ClientIdx = 0;
	for (const auto& WorldPair : Worlds)
	{
		const bool bIsServer = WorldPair.Key.Contains(TEXT("Server"));
		const int32 CIdx = bIsServer ? INDEX_NONE : ClientIdx++;

		FGorgeousMultiClientEndpointResult EPResult = RunScenariosOnWorld(
			WorldPair.Value, WorldPair.Key, bIsServer, CIdx, Scenarios);

		OutReport.TotalScenarios += EPResult.TotalScenarios;
		OutReport.TotalPassed += EPResult.PassedScenarios;
		OutReport.TotalFailed += EPResult.FailedScenarios;
		OutReport.TotalValueVerifications += EPResult.ValueVerificationCount;
		OutReport.TotalPIEProofs += EPResult.PIEProofCount;
		if (EPResult.EnvironmentFingerprint.IsRealNetEnvironment())
		{
			++OutReport.TotalEndpointsWithRealPIE;
		}

		OutReport.EndpointResults.Add(MoveTemp(EPResult));
	}

	OutReport.TotalDurationSeconds = FPlatformTime::Seconds() - StartTime;
	OutReport.bAllPassed = (OutReport.TotalFailed == 0);

	// ── Step 6: Save the report ──
	const FString ReportText = OutReport.BuildSummary();
	GT_I_LOG("GT.MultiClientTest", TEXT("\n%s"), *ReportText);

	return true;
#else
	GT_W_LOG("GT.MultiClientTest", TEXT("[MultiClientHarness] Editor-only feature. Cannot run outside of the editor."));
	return false;
#endif
}

// ═══════════════════════════════════════════════════════════════════════════
// Insight Matrix Scenario Registration
// ═══════════════════════════════════════════════════════════════════════════

#if WITH_DEV_AUTOMATION_TESTS

static FGorgeousInsightScenarioDescriptor MakeMultiClientOrchestrator()
{
	FGorgeousInsightScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.MultiClient.Orchestrator");
	D.DisplayName  = TEXT("Multi-Client PIE Orchestrator");
	D.Description  = TEXT("Runs all comprehensive scenarios on every PIE endpoint (server + clients) and aggregates results. Requires PIE running with multiple players.");
	D.Tags         = { TEXT("multiclient"), TEXT("orchestrator"), TEXT("comprehensive"), TEXT("pie") };
	D.Priority     = 250;
	D.bEnabledByDefault = false; // Must be explicitly invoked
	D.Runner = [](const FGorgeousInsightScenarioContext& Context) -> FGorgeousInsightScenarioResult
	{
		FGorgeousInsightScenarioResult Result;

		FGorgeousMultiClientTestConfig Config;
		Config.NumClients = 2;
		Config.bUseDedicatedServer = true;

		// Parse optional parameters from the context
		if (!Context.ParameterString.IsEmpty())
		{
			FParse::Value(*Context.ParameterString, TEXT("NumClients="), Config.NumClients);
			FParse::Value(*Context.ParameterString, TEXT("NetProfile="), Config.NetEmulationPreset);

			FString DedicatedStr;
			if (FParse::Value(*Context.ParameterString, TEXT("Dedicated="), DedicatedStr))
			{
				Config.bUseDedicatedServer = DedicatedStr.ToBool();
			}
		}

		FGorgeousMultiClientTestReport Report;
		const bool bStarted = FGorgeousMultiClientTestOrchestrator::RunTests(Config, Report);

		if (!bStarted)
		{
			Result.AddError(TEXT("Multi-client orchestrator could not start. Ensure PIE is running with multiple players."));
			Result.AddNote(TEXT("TIP: In the editor, go to Play → Multiplayer Options → set 'Number of Players' to 3 and enable 'Run Dedicated Server', then start PIE."));
			return Result;
		}

		// Transcribe results
		for (const FGorgeousMultiClientEndpointResult& EP : Report.EndpointResults)
		{
			Result.AddNote(FString::Printf(TEXT("[%s] %d/%d passed | ValVerify=%d | PIEProofs=%d | Duration=%.4fs"),
				*EP.EndpointLabel, EP.PassedScenarios, EP.TotalScenarios,
				EP.ValueVerificationCount, EP.PIEProofCount, EP.EndpointDurationSeconds));

			// Emit per-endpoint PIE fingerprint proof
			Result.AddNote(FString::Printf(TEXT("[%s] ENV: %s"), *EP.EndpointLabel, *EP.EnvironmentFingerprint.ToString()));
			EP.EnvironmentFingerprint.EmitMetrics(Result, FString::Printf(TEXT("multiclient.%s"), *EP.EndpointLabel));

			for (const FGorgeousInsightScenarioRunResult& SR : EP.ScenarioResults)
			{
				if (!SR.Result.bSuccess)
				{
					for (const FString& Err : SR.Result.Errors)
					{
						Result.AddError(FString::Printf(TEXT("[%s][%s] %s"),
							*EP.EndpointLabel, *SR.Descriptor.ScenarioName.ToString(), *Err));
					}
				}
				// Copy proof notes
				for (const FString& Note : SR.Result.Notes)
				{
					if (Note.Contains(TEXT("PROOF:")) || Note.Contains(TEXT("PIE-PROOF")))
					{
						Result.AddNote(FString::Printf(TEXT("[%s] %s"), *EP.EndpointLabel, *Note));
					}
				}
				// Copy child metrics
				for (const auto& MetricPair : SR.Result.Metrics)
				{
					Result.AddMetric(FString::Printf(TEXT("multiclient.%s.%s.%s"),
						*EP.EndpointLabel, *SR.Descriptor.ScenarioName.ToString(), *MetricPair.Key), MetricPair.Value);
				}
			}
		}

		Result.AddMetric(TEXT("multiclient.endpoints"), static_cast<double>(Report.EndpointResults.Num()));
		Result.AddMetric(TEXT("multiclient.totalScenarios"), static_cast<double>(Report.TotalScenarios));
		Result.AddMetric(TEXT("multiclient.totalPassed"), static_cast<double>(Report.TotalPassed));
		Result.AddMetric(TEXT("multiclient.totalFailed"), static_cast<double>(Report.TotalFailed));
		Result.AddMetric(TEXT("multiclient.durationSeconds"), Report.TotalDurationSeconds);
		Result.AddMetric(TEXT("multiclient.totalValueVerifications"), static_cast<double>(Report.TotalValueVerifications));
		Result.AddMetric(TEXT("multiclient.totalPIEProofs"), static_cast<double>(Report.TotalPIEProofs));
		Result.AddMetric(TEXT("multiclient.endpointsWithRealPIE"), static_cast<double>(Report.TotalEndpointsWithRealPIE));

		if (!Report.bAllPassed)
		{
			Result.AddError(FString::Printf(TEXT("%d/%d scenarios failed across all endpoints."),
				Report.TotalFailed, Report.TotalScenarios));
		}

		FGorgeousInsightHarness::SaveScenarioResult(Context.Descriptor, Result, TEXT("multiclient"));
		return Result;
	};
	return D;
}

REGISTER_GORGEOUS_INSIGHT_SCENARIO(MakeMultiClientOrchestrator());

// ═══════════════════════════════════════════════════════════════════════════
// Also register as a standard Unreal Automation Test for Session Frontend
// ═══════════════════════════════════════════════════════════════════════════

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousMultiClientAutomationTest,
	"GorgeousCore.AutoReplication.MultiClient.Orchestrator",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGorgeousMultiClientAutomationTest::RunTest(const FString& Parameters)
{
	FGorgeousMultiClientTestConfig Config;
	Config.NumClients = 2;
	Config.bUseDedicatedServer = true;

	if (!Parameters.IsEmpty())
	{
		FParse::Value(*Parameters, TEXT("NumClients="), Config.NumClients);
		FParse::Value(*Parameters, TEXT("NetProfile="), Config.NetEmulationPreset);
	}

	FGorgeousMultiClientTestReport Report;
	if (!FGorgeousMultiClientTestOrchestrator::RunTests(Config, Report))
	{
		AddWarning(TEXT("PIE is not running with multiple players. Start PIE with Play → Multiplayer Options → Number of Players = 3, Run Dedicated Server = true."));
		return true; // Don't fail the test — just warn
	}

	// Transcribe
	for (const FGorgeousMultiClientEndpointResult& EP : Report.EndpointResults)
	{
		AddInfo(FString::Printf(TEXT("[%s] %d/%d scenarios passed"),
			*EP.EndpointLabel, EP.PassedScenarios, EP.TotalScenarios));

		for (const FGorgeousInsightScenarioRunResult& SR : EP.ScenarioResults)
		{
			if (!SR.Result.bSuccess)
			{
				for (const FString& Err : SR.Result.Errors)
				{
					AddError(FString::Printf(TEXT("[%s][%s] %s"),
						*EP.EndpointLabel, *SR.Descriptor.ScenarioName.ToString(), *Err));
				}
			}
		}
	}

	TestTrue(TEXT("All multi-client scenarios passed"), Report.bAllPassed);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
