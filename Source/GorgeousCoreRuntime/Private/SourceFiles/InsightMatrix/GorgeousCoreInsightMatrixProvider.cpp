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
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/Slate/GorgeousObjectVariableBrowserWindow.h"
#include "InsightMatrix/Slate/SGorgeousNetworkTrafficInspectorWindow.h"
#include "InsightMatrix/Slate/SGorgeousRPCInspectorWindow.h"
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

	// ── AutoReplication merged actions ────────────────────────────────────
	const FName ActionOpenTrafficInspector(TEXT("AR.Window.TrafficInspector"));
	const FName ActionOpenRPCInspector(TEXT("AR.Window.RPCInspector"));

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

	bool IsARScenario(const FGorgeousInsightScenarioDescriptor& Descriptor)
	{
		return Descriptor.Tags.Contains(FName(TEXT("AutoReplication")))
			|| Descriptor.Tags.Contains(FName(TEXT("AR")))
			|| Descriptor.Tags.Contains(FName(TEXT("Networking")));
	}

	UWorld* FindActiveGameWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				if (UWorld* World = Context.World())
				{
					return World;
				}
			}
		}
		return nullptr;
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

		// Per-root variable counts
		for (const FName RootName : RootNames)
		{
			const TArray<UGorgeousObjectVariable*> RootVars = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(RootName);
			AddStat(OutStats, FName(*FString::Printf(TEXT("Core.OV.Root.%s.Count"), *RootName.ToString())),
				*FString::Printf(TEXT("OV [%s] Variable Count"), *RootName.ToString()), OVCategory,
				EGorgeousInsightStatValueType::Number, static_cast<double>(RootVars.Num()));
		}

		// Networking breakdown across all variables
		int32 ReplicatingCount = 0;
		int32 NetworkCapableCount = 0;
		int32 MaxDepth = 0;
		TMap<FName, int32> ClassDistribution;
		for (const UGorgeousObjectVariable* Variable : AllVariables)
		{
			if (!IsValid(Variable))
			{
				continue;
			}

			if (Variable->IsReplicationActive())
			{
				++ReplicatingCount;
			}
			if (static_cast<const UObject*>(Variable)->IsSupportedForNetworking())
			{
				++NetworkCapableCount;
			}

			// Class distribution
			const FName ClassName = Variable->GetClass()->GetFName();
			ClassDistribution.FindOrAdd(ClassName, 0)++;

			// Hierarchy depth
			int32 Depth = 0;
			const UGorgeousObjectVariable* Walker = Variable;
			while (Walker->GetParent() != nullptr)
			{
				Walker = Walker->GetParent();
				++Depth;
			}
			MaxDepth = FMath::Max(MaxDepth, Depth);
		}

		AddStat(OutStats, FName(TEXT("Core.OV.Replicating")), TEXT("OV Replicating"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(ReplicatingCount));
		AddStat(OutStats, FName(TEXT("Core.OV.NetworkCapable")), TEXT("OV Network Capable"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(NetworkCapableCount));
		AddStat(OutStats, FName(TEXT("Core.OV.MaxDepth")), TEXT("OV Max Hierarchy Depth"), OVCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(MaxDepth));

		// Top class types
		for (const TPair<FName, int32>& ClassEntry : ClassDistribution)
		{
			AddStat(OutStats, FName(*FString::Printf(TEXT("Core.OV.Class.%s"), *ClassEntry.Key.ToString())),
				*FString::Printf(TEXT("OV Class [%s]"), *ClassEntry.Key.ToString()), OVCategory,
				EGorgeousInsightStatValueType::Number, static_cast<double>(ClassEntry.Value));
		}
	}

	{
		const FName SignalCategory(TEXT("SignalBridge"));
		AddStat(OutStats, FName(TEXT("Core.SignalBridge.LocalFired")), TEXT("Local Signals Fired"), SignalCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(USignalBridgeStorage_OV::GetTotalLocalSignalsFired()));
		AddStat(OutStats, FName(TEXT("Core.SignalBridge.NoListeners")), TEXT("Local Signals Missed (No Listeners)"), SignalCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(USignalBridgeStorage_OV::GetTotalNoListenersFound()));
	}

	// Scenario registry stats
	{
		const FName ScenarioCategory(TEXT("Scenarios"));
		const TArray<FGorgeousInsightScenarioDescriptor> Scenarios = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
		int32 LocalTests = 0;
		int32 GauntletTests = 0;
		int32 CoreTests = 0;
		for (const FGorgeousInsightScenarioDescriptor& Descriptor : Scenarios)
		{
			if (Descriptor.Tags.Contains(FName(TEXT("gauntlet"))))
			{
				++GauntletTests;
			}
			else
			{
				++LocalTests;
			}
			if (IsCoreScenario(Descriptor))
			{
				++CoreTests;
			}
		}

		AddStat(OutStats, FName(TEXT("Core.Scenarios.Total")), TEXT("Registered Scenarios"), ScenarioCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(Scenarios.Num()));
		AddStat(OutStats, FName(TEXT("Core.Scenarios.Local")), TEXT("Local Scenarios"), ScenarioCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(LocalTests));
		AddStat(OutStats, FName(TEXT("Core.Scenarios.Gauntlet")), TEXT("Gauntlet Scenarios"), ScenarioCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(GauntletTests));
		AddStat(OutStats, FName(TEXT("Core.Scenarios.Core")), TEXT("Core Provider Scenarios"), ScenarioCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(CoreTests));
	}

	// ════════════════════════════════════════════════════════════════════════
	//  AutoReplication stats (merged from AR provider)
	// ════════════════════════════════════════════════════════════════════════
	{
		const FName ARCategory(TEXT("AutoReplication"));
		const TArray<UGorgeousObjectVariable*> AllVars = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(NAME_None);

		int32 ARReplicating = 0, ARNetCapable = 0, ARWithBinding = 0, ARShared = 0;
		int32 ModeAuto = 0, ModeHybrid = 0, ModeManual = 0;
		int32 BackendNative = 0, BackendIris = 0, BackendRepGraph = 0;
		double BandwidthTotal = 0.0;
		float FreqMin = FLT_MAX, FreqMax = 0.f;
		double FreqSum = 0.0;
		int32 FreqCount = 0;

		for (const UGorgeousObjectVariable* Var : AllVars)
		{
			if (!IsValid(Var))
			{
				continue;
			}
			if (Var->bSupportsNetworking)
			{
				++ARNetCapable;
			}
			if (Var->IsReplicationActive())
			{
				++ARReplicating;
			}
			if (Var->HasAutoReplicationBinding())
			{
				++ARWithBinding;
			}
			if (Var->bUseSharedNetworkStack)
			{
				++ARShared;
			}

			switch (Var->ReplicationMode)
			{
			case EGorgeousObjectVariableReplicationMode::EFullAutoReplication: ++ModeAuto; break;
			case EGorgeousObjectVariableReplicationMode::EHybrid:             ++ModeHybrid; break;
			case EGorgeousObjectVariableReplicationMode::EManual:             ++ModeManual; break;
			default: break;
			}

			if (Var->bSupportsNetworking)
			{
				switch (Var->AutoReplicationConfig.Backend)
				{
				case EGorgeousAutoReplicationBackend::Native:           ++BackendNative; break;
				case EGorgeousAutoReplicationBackend::Iris:             ++BackendIris; break;
				case EGorgeousAutoReplicationBackend::ReplicationGraph: ++BackendRepGraph; break;
				default: break;
				}

				if (Var->IsReplicationActive())
				{
					BandwidthTotal += Var->AutoReplicationConfig.BandwidthBudgetKB;
					const float Freq = Var->AutoReplicationConfig.GetEffectiveUpdateFrequency();
					FreqMin = FMath::Min(FreqMin, Freq);
					FreqMax = FMath::Max(FreqMax, Freq);
					FreqSum += Freq;
					++FreqCount;
				}
			}
		}

		if (FreqMin == FLT_MAX)
		{
			FreqMin = 0.f;
		}

		AddStat(OutStats, FName(TEXT("Core.AR.Replicating")), TEXT("AR Replicating"), ARCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(ARReplicating));
		AddStat(OutStats, FName(TEXT("Core.AR.NetCapable")), TEXT("AR Net Capable"), ARCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(ARNetCapable));
		AddStat(OutStats, FName(TEXT("Core.AR.WithBinding")), TEXT("AR With Binding"), ARCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(ARWithBinding));
		AddStat(OutStats, FName(TEXT("Core.AR.SharedStack")), TEXT("AR Shared Net Stack"), ARCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(ARShared));

		AddTextStat(OutStats, FName(TEXT("Core.AR.Mode")), TEXT("AR Dominant Mode"), ARCategory,
			(ModeAuto >= ModeHybrid && ModeAuto >= ModeManual) ? TEXT("Full Auto")
			: (ModeHybrid >= ModeManual) ? TEXT("Hybrid") : TEXT("Manual"));

		AddTextStat(OutStats, FName(TEXT("Core.AR.Backend")), TEXT("AR Dominant Backend"), ARCategory,
			(BackendNative >= BackendIris && BackendNative >= BackendRepGraph) ? TEXT("Native")
			: (BackendIris >= BackendRepGraph) ? TEXT("Iris") : TEXT("RepGraph"));

		AddStat(OutStats, FName(TEXT("Core.AR.Bandwidth")), TEXT("AR Total Bandwidth Budget"), ARCategory,
			EGorgeousInsightStatValueType::Number, BandwidthTotal, TEXT("KB"));

		AddStat(OutStats, FName(TEXT("Core.AR.FreqMin")), TEXT("AR Min Frequency"), ARCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(FreqMin), TEXT("Hz"));
		AddStat(OutStats, FName(TEXT("Core.AR.FreqMax")), TEXT("AR Max Frequency"), ARCategory,
			EGorgeousInsightStatValueType::Number, static_cast<double>(FreqMax), TEXT("Hz"));
		AddStat(OutStats, FName(TEXT("Core.AR.FreqAvg")), TEXT("AR Avg Frequency"), ARCategory,
			EGorgeousInsightStatValueType::Number, FreqCount > 0 ? FreqSum / FreqCount : 0.0, TEXT("Hz"));

		// Root Network Stack stats
		if (UWorld* World = FindActiveGameWorld())
		{
			if (UGorgeousRootNetworkStackSubsystem* StackSub = World->GetSubsystem<UGorgeousRootNetworkStackSubsystem>())
			{
				int32 ExposedCount = 0;
				int32 ChannelCount = 0;
				int32 PolicyEveryoneCount = 0;
				int32 PolicyOwnerCount = 0;
				int32 PolicyCustomCount = 0;

				for (const UGorgeousObjectVariable* Var : AllVars)
				{
					if (!IsValid(Var) || !Var->bSupportsNetworking)
					{
						continue;
					}
					if (Var->RootNetworkConfig.bExposeThroughRootNetworkStack)
					{
						++ExposedCount;
						if (!Var->RootNetworkConfig.ReplicationChannel.IsNone())
						{
							++ChannelCount;
						}
						switch (Var->RootNetworkConfig.AccessPolicy)
						{
						case EGorgeousObjectVariableAccessPolicy::Everyone:              ++PolicyEveryoneCount; break;
						case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly: ++PolicyOwnerCount; break;
						case EGorgeousObjectVariableAccessPolicy::Custom:               ++PolicyCustomCount; break;
						default: break;
						}
					}
				}

				AddStat(OutStats, FName(TEXT("Core.AR.Stack.Exposed")), TEXT("AR Stack Exposed"), ARCategory,
					EGorgeousInsightStatValueType::Number, static_cast<double>(ExposedCount));
				AddStat(OutStats, FName(TEXT("Core.AR.Stack.Channels")), TEXT("AR Stack Channels"), ARCategory,
					EGorgeousInsightStatValueType::Number, static_cast<double>(ChannelCount));
			}
		}

		// Connection info
		if (UWorld* World = FindActiveGameWorld())
		{
			const ENetMode NetMode = World->GetNetMode();
			const TCHAR* NetModeStr = TEXT("Standalone");
			switch (NetMode)
			{
			case NM_DedicatedServer: NetModeStr = TEXT("Dedicated Server"); break;
			case NM_ListenServer:    NetModeStr = TEXT("Listen Server"); break;
			case NM_Client:          NetModeStr = TEXT("Client"); break;
			default: break;
			}
			AddTextStat(OutStats, FName(TEXT("Core.AR.NetMode")), TEXT("AR Net Mode"), ARCategory, NetModeStr);

			int32 ControllerCount = 0;
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				++ControllerCount;
			}
			AddStat(OutStats, FName(TEXT("Core.AR.Controllers")), TEXT("AR Player Controllers"), ARCategory,
				EGorgeousInsightStatValueType::Number, static_cast<double>(ControllerCount));
		}
	}

	// TODO [Insight Matrix]: Feature 4 - Smart Debug HUD Overlay integration.
	// This marker notes the future integration of screen-space debug information 
	// from the Gorgeous Debug Assist system into the Insight Matrix overview.
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

	// Per-root variable count bar chart
	{
		const TArray<FName> RootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
		if (RootNames.Num() > 0)
		{
			TArray<FGorgeousInsightBarValue> RootBars;
			for (const FName RootName : RootNames)
			{
				const TArray<UGorgeousObjectVariable*> RootVars = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(RootName);
				RootBars.Add({ FText::FromName(RootName), static_cast<double>(RootVars.Num()) });
			}
			OutCharts.Add(FGorgeousInsightChartDefinition::MakeBarChart(
				FName(TEXT("Core.OV.PerRootCount")),
				NSLOCTEXT("GorgeousInsightCoreCharts", "PerRootTitle", "Variables per Root"),
				NSLOCTEXT("GorgeousInsightCoreCharts", "PerRootSubtitle", "Total OV count by root name"),
				RootBars));
		}
	}

	// Networking status pie chart
	{
		const TArray<UGorgeousObjectVariable*> AllVariables = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(NAME_None);
		int32 Replicating = 0;
		int32 NetworkCapable = 0;
		int32 LocalOnly = 0;
		for (const UGorgeousObjectVariable* Variable : AllVariables)
		{
			if (!IsValid(Variable))
			{
				continue;
			}
			if (Variable->IsReplicationActive())
			{
				++Replicating;
			}
			else if (static_cast<const UObject*>(Variable)->IsSupportedForNetworking())
			{
				++NetworkCapable;
			}
			else
			{
				++LocalOnly;
			}
		}
		TArray<FGorgeousInsightPieSlice> NetSlices;
		NetSlices.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "Replicating", "Replicating"), static_cast<double>(Replicating), FLinearColor(0.3f, 0.9f, 0.4f, 1.f) });
		NetSlices.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "NetCapable", "Net Capable"), static_cast<double>(NetworkCapable), FLinearColor(0.9f, 0.75f, 0.2f, 1.f) });
		NetSlices.Add({ NSLOCTEXT("GorgeousInsightCoreCharts", "LocalOnly", "Local Only"), static_cast<double>(LocalOnly), FLinearColor(0.5f, 0.5f, 0.5f, 1.f) });
		OutCharts.Add(FGorgeousInsightChartDefinition::MakePieChart(
			FName(TEXT("Core.OV.NetStatus")),
			NSLOCTEXT("GorgeousInsightCoreCharts", "NetStatusTitle", "OV Network Status"),
			NSLOCTEXT("GorgeousInsightCoreCharts", "NetStatusSubtitle", "Replication breakdown"),
			NetSlices,
			true));
	}

	// ════════════════════════════════════════════════════════════════════════
	//  AutoReplication charts (merged from AR provider)
	// ════════════════════════════════════════════════════════════════════════
	{
		const TArray<UGorgeousObjectVariable*> AllVars = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(NAME_None);

		// Replication Mode Distribution Pie
		{
			int32 ModeAuto = 0, ModeHybrid = 0, ModeManual = 0;
			for (const UGorgeousObjectVariable* Var : AllVars)
			{
				if (!IsValid(Var) || !Var->bSupportsNetworking)
				{
					continue;
				}
				switch (Var->ReplicationMode)
				{
				case EGorgeousObjectVariableReplicationMode::EFullAutoReplication: ++ModeAuto; break;
				case EGorgeousObjectVariableReplicationMode::EHybrid:             ++ModeHybrid; break;
				case EGorgeousObjectVariableReplicationMode::EManual:             ++ModeManual; break;
				default: break;
				}
			}
			TArray<FGorgeousInsightPieSlice> Slices;
			Slices.Add({ NSLOCTEXT("ARInsight", "FullAuto", "Full Auto"), static_cast<double>(ModeAuto), FLinearColor(0.3f, 0.85f, 0.45f, 1.f) });
			Slices.Add({ NSLOCTEXT("ARInsight", "Hybrid", "Hybrid"), static_cast<double>(ModeHybrid), FLinearColor(0.9f, 0.7f, 0.25f, 1.f) });
			Slices.Add({ NSLOCTEXT("ARInsight", "Manual", "Manual"), static_cast<double>(ModeManual), FLinearColor(0.85f, 0.35f, 0.35f, 1.f) });
			OutCharts.Add(FGorgeousInsightChartDefinition::MakePieChart(
				FName(TEXT("Core.AR.Chart.ReplicationMode")),
				NSLOCTEXT("ARInsight", "ModeTitle", "Replication Mode Distribution"),
				NSLOCTEXT("ARInsight", "ModeSubtitle", "Full Auto vs Hybrid vs Manual"),
				Slices, true));
		}

		// Backend Distribution Pie
		{
			int32 Native = 0, Iris = 0, RepGraph = 0;
			for (const UGorgeousObjectVariable* Var : AllVars)
			{
				if (!IsValid(Var) || !Var->bSupportsNetworking)
				{
					continue;
				}
				switch (Var->AutoReplicationConfig.Backend)
				{
				case EGorgeousAutoReplicationBackend::Native:           ++Native; break;
				case EGorgeousAutoReplicationBackend::Iris:             ++Iris; break;
				case EGorgeousAutoReplicationBackend::ReplicationGraph: ++RepGraph; break;
				default: break;
				}
			}
			TArray<FGorgeousInsightPieSlice> Slices;
			Slices.Add({ NSLOCTEXT("ARInsight", "Native", "Native UE"), static_cast<double>(Native), FLinearColor(0.3f, 0.6f, 0.95f, 1.f) });
			Slices.Add({ NSLOCTEXT("ARInsight", "Iris", "Iris"), static_cast<double>(Iris), FLinearColor(0.9f, 0.4f, 0.6f, 1.f) });
			Slices.Add({ NSLOCTEXT("ARInsight", "RepGraph", "Rep Graph"), static_cast<double>(RepGraph), FLinearColor(0.6f, 0.85f, 0.3f, 1.f) });
			OutCharts.Add(FGorgeousInsightChartDefinition::MakePieChart(
				FName(TEXT("Core.AR.Chart.Backend")),
				NSLOCTEXT("ARInsight", "BackendTitle", "Backend Distribution"),
				NSLOCTEXT("ARInsight", "BackendSubtitle", "Native vs Iris vs RepGraph"),
				Slices, true));
		}

		// Per-Root Replicating Bar Chart
		{
			const TArray<FName> RootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
			if (RootNames.Num() > 0)
			{
				TArray<FGorgeousInsightBarValue> Bars;
				for (const FName& RootName : RootNames)
				{
					const TArray<UGorgeousObjectVariable*> RootVars = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(RootName);
					int32 RepCount = 0;
					for (const UGorgeousObjectVariable* Var : RootVars)
					{
						if (IsValid(Var) && Var->IsReplicationActive())
						{
							++RepCount;
						}
					}
					Bars.Add({ FText::FromName(RootName), static_cast<double>(RepCount) });
				}
				OutCharts.Add(FGorgeousInsightChartDefinition::MakeBarChart(
					FName(TEXT("Core.AR.Chart.PerRootReplicating")),
					NSLOCTEXT("ARInsight", "PerRootTitle", "Replicating Variables per Root"),
					NSLOCTEXT("ARInsight", "PerRootSubtitle", "Active replication count by root"),
					Bars));
			}
		}

		// Bandwidth Budget Bar Chart
		{
			TArray<FGorgeousInsightBarValue> Bars;
			for (const UGorgeousObjectVariable* Var : AllVars)
			{
				if (!IsValid(Var) || !Var->bSupportsNetworking || !Var->IsReplicationActive())
				{
					continue;
				}
				const FString Label = !Var->GetDisplayName().IsEmpty() ? Var->GetDisplayName() : Var->GetName();
				Bars.Add({ FText::FromString(Label), static_cast<double>(Var->AutoReplicationConfig.BandwidthBudgetKB) });
			}
			if (Bars.Num() > 0)
			{
				Bars.Sort([](const FGorgeousInsightBarValue& A, const FGorgeousInsightBarValue& B) { return A.Value > B.Value; });
				if (Bars.Num() > 20)
				{
					Bars.SetNum(20);
				}
				OutCharts.Add(FGorgeousInsightChartDefinition::MakeBarChart(
					FName(TEXT("Core.AR.Chart.BandwidthBudget")),
					NSLOCTEXT("ARInsight", "BudgetTitle", "Bandwidth Budget (Top 20)"),
					NSLOCTEXT("ARInsight", "BudgetSubtitle", "KB per variable (replicating only)"),
					Bars));
			}
		}

		// Stream Config Table
		{
			TArray<FGorgeousInsightTableRow> Rows;
			int32 Count = 0;
			for (const UGorgeousObjectVariable* Var : AllVars)
			{
				if (!IsValid(Var) || !Var->bSupportsNetworking || !Var->IsReplicationActive())
				{
					continue;
				}
				if (++Count > 30)
				{
					break;
				}
				const FString Label = !Var->GetDisplayName().IsEmpty() ? Var->GetDisplayName() : Var->GetName();
				const FGorgeousAutoReplicationStreamConfig& Cfg = Var->AutoReplicationConfig;
				const TCHAR* BackendStr = TEXT("Native");
				switch (Cfg.Backend)
				{
				case EGorgeousAutoReplicationBackend::Iris:             BackendStr = TEXT("Iris"); break;
				case EGorgeousAutoReplicationBackend::ReplicationGraph: BackendStr = TEXT("RepGraph"); break;
				default: break;
				}
				Rows.Add({
					FText::FromString(Label),
					FText::FromString(FString::Printf(TEXT("%s | %.0fHz | %.1fKB | P%d"),
						BackendStr, Cfg.GetEffectiveUpdateFrequency(), Cfg.BandwidthBudgetKB, Cfg.Priority)),
					NSLOCTEXT("ARInsight", "StreamConfig", "Stream Config")
				});
			}
			if (Rows.Num() > 0)
			{
				OutCharts.Add(FGorgeousInsightChartDefinition::MakeTableChart(
					FName(TEXT("Core.AR.Chart.StreamTable")),
					NSLOCTEXT("ARInsight", "StreamTableTitle", "Active Stream Configuration"),
					NSLOCTEXT("ARInsight", "StreamTableSubtitle", "Backend | Freq | Budget | Priority"),
					Rows));
			}
		}

		// Access Policy Pie
		{
			int32 Everyone = 0, OwnerOnly = 0, Custom = 0;
			for (const UGorgeousObjectVariable* Var : AllVars)
			{
				if (!IsValid(Var) || !Var->bSupportsNetworking)
				{
					continue;
				}
				if (!Var->RootNetworkConfig.bExposeThroughRootNetworkStack)
				{
					continue;
				}
				switch (Var->RootNetworkConfig.AccessPolicy)
				{
				case EGorgeousObjectVariableAccessPolicy::Everyone:              ++Everyone; break;
				case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly: ++OwnerOnly; break;
				case EGorgeousObjectVariableAccessPolicy::Custom:               ++Custom; break;
				default: break;
				}
			}
			if (Everyone + OwnerOnly + Custom > 0)
			{
				TArray<FGorgeousInsightPieSlice> Slices;
				Slices.Add({ NSLOCTEXT("ARInsight", "Everyone", "Everyone"), static_cast<double>(Everyone), FLinearColor(0.3f, 0.75f, 0.5f, 1.f) });
				Slices.Add({ NSLOCTEXT("ARInsight", "OwnerOnly", "Owner Only"), static_cast<double>(OwnerOnly), FLinearColor(0.85f, 0.5f, 0.2f, 1.f) });
				Slices.Add({ NSLOCTEXT("ARInsight", "Custom", "Custom"), static_cast<double>(Custom), FLinearColor(0.6f, 0.4f, 0.85f, 1.f) });
				OutCharts.Add(FGorgeousInsightChartDefinition::MakePieChart(
					FName(TEXT("Core.AR.Chart.AccessPolicy")),
					NSLOCTEXT("ARInsight", "PolicyTitle", "Access Policy Distribution"),
					NSLOCTEXT("ARInsight", "PolicySubtitle", "Root Network Stack access policies"),
					Slices, true));
			}
		}
	}
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
		Test.Category = IsARScenario(Descriptor) ? FName(TEXT("AutoReplication")) : FName(TEXT("Tests"));
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
