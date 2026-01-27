// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#include "InsightMatrix/GorgeousCoreInsightMatrixProvider.h"

#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

namespace
{
	const FName ActionHarnessStart(TEXT("Core.Harness.Start"));
	const FName ActionHarnessStop(TEXT("Core.Harness.Stop"));
	const FName ActionHarnessDump(TEXT("Core.Harness.Dump"));

	FString BuildParameterString(const TMap<FString, FString>& Parameters)
	{
		if (Parameters.Num() == 0)
		{
			return FString();
		}

		TArray<FString> Segments;
		Segments.Reserve(Parameters.Num());
		for (const TPair<FString, FString>& Pair : Parameters)
		{
			Segments.Add(FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value));
		}
		return FString::Join(Segments, TEXT(" "));
	}

	void AddStat(TArray<FGorgeousInsightStat>& OutStats, const FName Id, const FString& Label, const FName Category,
		EGorgeousInsightStatValueType ValueType, double NumericValue, const FString& Unit = FString())
	{
		FGorgeousInsightStat Stat;
		Stat.Id = Id;
		Stat.DisplayName = FText::FromString(Label);
		Stat.Category = Category;
		Stat.ValueType = ValueType;
		Stat.NumericValue = NumericValue;
		Stat.Unit = FText::FromString(Unit);
		OutStats.Add(Stat);
	}

	void AddTextStat(TArray<FGorgeousInsightStat>& OutStats, const FName Id, const FString& Label, const FName Category, const FString& Value)
	{
		FGorgeousInsightStat Stat;
		Stat.Id = Id;
		Stat.DisplayName = FText::FromString(Label);
		Stat.Category = Category;
		Stat.ValueType = EGorgeousInsightStatValueType::Text;
		Stat.TextValue = FText::FromString(Value);
		OutStats.Add(Stat);
	}

	bool HasForeignProviderTag(const FGorgeousInsightScenarioDescriptor& Descriptor, const FName ProviderName)
	{
		if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
		{
			for (IGorgeousInsightMatrixProvider* Provider : Subsystem->GetProviders())
			{
				if (!Provider)
				{
					continue;
				}
				const FName OtherName = Provider->GetProviderName();
				if (OtherName != ProviderName && Descriptor.Tags.Contains(OtherName))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IsCoreScenario(const FGorgeousInsightScenarioDescriptor& Descriptor)
	{
		const FName ProviderName = FGorgeousCoreInsightMatrixProvider::ProviderName();
		if (Descriptor.Tags.Contains(ProviderName))
		{
			return true;
		}
		return !HasForeignProviderTag(Descriptor, ProviderName);
	}

	FGorgeousInsightTestResult ConvertScenarioResult(const FGorgeousInsightScenarioResult& ScenarioResult)
	{
		FGorgeousInsightTestResult Result;
		Result.bSuccess = ScenarioResult.bSuccess;
		Result.Errors = ScenarioResult.Errors;
		Result.Warnings = ScenarioResult.Warnings;
		Result.Notes = ScenarioResult.Notes;
		Result.Metrics = ScenarioResult.Metrics;
		return Result;
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

void FGorgeousCoreInsightMatrixProvider::GatherStats(TArray<FGorgeousInsightStat>& OutStats) const
{
	const FGorgeousInsightHarnessStatus Status = FGorgeousInsightHarness::GetStatus();
	const FName Category(TEXT("Harness"));

	AddStat(OutStats, FName(TEXT("Core.Harness.Active")), TEXT("Harness Active"), Category,
		EGorgeousInsightStatValueType::Number, Status.bHarnessActive ? 1.0 : 0.0);
	AddStat(OutStats, FName(TEXT("Core.Harness.Gauntlet")), TEXT("Gauntlet Active"), Category,
		EGorgeousInsightStatValueType::Number, Status.bUsingGauntlet ? 1.0 : 0.0);
	AddTextStat(OutStats, FName(TEXT("Core.Gauntlet.State")), TEXT("Gauntlet State"), Category, Status.GauntletState.ToString());
	AddStat(OutStats, FName(TEXT("Core.Gauntlet.StateSeconds")), TEXT("Gauntlet State Seconds"), Category,
		EGorgeousInsightStatValueType::TimeSeconds, Status.GauntletStateSeconds, TEXT("s"));
	AddStat(OutStats, FName(TEXT("Core.Harness.Connection")), TEXT("Has Connection"), Category,
		EGorgeousInsightStatValueType::Number, Status.bHasConnection ? 1.0 : 0.0);
	AddStat(OutStats, FName(TEXT("Core.Harness.ServerPort")), TEXT("Server Port"), Category,
		EGorgeousInsightStatValueType::Number, static_cast<double>(Status.ServerPort));

	AddTextStat(OutStats, FName(TEXT("Core.Harness.Map")), TEXT("Harness Map"), Category, Status.HarnessMapPath);
	AddTextStat(OutStats, FName(TEXT("Core.Harness.Address")), TEXT("Listen Address"), Category, Status.ListenAddress);

	AddStat(OutStats, FName(TEXT("Core.Harness.OutBytes")), TEXT("Outgoing Bytes"), Category,
		EGorgeousInsightStatValueType::Bytes, Status.Stats.OutgoingBytes, TEXT("B"));
	AddStat(OutStats, FName(TEXT("Core.Harness.InBytes")), TEXT("Incoming Bytes"), Category,
		EGorgeousInsightStatValueType::Bytes, Status.Stats.IncomingBytes, TEXT("B"));
	AddStat(OutStats, FName(TEXT("Core.Harness.ServerOutRate")), TEXT("Server Out Rate"), Category,
		EGorgeousInsightStatValueType::Bytes, Status.Stats.ServerOutRateBytesPerSecond, TEXT("B/s"));
	AddStat(OutStats, FName(TEXT("Core.Harness.ServerInRate")), TEXT("Server In Rate"), Category,
		EGorgeousInsightStatValueType::Bytes, Status.Stats.ServerInRateBytesPerSecond, TEXT("B/s"));
	AddStat(OutStats, FName(TEXT("Core.Harness.ClientOutRate")), TEXT("Client Out Rate"), Category,
		EGorgeousInsightStatValueType::Bytes, Status.Stats.ClientOutRateBytesPerSecond, TEXT("B/s"));
	AddStat(OutStats, FName(TEXT("Core.Harness.ClientInRate")), TEXT("Client In Rate"), Category,
		EGorgeousInsightStatValueType::Bytes, Status.Stats.ClientInRateBytesPerSecond, TEXT("B/s"));

	if (Status.NetworkMetricLines.Num() > 0)
	{
		int32 Index = 0;
		for (const FString& Line : Status.NetworkMetricLines)
		{
			AddTextStat(OutStats, FName(*FString::Printf(TEXT("Core.Harness.Metric.%d"), Index++)), TEXT("Network Metric"), Category, Line);
		}
	}

	const UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get();
	if (Subsystem)
	{
		const UGorgeousInsightMatrixSubsystem::FGorgeousInsightLastRunStats& LastRun = Subsystem->GetLastRunStats();
		const FName LastRunCategory(TEXT("LastRun"));
		if (LastRun.bHasRun)
		{
			AddTextStat(OutStats, FName(TEXT("Core.LastRun.Provider")), TEXT("Last Run Provider"), LastRunCategory, LastRun.ProviderName.ToString());
			AddTextStat(OutStats, FName(TEXT("Core.LastRun.Test")), TEXT("Last Run Test"), LastRunCategory, LastRun.TestId.ToString());
			AddTextStat(OutStats, FName(TEXT("Core.LastRun.Timestamp")), TEXT("Last Run Timestamp"), LastRunCategory, LastRun.Timestamp.ToString());
			AddStat(OutStats, FName(TEXT("Core.LastRun.Duration")), TEXT("Last Run Duration"), LastRunCategory,
				EGorgeousInsightStatValueType::TimeSeconds, LastRun.DurationSeconds, TEXT("s"));
			AddStat(OutStats, FName(TEXT("Core.LastRun.Errors")), TEXT("Last Run Errors"), LastRunCategory,
				EGorgeousInsightStatValueType::Number, static_cast<double>(LastRun.ErrorCount));
			AddStat(OutStats, FName(TEXT("Core.LastRun.Warnings")), TEXT("Last Run Warnings"), LastRunCategory,
				EGorgeousInsightStatValueType::Number, static_cast<double>(LastRun.WarningCount));
			AddStat(OutStats, FName(TEXT("Core.LastRun.Notes")), TEXT("Last Run Notes"), LastRunCategory,
				EGorgeousInsightStatValueType::Number, static_cast<double>(LastRun.NoteCount));
			AddStat(OutStats, FName(TEXT("Core.LastRun.OVOverhead")), TEXT("OV Overhead"), LastRunCategory,
				EGorgeousInsightStatValueType::Number, LastRun.ObjectVariableOverheadMs, TEXT("ms"));
			AddStat(OutStats, FName(TEXT("Core.LastRun.OVRegistered")), TEXT("Object Variables Registered (Last Run)"), LastRunCategory,
				EGorgeousInsightStatValueType::Number, static_cast<double>(LastRun.ObjectVariablesRegistered));
		}
		else
		{
			AddTextStat(OutStats, FName(TEXT("Core.LastRun.Timestamp")), TEXT("Last Run Timestamp"), LastRunCategory, TEXT("Never"));
			AddStat(OutStats, FName(TEXT("Core.LastRun.OVOverhead")), TEXT("OV Overhead"), LastRunCategory,
				EGorgeousInsightStatValueType::Number, 0.0, TEXT("ms"));
			AddStat(OutStats, FName(TEXT("Core.LastRun.OVRegistered")), TEXT("Object Variables Registered (Last Run)"), LastRunCategory,
				EGorgeousInsightStatValueType::Number, 0.0);
		}
	}

	{
		const FName OVCategory(TEXT("ObjectVariables"));
		const TArray<FName> RootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
		const TArray<UGorgeousObjectVariable*> AllVariables = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(NAME_None);
		const TArray<UGorgeousObjectVariable*> RootVariables = UGorgeousRootObjectVariable::GetRootVariableRegistry(NAME_None);
		int32 SharedRoots = 0;
		int32 EnforcedRoots = 0;
		for (const FName RootName : RootNames)
		{
			SharedRoots += UGorgeousRootObjectVariable::IsSharedNetworkingRoot(RootName) ? 1 : 0;
			EnforcedRoots += UGorgeousRootObjectVariable::IsEnforcedNetworkingRoot(RootName) ? 1 : 0;
		}

		AddStat(OutStats, FName(TEXT("Core.OV.RootCount")), TEXT("OV Root Count"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(RootNames.Num()));
		AddStat(OutStats, FName(TEXT("Core.OV.RootVariableCount")), TEXT("OV Root Variables"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(RootVariables.Num()));
		AddStat(OutStats, FName(TEXT("Core.OV.TotalVariables")), TEXT("OV Total Variables"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(AllVariables.Num()));
		AddStat(OutStats, FName(TEXT("Core.OV.SharedRoots")), TEXT("OV Shared Roots"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(SharedRoots));
		AddStat(OutStats, FName(TEXT("Core.OV.EnforcedRoots")), TEXT("OV Enforced Roots"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(EnforcedRoots));
	}
}

void FGorgeousCoreInsightMatrixProvider::GatherCharts(TArray<FGorgeousInsightChartDefinition>& OutCharts) const
{
	const FGorgeousInsightHarnessStatus Status = FGorgeousInsightHarness::GetStatus();

	TArray<FGorgeousInsightBarValue> RateBars;
	RateBars.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "ServerOut", "Server Out"), Status.Stats.ServerOutRateBytesPerSecond });
	RateBars.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "ServerIn", "Server In"), Status.Stats.ServerInRateBytesPerSecond });
	RateBars.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "ClientOut", "Client Out"), Status.Stats.ClientOutRateBytesPerSecond });
	RateBars.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "ClientIn", "Client In"), Status.Stats.ClientInRateBytesPerSecond });
	OutCharts.Add(FGorgeousInsightChartDefinition::MakeBarChart(
		FName(TEXT("Core.Harness.Rates")),
		NSLOCTEXT("GorgeousInsightCoreCharts", "HarnessRatesTitle", "Harness Throughput"),
		NSLOCTEXT("GorgeousInsightCoreCharts", "HarnessRatesSubtitle", "Bytes per second"),
		RateBars));

	{
		const TArray<FName> RootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
		int32 SharedRoots = 0;
		int32 EnforcedRoots = 0;
		for (const FName RootName : RootNames)
		{
			SharedRoots += UGorgeousRootObjectVariable::IsSharedNetworkingRoot(RootName) ? 1 : 0;
			EnforcedRoots += UGorgeousRootObjectVariable::IsEnforcedNetworkingRoot(RootName) ? 1 : 0;
		}
		const int32 OtherRoots = FMath::Max(0, RootNames.Num() - SharedRoots - EnforcedRoots);
		TArray<FGorgeousInsightPieSlice> RootSlices;
		RootSlices.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "SharedRoots", "Shared"), static_cast<double>(SharedRoots), FLinearColor(0.35f, 0.85f, 0.45f, 1.f) });
		RootSlices.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "EnforcedRoots", "Enforced"), static_cast<double>(EnforcedRoots), FLinearColor(0.9f, 0.55f, 0.25f, 1.f) });
		RootSlices.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "OtherRoots", "Other"), static_cast<double>(OtherRoots), FLinearColor(0.4f, 0.6f, 1.f, 1.f) });
		OutCharts.Add(FGorgeousInsightChartDefinition::MakePieChart(
			FName(TEXT("Core.OV.Roots")),
			NSLOCTEXT("GorgeousInsightCoreCharts", "OVRootsTitle", "OV Roots"),
			NSLOCTEXT("GorgeousInsightCoreCharts", "OVRootsSubtitle", "Shared vs enforced"),
			RootSlices,
			true));
	}

	if (const UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		const UGorgeousInsightMatrixSubsystem::FGorgeousInsightLastRunStats& LastRun = Subsystem->GetLastRunStats();
		TArray<FGorgeousInsightTableRow> Rows;
		Rows.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunProvider", "Provider"), FText::FromName(LastRun.ProviderName), NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunCategory", "Last Run") });
		Rows.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunTest", "Test"), FText::FromName(LastRun.TestId), NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunCategory", "Last Run") });
		Rows.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunDuration", "Duration (s)"), FText::AsNumber(LastRun.DurationSeconds, &FNumberFormattingOptions().SetMaximumFractionalDigits(2)), NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunCategory", "Last Run") });
		Rows.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunErrors", "Errors"), FText::AsNumber(LastRun.ErrorCount), NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunCategory", "Last Run") });
		Rows.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunWarnings", "Warnings"), FText::AsNumber(LastRun.WarningCount), NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunCategory", "Last Run") });
		OutCharts.Add(FGorgeousInsightChartDefinition::MakeTableChart(
			FName(TEXT("Core.LastRun.Table")),
			NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunTitle", "Last Run Summary"),
			NSLOCTEXT("GorgeousInsightCoreCharts", "LastRunSubtitle", "Most recent test run"),
			Rows));
	}
}

