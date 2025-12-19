#include "Automation/GorgeousAutomationTestMatrix.h"
#include "Automation/AutoReplicationTestMatrixCore.h"
#include "UnitTests/Harness/GorgeousServerClientHarness.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ScopeExit.h"
#include "Templates/Function.h"
#include "Engine/NetDriver.h"
#include "Engine/ReplicationDriver.h"

#if WITH_SERVER_CODE
#include "ReplicationGraph.h"
#endif

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationNetworking
{
static constexpr double GHarnessWarmupSeconds = 0.5;
static constexpr double GHarnessObservationSeconds = 1.5;

struct FNetEmulationPreset
{
	const TCHAR* Label;
	int32 PktLagMin = 0;
	int32 PktLagMax = 0;
	int32 PktLagVariance = 0;
	int32 IncomingLagMin = 0;
	int32 IncomingLagMax = 0;
	int32 PacketLossPercent = 0;
	int32 PacketDupPercent = 0;
	float ExpectedMinPingMs = 0.0f;
};

static const FNetEmulationPreset GNetEmulationPresets[] =
{
	{ TEXT("Baseline"), 0, 0, 0, 0, 0, 0, 0, 0.0f },
	{ TEXT("CafeWiFi"), 20, 60, 6, 10, 40, 1, 0, 24.0f },
	{ TEXT("4G_LightCongestion"), 45, 110, 12, 35, 90, 2, 0, 38.0f },
	{ TEXT("RuralCell"), 90, 220, 30, 70, 200, 5, 2, 72.0f },
	{ TEXT("PacketLossy"), 40, 140, 40, 25, 120, 12, 4, 42.0f },
};

inline const FNetEmulationPreset& GetNetEmulationPreset(int32 VariantIndex)
{
	const int32 PresetIndex = FMath::Clamp(VariantIndex, 0, static_cast<int32>(UE_ARRAY_COUNT(GNetEmulationPresets)) - 1);
	return GNetEmulationPresets[PresetIndex];
}

inline void AppendDriverSnapshotNotes(const FGorgeousServerClientHarness::FDriverStatSnapshot& Snapshot, FGorgeousAutomationScenarioResult& Result, const TCHAR* Prefix)
{
	if (Snapshot.Label.IsEmpty())
	{
		return;
	}

	const FString Note = Prefix
		? FString::Printf(TEXT("%s%s"), Prefix, *Snapshot.Describe())
		: Snapshot.Describe();
	Result.AddNote(Note);
}

#if WITH_SERVER_CODE
class FScopedConsoleVariableOverride
{
public:
	FScopedConsoleVariableOverride(const TCHAR* InName, int32 InValue)
		: ConsoleVariable(IConsoleManager::Get().FindConsoleVariable(InName))
	{
		if (ConsoleVariable)
		{
			PreviousValue = ConsoleVariable->GetInt();
			ConsoleVariable->Set(InValue, ECVF_SetByCode);
		}
	}

	~FScopedConsoleVariableOverride()
	{
		if (ConsoleVariable)
		{
			ConsoleVariable->Set(PreviousValue, ECVF_SetByCode);
		}
	}

private:
	IConsoleVariable* ConsoleVariable = nullptr;
	int32 PreviousValue = 0;
};
#endif // WITH_SERVER_CODE

static FGorgeousAutomationScenarioResult RunNetworkingHarnessScenario(
	const FGorgeousAutomationScenarioContext& Context,
	TFunctionRef<void(FGorgeousServerClientHarnessOptions&)> ConfigureOptions,
	TFunctionRef<void(FGorgeousServerClientHarness&)> BeforeObservation,
	TFunctionRef<void(FGorgeousServerClientHarness&, FGorgeousAutomationScenarioResult&)> ValidateHarness)
{
	FGorgeousAutomationScenarioResult Result;

#if !WITH_SERVER_CODE
	Result.AddWarning(TEXT("WITH_SERVER_CODE disabled; networking harness unavailable."));
	return Result;
#else
	FGorgeousServerClientHarnessOptions Options;
	ConfigureOptions(Options);

	FGorgeousServerClientHarness Harness;
	FString HarnessError;
	if (!Harness.Initialize(Options, HarnessError))
	{
		Result.AddError(FString::Printf(TEXT("Harness initialization failed: %s"), *HarnessError));
		return Result;
	}

	ON_SCOPE_EXIT
	{
		Harness.Shutdown();
	};

	BeforeObservation(Harness);

	const float TickStep = static_cast<float>(Options.TickStepSeconds);
	const int32 WarmupIterations = FMath::Max(1, FMath::RoundToInt(static_cast<float>(GHarnessWarmupSeconds) / TickStep));
	const int32 ObservationIterations = FMath::Max(1, FMath::RoundToInt(static_cast<float>(GHarnessObservationSeconds) / TickStep));
	Harness.Tick(TickStep, WarmupIterations);
	if (!Harness.HasLiveConnection())
	{
		Result.AddError(TEXT("Harness never established a live connection."));
		return Result;
	}
	Harness.Tick(TickStep, ObservationIterations);

	ValidateHarness(Harness, Result);

	const FGorgeousServerClientHarness::FCollectedStats Stats = Harness.GatherStats();
	Result.AddMetric(TEXT("net.OutgoingKB"), Stats.OutgoingBytes / 1024.0);
	Result.AddMetric(TEXT("net.IncomingKB"), Stats.IncomingBytes / 1024.0);
	if (Stats.ServerSnapshot.bValid)
	{
		Result.AddMetric(TEXT("server.AvgPingMs"), Stats.ServerSnapshot.AvgPingMs);
		Result.AddMetric(TEXT("server.OutLoss"), Stats.ServerSnapshot.OutLossPercent);
		Result.AddMetric(TEXT("server.InLoss"), Stats.ServerSnapshot.InLossPercent);
		AppendDriverSnapshotNotes(Stats.ServerSnapshot, Result, TEXT("[Server] "));
	}
	if (Stats.ClientSnapshot.bValid)
	{
		Result.AddMetric(TEXT("client.AvgPingMs"), Stats.ClientSnapshot.AvgPingMs);
		Result.AddMetric(TEXT("client.OutLoss"), Stats.ClientSnapshot.OutLossPercent);
		Result.AddMetric(TEXT("client.InLoss"), Stats.ClientSnapshot.InLossPercent);
		AppendDriverSnapshotNotes(Stats.ClientSnapshot, Result, TEXT("[Client] "));
	}

	TArray<FString> MetricLines;
	Harness.CollectRegisteredNetworkMetrics(MetricLines);
	for (const FString& Line : MetricLines)
	{
		Result.AddNote(Line);
	}

	return Result;
#endif // WITH_SERVER_CODE
}

#if WITH_SERVER_CODE
enum class EGorgeousNetworkingBackend : uint8
{
	Iris,
	ReplicationGraph
};

static const EGorgeousNetworkingBackend GNetworkingBackends[] =
{
	EGorgeousNetworkingBackend::Iris,
	EGorgeousNetworkingBackend::ReplicationGraph
};

static const TCHAR* GetNetworkingBackendLabel(const EGorgeousNetworkingBackend Backend)
{
	switch (Backend)
	{
	case EGorgeousNetworkingBackend::Iris:
		return TEXT("Iris");
	case EGorgeousNetworkingBackend::ReplicationGraph:
		return TEXT("ReplicationGraph");
	default:
		return TEXT("Unknown");
	}
}

static FGorgeousAutomationScenarioResult RunNetworkingHarnessScenarioWithBackend(
	const FGorgeousAutomationScenarioContext& Context,
	const EGorgeousNetworkingBackend Backend,
	TFunctionRef<void(FGorgeousServerClientHarnessOptions&)> ConfigureOptions,
	TFunctionRef<void(FGorgeousServerClientHarness&)> BeforeObservation,
	TFunctionRef<void(FGorgeousServerClientHarness&, FGorgeousAutomationScenarioResult&)> ValidateHarness)
{
	FScopedConsoleVariableOverride UseIris(TEXT("net.UseIris"), Backend == EGorgeousNetworkingBackend::Iris ? 1 : 0);
	return RunNetworkingHarnessScenario(Context, ConfigureOptions, BeforeObservation, ValidateHarness);
}

static void MergeBackendResults(FGorgeousAutomationScenarioResult& CombinedResult, const FGorgeousAutomationScenarioResult& BackendResult, const TCHAR* BackendLabel)
{
	const FString Prefix = FString::Printf(TEXT("[%s] "), BackendLabel);
	CombinedResult.AddNote(FString::Printf(TEXT("[%s] %s"), BackendLabel, BackendResult.bSuccess ? TEXT("PASS") : TEXT("FAIL")));
	for (const FString& Note : BackendResult.Notes)
	{
		CombinedResult.AddNote(Prefix + Note);
	}
	for (const FString& Warning : BackendResult.Warnings)
	{
		CombinedResult.AddWarning(Prefix + Warning);
	}
	for (const FString& Error : BackendResult.Errors)
	{
		CombinedResult.AddError(Prefix + Error);
	}
	for (const TPair<FString, FString>& Metric : BackendResult.Metrics)
	{
		CombinedResult.AddMetric(FString::Printf(TEXT("%s.%s"), BackendLabel, *Metric.Key), Metric.Value);
	}
	CombinedResult.bSuccess = CombinedResult.bSuccess && BackendResult.bSuccess;
}
#endif // WITH_SERVER_CODE

static FGorgeousAutomationScenarioResult RunNetworkingHarnessScenarioForBackends(
	const FGorgeousAutomationScenarioContext& Context,
	TFunctionRef<void(FGorgeousServerClientHarnessOptions&)> ConfigureOptions,
	TFunctionRef<void(FGorgeousServerClientHarness&)> BeforeObservation,
	TFunctionRef<void(FGorgeousServerClientHarness&, FGorgeousAutomationScenarioResult&)> ValidateHarness)
{
#if WITH_SERVER_CODE
	FGorgeousAutomationScenarioResult CombinedResult;
	for (int32 BackendIndex = 0; BackendIndex < UE_ARRAY_COUNT(GNetworkingBackends); ++BackendIndex)
	{
		const EGorgeousNetworkingBackend Backend = GNetworkingBackends[BackendIndex];
		const FGorgeousAutomationScenarioResult BackendResult = RunNetworkingHarnessScenarioWithBackend(
			Context,
			Backend,
			ConfigureOptions,
			BeforeObservation,
			ValidateHarness);
		MergeBackendResults(CombinedResult, BackendResult, GetNetworkingBackendLabel(Backend));
	}
	return CombinedResult;
#else
	return RunNetworkingHarnessScenario(Context, ConfigureOptions, BeforeObservation, ValidateHarness);
#endif // WITH_SERVER_CODE
}

} // namespace GorgeousAutomationNetworking

