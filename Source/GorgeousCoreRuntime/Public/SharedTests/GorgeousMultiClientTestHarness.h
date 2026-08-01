// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousObjectVariablePerfTestTypes.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

#if WITH_EDITOR
#include "Editor.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#endif

struct FGorgeousMultiClientEndpointResult
{
	FString EndpointLabel; // "Server", "Client_0", "Client_1" etc.
	bool bIsServer = false;
	int32 ClientIndex = INDEX_NONE;
	TArray<FGorgeousInsightScenarioRunResult> ScenarioResults;
	int32 TotalScenarios = 0;
	int32 PassedScenarios = 0;
	int32 FailedScenarios = 0;

	/** PIE environment fingerprint captured for this endpoint. */
	FGorgeousPerfEnvironmentFingerprint EnvironmentFingerprint;

	/** Aggregate stats: value verification count, PIE proof count. */
	int32 ValueVerificationCount = 0;
	int32 PIEProofCount = 0;
	double EndpointDurationSeconds = 0.0;
};

struct FGorgeousMultiClientTestConfig
{
	/** Number of clients to spawn in PIE (default 2). */
	int32 NumClients = 2;

	/** Use a dedicated server (vs listen server). */
	bool bUseDedicatedServer = true;

	/** Map to use for the PIE session (empty = current editor map). */
	FString MapPath;

	/** Seconds to wait for all PIE worlds to settle connections. */
	float ConnectionTimeoutSeconds = 15.0f;

	/** Seconds after connections settle before running scenarios. */
	float WarmupSeconds = 2.0f;

	/** Tag filter, empty = run all 'comprehensive' scenarios. */
	TSet<FName> TagFilter;

	/** Network emulation preset (-1 = none). */
	int32 NetEmulationPreset = INDEX_NONE;
};

struct FGorgeousMultiClientTestReport
{
	FGorgeousMultiClientTestConfig Config;
	TArray<FGorgeousMultiClientEndpointResult> EndpointResults;
	int32 TotalScenarios = 0;
	int32 TotalPassed = 0;
	int32 TotalFailed = 0;
	double TotalDurationSeconds = 0.0;
	bool bAllPassed = false;

	/** Aggregate stats across all endpoints. */
	int32 TotalValueVerifications = 0;
	int32 TotalPIEProofs = 0;
	int32 TotalEndpointsWithRealPIE = 0;

	FString BuildSummary() const
	{
		FString Summary;
		Summary += FString::Printf(TEXT("=== Gorgeous MultiClient Replication Test Report ===\n"));
		Summary += FString::Printf(TEXT("Config: %d clients, DedicatedServer=%s\n"),
			Config.NumClients, Config.bUseDedicatedServer ? TEXT("Yes") : TEXT("No"));
		Summary += FString::Printf(TEXT("Duration: %.2fs\n"), TotalDurationSeconds);
		Summary += FString::Printf(TEXT("Result: %s (%d/%d passed)\n"),
			bAllPassed ? TEXT("ALL PASSED") : TEXT("FAILURES DETECTED"),
			TotalPassed, TotalScenarios);
		Summary += FString::Printf(TEXT("PIE Proof: %d endpoints with real PIE, %d value verifications, %d PIE proofs\n\n"),
			TotalEndpointsWithRealPIE, TotalValueVerifications, TotalPIEProofs);

		for (const FGorgeousMultiClientEndpointResult& EP : EndpointResults)
		{
			Summary += FString::Printf(TEXT("  [%s] %d/%d passed | ValVerify=%d | PIEProofs=%d | Duration=%.4fs\n"),
				*EP.EndpointLabel, EP.PassedScenarios, EP.TotalScenarios,
				EP.ValueVerificationCount, EP.PIEProofCount, EP.EndpointDurationSeconds);
			Summary += FString::Printf(TEXT("    ENV: %s\n"), *EP.EnvironmentFingerprint.ToString());

			for (const FGorgeousInsightScenarioRunResult& SR : EP.ScenarioResults)
			{
				if (!SR.Result.bSuccess)
				{
					Summary += FString::Printf(TEXT("    FAIL: %s\n"), *SR.Descriptor.ScenarioName.ToString());
					for (const FString& Err : SR.Result.Errors)
					{
						Summary += FString::Printf(TEXT("      - %s\n"), *Err);
					}
				}
			}
		}

		return Summary;
	}
};

/**
 * Static orchestrator that drives multi-client PIE-based replication testing.
 */
class GORGEOUSCORERUNTIME_API FGorgeousMultiClientTestOrchestrator
{
public:
	/**
	 * Runs the full multi-client test pipeline synchronously (editor-only).
	 * Returns false if PIE cannot be started.
	 */
	static bool RunTests(const FGorgeousMultiClientTestConfig& Config, FGorgeousMultiClientTestReport& OutReport);

	/**
	 * Returns true when a PIE session is currently running with multiple worlds.
	 */
	static bool IsMultiPlayerPIEActive();

	/**
	 * Collects all PIE worlds into an array labeled "Server"/"Client_N".
	 */
	static TArray<TPair<FString, UWorld*>> CollectPIEWorlds();

	/**
	 * Runs the provided scenarios on a single world and returns results.
	 */
	static FGorgeousMultiClientEndpointResult RunScenariosOnWorld(
		UWorld* World,
		const FString& EndpointLabel,
		bool bIsServer,
		int32 ClientIndex,
		const TArray<FGorgeousInsightScenarioDescriptor>& Scenarios,
		FAutomationTestBase* OptionalTest = nullptr);
};