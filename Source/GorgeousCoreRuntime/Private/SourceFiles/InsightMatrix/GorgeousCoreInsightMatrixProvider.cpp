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

#include "InsightMatrix/GorgeousCoreInsightMatrixProvider.h"

#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "InsightMatrix/GorgeousInsightStatBuilder.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "ObjectVariables/Slate/GorgeousObjectVariableBrowserWindow.h"
#include "AutoReplication/Slate/SGorgeousNetworkTrafficInspectorWindow.h"
#include "AutoReplication/Slate/SGorgeousRPCInspectorWindow.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const FName ActionOpenOVBrowser(TEXT("Core.OV.Browser"));
	const FName ActionOpenTrafficInspector(TEXT("AR.Window.TrafficInspector"));
	const FName ActionOpenRPCInspector(TEXT("AR.Window.RPCInspector"));

	bool IsCoreScenario(const FGorgeousInsightScenarioDescriptor& Descriptor)
	{
		return Descriptor.Tags.Contains(FName(TEXT("GorgeousCore"))) || Descriptor.Tags.Contains(FName(TEXT("System.Core")));
	}

	bool IsARScenario(const FGorgeousInsightScenarioDescriptor& Descriptor)
	{
		return Descriptor.Tags.Contains(FName(TEXT("System.AutoReplication")));
	}

	FString BuildParameterString(const TMap<FString, FString>& Parameters)
	{
		FString Result;
		for (const auto& Pair : Parameters)
		{
			Result += FString::Printf(TEXT("%s=%s "), *Pair.Key, *Pair.Value);
		}
		return Result;
	}

	FGorgeousInsightTestResult ConvertScenarioResult(const FGorgeousInsightScenarioResult& Result)
	{
		FGorgeousInsightTestResult OutResult;
		OutResult.bSuccess = Result.bSuccess;
		OutResult.Errors = Result.Errors;
		OutResult.Warnings = Result.Warnings;
		OutResult.Notes = Result.Notes;
		OutResult.Metrics = Result.Metrics;
		OutResult.LogCapture = Result.LogCapture;
		return OutResult;
	}
}

FName FGorgeousCoreInsightMatrixProvider::ProviderName()
{
	return FName(TEXT("GorgeousCore"));
}

FText FGorgeousCoreInsightMatrixProvider::GetProviderDisplayName() const
{
	return FText::FromString(TEXT("Gorgeous Core"));
}

void FGorgeousCoreInsightMatrixProvider::GatherStats(const FGorgeousInsightGatherContext& Context, TArray<FGorgeousInsightStat>& OutStats) const
{
	// Purged. Will pull published stats instead.
	UGorgeousInsightStatBuilder::GatherPublishedStats(ProviderName(), OutStats);

	// --- 1.2 SIGNAL BRIDGE STATS ---
	FGorgeousInsightStat DispatchedStat;
	DispatchedStat.Id = TEXT("Core.SignalBridge.Dispatched");
	DispatchedStat.DisplayName = FText::FromString(TEXT("Signals Dispatched"));
	DispatchedStat.Description = FText::FromString(TEXT("Total local signals fired since boot."));
	DispatchedStat.Category = FName(TEXT("Signal Bridge"));
	DispatchedStat.NumericValue = (USignalBridgeStorage_OV::GetTotalLocalSignalsFired());
	OutStats.Add(DispatchedStat);

	FGorgeousInsightStat DroppedStat;
	DroppedStat.Id = TEXT("Core.SignalBridge.Dropped");
	DroppedStat.DisplayName = FText::FromString(TEXT("Dropped Signals"));
	DroppedStat.Description = FText::FromString(TEXT("Signals fired with zero active listeners."));
	DroppedStat.Category = FName(TEXT("Signal Bridge"));
	DroppedStat.NumericValue = (USignalBridgeStorage_OV::GetTotalNoListenersFound());
	OutStats.Add(DroppedStat);

	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			if (USignalBridgeStorage_OV* Bridge = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(World, false))
			{
				FGorgeousInsightStat ListenersStat;
				ListenersStat.Id = TEXT("Core.SignalBridge.Listeners");
				ListenersStat.DisplayName = FText::FromString(TEXT("Active Tag Listeners"));
				ListenersStat.Description = FText::FromString(TEXT("Number of unique gameplay tags currently being listened to."));
				ListenersStat.Category = FName(TEXT("Signal Bridge"));
				ListenersStat.NumericValue = (Bridge->GetTotalActiveListeners());
				OutStats.Add(ListenersStat);
			}
		}
	}
}