static FGorgeousAutomationScenarioDescriptor MakeNetworking_Iris()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.Networking.Iris");
	D.DisplayName = TEXT("Networking: Iris Backend Smoke");
	D.Description = TEXT("Validates Iris backend activation against the shared harness and captures timing metrics.");
	D.Tags = { TEXT("networking"), TEXT("iris"), TEXT("smoke") };
	D.Priority = 100;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		using namespace GorgeousAutomationNetworking;

#if WITH_SERVER_CODE
		FScopedConsoleVariableOverride UseIris(TEXT("net.UseIris"), 1);
#endif

		FGorgeousAutomationScenarioResult Result = RunNetworkingHarnessScenario(
			Context,
			[](FGorgeousServerClientHarnessOptions& Options)
			{
				Options.ConnectTimeoutSeconds = 30.0;
			},
			[](FGorgeousServerClientHarness&)
			{
			},
			[](FGorgeousServerClientHarness& Harness, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
#if WITH_SERVER_CODE
				const UNetDriver* ServerDriver = Harness.GetServerDriver();
				const UNetDriver* ClientDriver = Harness.GetClientDriver();
				const bool bServerIris = ServerDriver && ServerDriver->IsUsingIrisReplication();
				const bool bClientIris = ClientDriver && ClientDriver->IsUsingIrisReplication();
				ScenarioResult.AddNote(FString::Printf(TEXT("Iris state: Server=%s | Client=%s"),
					bServerIris ? TEXT("true") : TEXT("false"),
					bClientIris ? TEXT("true") : TEXT("false")));
				if (!bServerIris || !bClientIris)
				{
					ScenarioResult.AddError(TEXT("Expected Iris replication backend on both harness endpoints."));
				}
#else
				ScenarioResult.AddWarning(TEXT("WITH_SERVER_CODE disabled; scenario skipped."));
#endif
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("networking"));
		return Result;
	};
	return D;
}

