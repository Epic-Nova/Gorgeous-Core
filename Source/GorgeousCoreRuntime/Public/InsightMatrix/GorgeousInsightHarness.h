// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix Harness (Runtime)            |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"

/** Runtime harness status snapshot for UI/automation consumers. */
struct GORGEOUSCORERUNTIME_API FGorgeousInsightHarnessStatus
{
	bool bHarnessEnabled = false;
	bool bHarnessActive = false;
	bool bRequiresMultiSession = false;
	bool bUsingGauntlet = false;
	FName GauntletState = NAME_None;
	double GauntletStateSeconds = 0.0;
	FString HarnessMapPath;
	FString ListenAddress;
	int32 ServerPort = 0;
	bool bHasConnection = false;
	struct FGorgeousServerClientHarnessStats
	{
		uint64 OutgoingPackets = 0;
		uint64 IncomingPackets = 0;
		double OutgoingBytes = 0.0;
		double IncomingBytes = 0.0;
		double ServerInRateBytesPerSecond = 0.0;
		double ServerOutRateBytesPerSecond = 0.0;
		double ClientInRateBytesPerSecond = 0.0;
		double ClientOutRateBytesPerSecond = 0.0;
	} Stats;
	TArray<FString> NetworkMetricLines;
};

/** Public harness utilities for Insight Matrix runs. */
class GORGEOUSCORERUNTIME_API FGorgeousInsightHarness
{
public:
	/** Start shared harness resources for a matrix run.
	 *  TODO: integrate Gauntlet or native automation orchestration.
	 */
	static bool StartHarness(const FGorgeousInsightMatrixRequest& Request);

	/** Stop shared harness resources. */
	static void StopHarness();

	/** Save scenario result to disk under the given folder.
	 *  Path is relative to Saved/Automation/InsightMatrix.
	 */
	static bool SaveScenarioResult(const FGorgeousInsightScenarioDescriptor& Descriptor, const FGorgeousInsightScenarioResult& Result, const FString& Folder = TEXT(""));

	/** Save provider test result to disk under the given folder.
	 *  Path is relative to Saved/Automation/InsightMatrix.
	 */
	static bool SaveTestResult(const FName ProviderName, const FGorgeousInsightTest& Test, const FGorgeousInsightTestResult& Result, const FString& Folder = TEXT("Tests"));

	/** Returns a snapshot of the current harness status. */
	static FGorgeousInsightHarnessStatus GetStatus();

	/** Returns true when running under Gauntlet. */
	static bool IsGauntletActive();

	/** Returns the Gauntlet module if available. */
	static class FGauntletModule* GetGauntletModule();

	/** Broadcast a Gauntlet state change if Gauntlet is available. */
	static void BroadcastGauntletState(FName NewState);

	/** Return the current Gauntlet state if available. */
	static FName GetGauntletState();

	/** Return time spent in current Gauntlet state if available. */
	static double GetGauntletStateSeconds();

	/** Mark a Gauntlet heartbeat as active if available. */
	static void MarkGauntletHeartbeat(const FString& OptionalStatusMessage = FString());

};