void FGorgeousCoreInsightMatrixProvider::GatherCharts(TArray<FGorgeousInsightChartDefinition>& OutCharts) const
{
	// Purged. Will pull published charts instead.
	UGorgeousInsightStatBuilder::GatherPublishedCharts(ProviderName(), OutCharts);
}

void FGorgeousCoreInsightMatrixProvider::GetActions(TArray<FGorgeousInsightAction>& OutActions) const
{
	FGorgeousInsightAction BrowserAction;
	BrowserAction.Id = ActionOpenOVBrowser;
	BrowserAction.DisplayName = FText::FromString(TEXT("Object Variable Browser"));
	BrowserAction.Description = FText::FromString(TEXT("Opens the general-purpose Object Variable tree browser window."));
	BrowserAction.Category = FName(TEXT("Windows"));
	OutActions.Add(BrowserAction);

	// ── AutoReplication merged actions ────────────────────────────────────
	{
		FGorgeousInsightAction Action;
		Action.Id = ActionOpenTrafficInspector;
		Action.DisplayName = FText::FromString(TEXT("Network Traffic Inspector"));
		Action.Description = FText::FromString(TEXT("Opens the Network Traffic Inspector companion window."));
		Action.Category = FName(TEXT("Windows"));
		OutActions.Add(Action);
	}
	{
		FGorgeousInsightAction Action;
		Action.Id = ActionOpenRPCInspector;
		Action.DisplayName = FText::FromString(TEXT("RPC Inspector"));
		Action.Description = FText::FromString(TEXT("Opens the RPC Inspector companion window."));
		Action.Category = FName(TEXT("Windows"));
		OutActions.Add(Action);
	}
}

void FGorgeousCoreInsightMatrixProvider::ExecuteAction(FName ActionId, const FGorgeousInsightActionContext& Context)
{
	if (ActionId == ActionOpenOVBrowser)
	{
		SGorgeousObjectVariableBrowserWindow::Open();
		return;
	}

	// ── AutoReplication merged action handlers ───────────────────────────
	if (ActionId == ActionOpenTrafficInspector)
	{
		SGorgeousNetworkTrafficInspectorWindow::Open();
		return;
	}

	if (ActionId == ActionOpenRPCInspector)
	{
		SGorgeousRPCInspectorWindow::Open();
		return;
	}
}

void FGorgeousCoreInsightMatrixProvider::GetTests(TArray<FGorgeousInsightTest>& OutTests) const
{
	const TArray<FGorgeousInsightScenarioDescriptor> Scenarios = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
	for (const FGorgeousInsightScenarioDescriptor& Descriptor : Scenarios)
	{
		// Include both Core-tagged and AR-tagged scenarios
		if (!IsCoreScenario(Descriptor) && !IsARScenario(Descriptor))
		{
			continue;
		}

		FGorgeousInsightTest Test;
		Test.Id = Descriptor.ScenarioName;
		Test.DisplayName = FText::FromString(Descriptor.GetDisplayName());
		Test.Description = FText::FromString(Descriptor.Description);
		if (IsARScenario(Descriptor)) {
			Test.Category = FName(TEXT("AutoReplication"));
		} else if (Descriptor.Tags.Contains(TEXT("signal-bridge"))) {
			Test.Category = FName(TEXT("Signal Bridge"));
		} else {
			Test.Category = FName(TEXT("Tests"));
		}
		Test.Tags = Descriptor.Tags;
		Test.Inputs = Descriptor.Inputs;
		OutTests.Add(Test);
	}
}

FGorgeousInsightTestResult FGorgeousCoreInsightMatrixProvider::RunTest(const FName TestId, const FGorgeousInsightTestContext& Context)
{
	UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get();
	if (!Subsystem)
	{
		FGorgeousInsightTestResult Result;
		Result.bSuccess = false;
		Result.Errors.Add(TEXT("Insight Matrix subsystem unavailable"));
		return Result;
	}

	const FString Parameters = BuildParameterString(Context.Parameters);
	const FGorgeousInsightScenarioResult ScenarioResult = Subsystem->RunScenarioByName(TestId, Parameters, /*VariantIndex*/ 0, Context.WorldContextObject);
	return ConvertScenarioResult(ScenarioResult);
}