static FGorgeousAutomationScenarioDescriptor MakeNetworking_Graph()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.Networking.Graph");
	D.DisplayName = TEXT("Networking: Replication Graph");
	D.Description = TEXT("Turns off Iris, asserts replication graph activation, and records harness metrics.");
	D.Tags = { TEXT("networking"), TEXT("graph") };
	D.Priority = 90;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		using namespace GorgeousAutomationNetworking;

#if WITH_SERVER_CODE
		FScopedConsoleVariableOverride DisableIris(TEXT("net.UseIris"), 0);
#endif

		FGorgeousAutomationScenarioResult Result = RunNetworkingHarnessScenario(
			Context,
			[](FGorgeousServerClientHarnessOptions& Options)
			{
				Options.ConnectTimeoutSeconds = 30.0;
			},
			[](FGorgeousServerClientHarness& Harness)
			{
#if WITH_SERVER_CODE
				// Force Replication Graph on and Iris off for both harness worlds before observation starts.
				UGorgeousCoreRuntimeGlobals::SetAutoReplicationUseIrisOverride(Harness.GetServerWorld(), false);
				UGorgeousCoreRuntimeGlobals::SetAutoReplicationUseIrisOverride(Harness.GetClientWorld(), false);
				UGorgeousCoreRuntimeGlobals::SetAutoReplicationReplicationGraphOverride(Harness.GetServerWorld(), true);
				UGorgeousCoreRuntimeGlobals::SetAutoReplicationReplicationGraphOverride(Harness.GetClientWorld(), true);
				UGorgeousCoreRuntimeGlobals::InitializeAutoReplicationForWorld(Harness.GetServerWorld());
				UGorgeousCoreRuntimeGlobals::InitializeAutoReplicationForWorld(Harness.GetClientWorld());
#endif
			},
			[](FGorgeousServerClientHarness& Harness, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
#if WITH_SERVER_CODE
				const UNetDriver* ServerDriver = Harness.GetServerDriver();
				const UNetDriver* ClientDriver = Harness.GetClientDriver();
				const bool bServerGraph = ServerDriver && ServerDriver->GetReplicationDriver() && ServerDriver->GetReplicationDriver()->IsA(UReplicationGraph::StaticClass());
				const bool bClientGraph = ClientDriver && ClientDriver->GetReplicationDriver() && ClientDriver->GetReplicationDriver()->IsA(UReplicationGraph::StaticClass());
				if (!bServerGraph || !bClientGraph)
				{
					ScenarioResult.AddError(TEXT("Replication Graph is not active on both harness endpoints."));
				}

				const bool bServerIris = ServerDriver && ServerDriver->IsUsingIrisReplication();
				const bool bClientIris = ClientDriver && ClientDriver->IsUsingIrisReplication();
				if (bServerIris || bClientIris)
				{
					ScenarioResult.AddWarning(TEXT("Iris remained enabled while validating Replication Graph."));
				}
#else
				ScenarioResult.AddWarning(TEXT("WITH_SERVER_CODE disabled; scenario skipped."));
#endif
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("networking"));
		return Result;
	};
	return D;
}