void FGorgeousCoreInsightMatrixProvider::GetActions(TArray<FGorgeousInsightAction>& OutActions) const
{
	FGorgeousInsightAction StartAction;
	StartAction.Id = ActionHarnessStart;
	StartAction.DisplayName = FText::FromString(TEXT("Start Harness"));
	StartAction.Description = FText::FromString(TEXT("Starts the server/client harness (optional parameters supported)."));
	StartAction.Category = FName(TEXT("Harness"));
	OutActions.Add(StartAction);

	FGorgeousInsightAction StopAction;
	StopAction.Id = ActionHarnessStop;
	StopAction.DisplayName = FText::FromString(TEXT("Stop Harness"));
	StopAction.Description = FText::FromString(TEXT("Stops the server/client harness."));
	StopAction.Category = FName(TEXT("Harness"));
	OutActions.Add(StopAction);

	FGorgeousInsightAction DumpAction;
	DumpAction.Id = ActionHarnessDump;
	DumpAction.DisplayName = FText::FromString(TEXT("Dump Harness Status"));
	DumpAction.Description = FText::FromString(TEXT("Logs current harness status to output log."));
	DumpAction.Category = FName(TEXT("Harness"));
	OutActions.Add(DumpAction);
}

void FGorgeousCoreInsightMatrixProvider::ExecuteAction(FName ActionId, const FGorgeousInsightActionContext& Context)
{
	if (ActionId == ActionHarnessStart)
	{
		const FString Parameters = BuildParameterString(Context.Parameters);
		const FGorgeousInsightMatrixRequest Request = FGorgeousInsightMatrixRequest::FromParameters(Parameters);
		const bool bStarted = FGorgeousInsightHarness::StartHarness(Request);
		GT_I_LOG("GT.InsightMatrix.Harness.ActionStart", TEXT("[InsightHarness] Start requested (success=%s)"), bStarted ? TEXT("true") : TEXT("false"));
		return;
	}

	if (ActionId == ActionHarnessStop)
	{
		FGorgeousInsightHarness::StopHarness();
		return;
	}

	if (ActionId == ActionHarnessDump)
	{
		const FGorgeousInsightHarnessStatus Status = FGorgeousInsightHarness::GetStatus();
		GT_I_LOG("GT.InsightMatrix.Harness.Status", TEXT("[InsightHarness] Active=%s Gauntlet=%s Conn=%s Map=%s Addr=%s Port=%d"),
			Status.bHarnessActive ? TEXT("true") : TEXT("false"),
			Status.bUsingGauntlet ? TEXT("true") : TEXT("false"),
			Status.bHasConnection ? TEXT("true") : TEXT("false"),
			*Status.HarnessMapPath,
			*Status.ListenAddress,
			Status.ServerPort);
		return;
	}
}

void FGorgeousCoreInsightMatrixProvider::GetTests(TArray<FGorgeousInsightTest>& OutTests) const
{
	const TArray<FGorgeousInsightScenarioDescriptor> Scenarios = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
	for (const FGorgeousInsightScenarioDescriptor& Descriptor : Scenarios)
	{
		if (!IsCoreScenario(Descriptor))
		{
			continue;
		}

		FGorgeousInsightTest Test;
		Test.Id = Descriptor.ScenarioName;
		Test.DisplayName = FText::FromString(Descriptor.GetDisplayName());
		Test.Description = FText::FromString(Descriptor.Description);
		Test.Category = FName(TEXT("Tests"));
		Test.Tags = Descriptor.Tags;
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