static FGorgeousAutomationScenarioDescriptor MakeNetworking_NetEmulation()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.Networking.NetEmulation");
	D.DisplayName = TEXT("Networking: Net Emulation Preset Sweep");
	D.Description = TEXT("Applies a suite of network emulation presets to the harness and captures latency/loss metrics.");
	D.Tags = { TEXT("networking"), TEXT("netem"), TEXT("presets") };
	D.Priority = 80;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		using namespace GorgeousAutomationNetworking;
		const FNetEmulationPreset& Preset = GetNetEmulationPreset(Context.VariantIndex);

		auto ConfigurePacketSim = [&Preset](FGorgeousServerClientHarness& Harness)
		{
#if WITH_SERVER_CODE
			auto ApplySettings = [&Preset](UNetDriver* Driver)
			{
				if (!Driver)
				{
					return;
				}

				FPacketSimulationSettings Settings;
				Settings.PktLagMin = Preset.PktLagMin;
				Settings.PktLagMax = Preset.PktLagMax;
				Settings.PktLagVariance = Preset.PktLagVariance;
				Settings.PktIncomingLagMin = Preset.IncomingLagMin;
				Settings.PktIncomingLagMax = Preset.IncomingLagMax;
				Settings.PktLoss = Preset.PacketLossPercent;
				Settings.PktDup = Preset.PacketDupPercent;
				Driver->SetPacketSimulationSettings(Settings);
			};

			ApplySettings(Harness.GetServerDriver());
			ApplySettings(Harness.GetClientDriver());
#endif
		};

		auto ValidateEmulation = [&Preset](FGorgeousServerClientHarness& Harness, FGorgeousAutomationScenarioResult& ScenarioResult)
		{
#if WITH_SERVER_CODE
			const FGorgeousServerClientHarness::FCollectedStats Stats = Harness.GatherStats();
			ScenarioResult.AddNote(FString::Printf(TEXT("Applied net emulation preset '%s'."), Preset.Label));
			if (Stats.ClientSnapshot.bValid)
			{
				const double ObservedPing = Stats.ClientSnapshot.AvgPingMs;
				ScenarioResult.AddMetric(TEXT("netem.ObservedClientPing"), ObservedPing);
				if (ObservedPing + 5.0 < Preset.ExpectedMinPingMs && Preset.ExpectedMinPingMs > 0.0f)
				{
					ScenarioResult.AddWarning(FString::Printf(TEXT("Observed ping %.2f ms is below expected floor %.2f ms for preset '%s'."),
						ObservedPing,
						Preset.ExpectedMinPingMs,
						Preset.Label));
				}
			}
#else
			ScenarioResult.AddWarning(TEXT("WITH_SERVER_CODE disabled; scenario skipped."));
#endif
		};

		FGorgeousAutomationScenarioResult Result = RunNetworkingHarnessScenarioForBackends(
			Context,
			[](FGorgeousServerClientHarnessOptions& Options)
			{
				Options.ConnectTimeoutSeconds = 30.0;
			},
			ConfigurePacketSim,
			ValidateEmulation);

		Result.AddNote(FString::Printf(TEXT("Preset=%s | VariantIndex=%d"), Preset.Label, Context.VariantIndex));
		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("networking"));
		return Result;
	};
	return D;
}

REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeNetworking_Iris());
REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeNetworking_Graph());
REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeNetworking_NetEmulation());

#endif // WITH_DEV_AUTOMATION_TESTS
