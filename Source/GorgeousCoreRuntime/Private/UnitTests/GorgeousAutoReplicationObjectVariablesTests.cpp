#include "Automation/GorgeousAutomationTestMatrix.h"
#include "Automation/AutoReplicationTestMatrixCore.h"
#include "UnitTests/Harness/GorgeousServerClientHarness.h"
#include "GorgeousObjectVariablePerfTestTypes.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "Engine/World.h"
#include "UnitTests/Helpers/GorgeousNetworkEmulationHelpers.h"
#include "Misc/ScopeExit.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationObjectVariables
{
static constexpr double GHarnessWarmupSeconds = 0.5;
static constexpr double GHarnessTickSeconds = 1.0 / 120.0;
static constexpr double GReplicationTimeoutSeconds = 5.0;
static constexpr double GStimulusIntervalSeconds = 0.2;
static const FName PlayerControllerEntryKey = "GorgeousPlayerController"; //Not sure if this is correct

struct FScenarioContext
{
	FGorgeousServerClientHarness Harness;
	AGorgeousPlayerController* ServerController = nullptr;
	AGorgeousPlayerController* ClientController = nullptr;
	double TickStepSeconds = GHarnessTickSeconds;
};

static AGorgeousPlayerController* ResolveFirstController(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (AGorgeousPlayerController* PC = Cast<AGorgeousPlayerController>(It->Get()))
		{
			return PC;
		}
	}

	return nullptr;
}

static FString DescribeControllers(UWorld* World, const TCHAR* Label)
{
	int32 Count = 0;
	FString First = TEXT("<none>");
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (const APlayerController* PC = It->Get())
			{
				if (Count == 0)
				{
					First = PC->GetName();
				}
				++Count;
			}
		}
	}
	return FString::Printf(TEXT("%s controllers: %d (first=%s)"), Label ? Label : TEXT("?"), Count, *First);
}

static FString DescribeWorldName(const UWorld* World, const TCHAR* Label)
{
	const FString WorldName = World
		? (World->GetOutermost() ? World->GetOutermost()->GetName() : World->GetMapName())
		: TEXT("<null>");
	return FString::Printf(TEXT("%s world: %s"), Label ? Label : TEXT("?"), *WorldName);
}

static void AppendControllerSummary(const FGorgeousServerClientHarness& Harness, FGorgeousAutomationScenarioResult& Result)
{
	Result.AddNote(DescribeControllers(Harness.GetServerWorld(), TEXT("Server")));
	Result.AddNote(DescribeControllers(Harness.GetClientWorld(), TEXT("Client")));
}

static FString DescribeConnectionSummary(const FGorgeousServerClientHarness& Harness, bool bServer)
{
#if WITH_SERVER_CODE
	UNetDriver* Driver = bServer ? Harness.GetServerDriver() : Harness.GetClientDriver();
	if (!Driver)
	{
		return FString::Printf(TEXT("%s driver: <null>"), bServer ? TEXT("Server") : TEXT("Client"));
	}

	if (bServer)
	{
		const int32 ConnCount = Driver->ClientConnections.Num();
		const FString ConnState = ConnCount > 0 && Driver->ClientConnections[0]
			? LexToString(Driver->ClientConnections[0]->GetConnectionState())
			: TEXT("None");
		return FString::Printf(TEXT("Server net: Driver=%s Conns=%d FirstState=%s"), *Driver->GetName(), ConnCount, *ConnState);
	}

	const FString ConnState = (Driver->ServerConnection)
		? LexToString(Driver->ServerConnection->GetConnectionState())
		: TEXT("None");
	return FString::Printf(TEXT("Client net: Driver=%s ServerState=%s"), *Driver->GetName(), *ConnState);
#else
	return FString::Printf(TEXT("%s net: WITH_SERVER_CODE=0"), bServer ? TEXT("Server") : TEXT("Client"));
#endif
}

static FString DescribeConnectionDetail(const FGorgeousServerClientHarness& Harness, bool bServer)
{
#if WITH_SERVER_CODE
	UNetDriver* Driver = bServer ? Harness.GetServerDriver() : Harness.GetClientDriver();
	UNetConnection* Conn = nullptr;
	if (Driver)
	{
		Conn = bServer ? (Driver->ClientConnections.IsValidIndex(0) ? Driver->ClientConnections[0] : nullptr) : Driver->ServerConnection;
	}

	const FString ConnState = Conn ? LexToString(Conn->GetConnectionState()) : TEXT("None");
	const FString OwningActor = (Conn && Conn->OwningActor) ? Conn->OwningActor->GetName() : TEXT("<null>");
	const FString PC = (Conn && Conn->PlayerController) ? Conn->PlayerController->GetName() : TEXT("<null>");
	return FString::Printf(TEXT("%s net detail: ConnState=%s OwningActor=%s PlayerController=%s"), bServer ? TEXT("Server") : TEXT("Client"), *ConnState, *OwningActor, *PC);
#else
	return FString::Printf(TEXT("%s net detail: WITH_SERVER_CODE=0"), bServer ? TEXT("Server") : TEXT("Client"));
#endif
}

static void AppendConnectionSummary(const FGorgeousServerClientHarness& Harness, FGorgeousAutomationScenarioResult& Result)
{
	Result.AddNote(DescribeConnectionSummary(Harness, true));
	Result.AddNote(DescribeConnectionSummary(Harness, false));
	Result.AddNote(DescribeConnectionDetail(Harness, true));
	Result.AddNote(DescribeConnectionDetail(Harness, false));
	Result.AddNote(DescribeWorldName(Harness.GetServerWorld(), TEXT("Server")));
	Result.AddNote(DescribeWorldName(Harness.GetClientWorld(), TEXT("Client")));
}

static bool InitializeScenarioContext(FScenarioContext& Context, FGorgeousAutomationScenarioResult& Result)
{
	FGorgeousServerClientHarnessOptions Options;
	Options.ConnectTimeoutSeconds = 30.0;
	Options.TickStepSeconds = GHarnessTickSeconds;
	FString HarnessError;
	if (!Context.Harness.Initialize(Options, HarnessError))
	{
		Result.AddError(FString::Printf(TEXT("Failed to initialize harness: %s"), *HarnessError));
		return false;
	}

	const int32 WarmupIterations = FMath::Max(1, FMath::RoundToInt(static_cast<float>(GHarnessWarmupSeconds / Options.TickStepSeconds)));
	Context.Harness.Tick(static_cast<float>(Options.TickStepSeconds), WarmupIterations);
	if (!Context.Harness.HasLiveConnection())
	{
		Result.AddError(TEXT("Harness never established a live server/client connection."));
		return false;
	}

	AppendControllerSummary(Context.Harness, Result);
	AppendConnectionSummary(Context.Harness, Result);

	Context.ServerController = ResolveFirstController(Context.Harness.GetServerWorld());
	Context.ClientController = ResolveFirstController(Context.Harness.GetClientWorld());
	if (!Context.ServerController || !Context.ClientController)
	{
		Result.AddError(TEXT("Failed to locate GorgeousPlayerController instances on both harness endpoints."));
		return false;
	}

	Context.TickStepSeconds = Options.TickStepSeconds;
	return true;
}

static void TickOnce(FScenarioContext& Context)
{
	Context.Harness.Tick(static_cast<float>(Context.TickStepSeconds), 1);
}

static void TickForSeconds(FScenarioContext& Context, const double Seconds)
{
	const int32 Iterations = FMath::Max(1, FMath::RoundToInt(Seconds / Context.TickStepSeconds));
	Context.Harness.Tick(static_cast<float>(Context.TickStepSeconds), Iterations);
}

static void InjectStimulus(UGorgeousPerfObjectVariable* Variable)
{
	if (!Variable)
	{
		return;
	}

	const double StampSeconds = FPlatformTime::Seconds();
	const float RandomScalar = FMath::FRand();
	Variable->InjectReplicationStimulus(static_cast<float>(StampSeconds), RandomScalar);
}

static UGorgeousPerfObjectVariable* CreatePerfVariable(AGorgeousPlayerController* Owner, const TCHAR* DebugLabel, const EGorgeousObjectVariableReplicationMode Mode, const bool bForceRootStack)
{
	if (!Owner)
	{
		return nullptr;
	}

	UGorgeousPerfObjectVariable* Variable = NewObject<UGorgeousPerfObjectVariable>(Owner);
	if (!Variable)
	{
		return nullptr;
	}

	Variable->bSupportsNetworking = true;
	Variable->ReplicationMode = Mode;
	Variable->RootNetworkConfig.bExposeThroughRootNetworkStack = bForceRootStack;
	Variable->RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
	Variable->RootNetworkConfig.ReplicationChannel = PlayerControllerEntryKey;
	Variable->bUseSharedNetworkStack = !bForceRootStack;
	Variable->EnsureSharedNetworkStackOwner(Owner);
	const FString DisplayLabel = FString::Printf(TEXT("Automation_%s"), DebugLabel ? DebugLabel : TEXT("ObjectVariable"));
	Variable->SetDisplayName(DisplayLabel);
	return Variable;
}

static bool AssignVariableToPlayerEntry(FScenarioContext& Context, UGorgeousPerfObjectVariable* Variable, FGorgeousAutomationScenarioResult& Result)
{
	if (!Context.ServerController || !Variable)
	{
		Result.AddError(TEXT("Invalid context while assigning object variable."));
		return false;
	}

	Variable->EnsureSharedNetworkStackOwner(Context.ServerController);
	if (!UGorgeousCoreRuntimeGlobals::SetNetGorgeousAutoReplicationValue(Context.ServerController, PlayerControllerEntryKey, Variable, Context.ServerController))
	{
		Result.AddError(TEXT("Failed to bind perf object variable to the player controller entry."));
		return false;
	}

	return true;
}

static bool TryGetClientPerfVariable(FScenarioContext& Context, UGorgeousPerfObjectVariable*& OutVariable)
{
	OutVariable = nullptr;
	UGorgeousObjectVariable* RawValue = nullptr;
	if (!UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(Context.ClientController, PlayerControllerEntryKey, RawValue, Context.ClientController))
	{
		return false;
	}

	OutVariable = Cast<UGorgeousPerfObjectVariable>(RawValue);
	return OutVariable != nullptr;
}

static bool WaitForClientSequence(FScenarioContext& Context, const int32 TargetSequence, const double TimeoutSeconds, FGorgeousAutomationScenarioResult& Result, UGorgeousPerfObjectVariable*& OutClientVariable)
{
	UGorgeousPerfObjectVariable* LatestClientVar = nullptr;
	const int32 MaxIterations = FMath::Max(1, FMath::CeilToInt(TimeoutSeconds / Context.TickStepSeconds));
	for (int32 Iteration = 0; Iteration < MaxIterations; ++Iteration)
	{
		TryGetClientPerfVariable(Context, LatestClientVar);
		if (LatestClientVar && LatestClientVar->GetPerfReplicationStimulusCount() >= TargetSequence)
		{
			OutClientVariable = LatestClientVar;
			return true;
		}

		TickOnce(Context);
	}

	const int32 ObservedSequence = LatestClientVar ? LatestClientVar->GetPerfReplicationStimulusCount() : -1;
	Result.AddError(FString::Printf(TEXT("Timed out waiting for client sequence %d (last observed %d)."), TargetSequence, ObservedSequence));
	return false;
}

static void AppendHarnessMetrics(const FGorgeousServerClientHarness& Harness, const TCHAR* Prefix, FGorgeousAutomationScenarioResult& Result)
{
	const FGorgeousServerClientHarness::FCollectedStats Stats = Harness.GatherStats();
	const FString PrefixString = Prefix ? Prefix : TEXT("");
	Result.AddMetric(PrefixString + TEXT("net.TxKB"), Stats.OutgoingBytes / 1024.0);
	Result.AddMetric(PrefixString + TEXT("net.RxKB"), Stats.IncomingBytes / 1024.0);
	if (Stats.ServerSnapshot.bValid)
	{
		Result.AddMetric(PrefixString + TEXT("server.PingMs"), Stats.ServerSnapshot.AvgPingMs);
	}
	if (Stats.ClientSnapshot.bValid)
	{
		Result.AddMetric(PrefixString + TEXT("client.PingMs"), Stats.ClientSnapshot.AvgPingMs);
	}
}
} // namespace GorgeousAutomationObjectVariables

using namespace GorgeousAutomationObjectVariables;

static FGorgeousAutomationScenarioResult RunObjectVariableScenario(const FGorgeousAutomationScenarioContext& Context,
	TFunctionRef<void(FScenarioContext&, FGorgeousAutomationScenarioResult&)> ScenarioBody)
{
	FGorgeousAutomationScenarioResult Result;
	const FGorgeousNetworkEmulation::FRuntimeProfile SuiteRuntime = FGorgeousNetworkEmulation::EnsureSuitePresetApplied(&Result);
	FGorgeousNetworkEmulation::AppendRuntimeMetrics(SuiteRuntime, Result, TEXT("objectvariables.netemu."));

#if !WITH_SERVER_CODE
	Result.AddWarning(TEXT("WITH_SERVER_CODE is disabled; object variable scenarios are skipped."));
	return Result;
#else
	FScenarioContext ScenarioContext;
	if (!InitializeScenarioContext(ScenarioContext, Result))
	{
		ScenarioContext.Harness.Shutdown();
		return Result;
	}

	ON_SCOPE_EXIT
	{
		ScenarioContext.Harness.Shutdown();
	};

	ScenarioBody(ScenarioContext, Result);
	AppendHarnessMetrics(ScenarioContext.Harness, TEXT("objectvariables."), Result);
	return Result;
#endif // WITH_SERVER_CODE
}

static FGorgeousAutomationScenarioDescriptor MakeObjectVariables_FullAuto()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.ObjectVariables.FullAuto");
	D.DisplayName = TEXT("ObjectVariables: Full Auto Replication");
	D.Description = TEXT("Validates FullAuto replication streams using the server/client harness.");
	D.Tags = { TEXT("objectvariables"), TEXT("fullauto"), TEXT("smoke") };
	D.Priority = 100;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunObjectVariableScenario(Context,
			[](FScenarioContext& ScenarioContext, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				UGorgeousPerfObjectVariable* ServerVariable = CreatePerfVariable(ScenarioContext.ServerController, TEXT("FullAuto"), EGorgeousObjectVariableReplicationMode::EFullAutoReplication, true);
				if (!ServerVariable)
				{
					ScenarioResult.AddError(TEXT("Failed to create perf object variable for FullAuto scenario."));
					return;
				}

				ServerVariable->SetAutoReplicationRespectAccessPolicy(true);
				if (!AssignVariableToPlayerEntry(ScenarioContext, ServerVariable, ScenarioResult))
				{
					return;
				}

				for (int32 StimulusIndex = 0; StimulusIndex < 3; ++StimulusIndex)
				{
					InjectStimulus(ServerVariable);
					TickForSeconds(ScenarioContext, GStimulusIntervalSeconds);
				}

				const int32 TargetSequence = ServerVariable->GetPerfReplicationStimulusCount();
				UGorgeousPerfObjectVariable* ClientVariable = nullptr;
				if (!WaitForClientSequence(ScenarioContext, TargetSequence, GReplicationTimeoutSeconds, ScenarioResult, ClientVariable))
				{
					return;
				}

				ScenarioResult.AddNote(FString::Printf(TEXT("FullAuto replication synced sequence %d."), TargetSequence));
				ScenarioResult.AddMetric(TEXT("ov.fullauto.Sequence"), static_cast<double>(TargetSequence));
				ScenarioResult.AddMetric(TEXT("ov.fullauto.ServerSends"), static_cast<double>(ServerVariable->GetPerfNetSendCount()));
				ScenarioResult.AddMetric(TEXT("ov.fullauto.ClientReceives"), static_cast<double>(ClientVariable->GetPerfNetReceiveCount()));
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("objectvariables"));
		return Result;
	};
	return D;
}

static FGorgeousAutomationScenarioDescriptor MakeObjectVariables_Hybrid()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.ObjectVariables.Hybrid");
	D.DisplayName = TEXT("ObjectVariables: Hybrid Replication");
	D.Description = TEXT("Ensures hybrid object variables engage both auto replication and legacy subobject paths.");
	D.Tags = { TEXT("objectvariables"), TEXT("hybrid") };
	D.Priority = 90;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunObjectVariableScenario(Context,
			[](FScenarioContext& ScenarioContext, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				UGorgeousPerfObjectVariable* ServerVariable = CreatePerfVariable(ScenarioContext.ServerController, TEXT("Hybrid"), EGorgeousObjectVariableReplicationMode::EHybrid, true);
				if (!ServerVariable)
				{
					ScenarioResult.AddError(TEXT("Failed to create perf object variable for Hybrid scenario."));
					return;
				}

				ServerVariable->SetAutoReplicationRespectAccessPolicy(true);
				if (!AssignVariableToPlayerEntry(ScenarioContext, ServerVariable, ScenarioResult))
				{
					return;
				}

				if (!ServerVariable->SupportsAutoReplicationFeatures())
				{
					ScenarioResult.AddError(TEXT("Hybrid variable failed to activate auto replication features."));
					return;
				}
				if (!ServerVariable->SupportsLegacyReplication())
				{
					ScenarioResult.AddError(TEXT("Hybrid variable is missing legacy replication support."));
					return;
				}

				InjectStimulus(ServerVariable);
				TickForSeconds(ScenarioContext, GStimulusIntervalSeconds);
				InjectStimulus(ServerVariable);

				const int32 TargetSequence = ServerVariable->GetPerfReplicationStimulusCount();
				UGorgeousPerfObjectVariable* ClientVariable = nullptr;
				if (!WaitForClientSequence(ScenarioContext, TargetSequence, GReplicationTimeoutSeconds, ScenarioResult, ClientVariable))
				{
					return;
				}

				ScenarioResult.AddNote(ServerVariable->IsLegacyReplicationRegistered()
					? TEXT("Hybrid variable registered as a replicated subobject and streamed via auto replication.")
					: TEXT("Hybrid variable did not register as a replicated subobject (legacy path unavailable)."));
				ScenarioResult.AddMetric(TEXT("ov.hybrid.Sequence"), static_cast<double>(TargetSequence));
				ScenarioResult.AddMetric(TEXT("ov.hybrid.ServerSends"), static_cast<double>(ServerVariable->GetPerfNetSendCount()));
				ScenarioResult.AddMetric(TEXT("ov.hybrid.ClientReceives"), static_cast<double>(ClientVariable->GetPerfNetReceiveCount()));
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("objectvariables"));
		return Result;
	};
	return D;
}

static FGorgeousAutomationScenarioDescriptor MakeObjectVariables_Manual()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.ObjectVariables.Manual");
	D.DisplayName = TEXT("ObjectVariables: Manual (Legacy) Replication");
	D.Description = TEXT("Verifies legacy-only object variables continue to replicate via the replicated subobject path.");
	D.Tags = { TEXT("objectvariables"), TEXT("manual"), TEXT("legacy") };
	D.Priority = 80;
	D.bEnabledByDefault = false;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunObjectVariableScenario(Context,
			[](FScenarioContext& ScenarioContext, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				UGorgeousPerfObjectVariable* ServerVariable = CreatePerfVariable(ScenarioContext.ServerController, TEXT("Manual"), EGorgeousObjectVariableReplicationMode::EManual, false);
				if (!ServerVariable)
				{
					ScenarioResult.AddError(TEXT("Failed to create perf object variable for Manual scenario."));
					return;
				}

				if (!AssignVariableToPlayerEntry(ScenarioContext, ServerVariable, ScenarioResult))
				{
					return;
				}

				if (ServerVariable->SupportsAutoReplicationFeatures())
				{
					ScenarioResult.AddError(TEXT("Manual replication should not enable auto replication features."));
					return;
				}
				if (!ServerVariable->SupportsLegacyReplication())
				{
					ScenarioResult.AddError(TEXT("Manual replication lost legacy support."));
					return;
				}

				InjectStimulus(ServerVariable);
				TickForSeconds(ScenarioContext, GStimulusIntervalSeconds);
				InjectStimulus(ServerVariable);

				const int32 TargetSequence = ServerVariable->GetPerfReplicationStimulusCount();
				UGorgeousPerfObjectVariable* ClientVariable = nullptr;
				if (!WaitForClientSequence(ScenarioContext, TargetSequence, GReplicationTimeoutSeconds, ScenarioResult, ClientVariable))
				{
					return;
				}

				ScenarioResult.AddNote(ServerVariable->IsLegacyReplicationRegistered()
					? TEXT("Manual variable replicated via the legacy subobject pipeline.")
					: TEXT("Manual variable never registered for legacy replication."));
				ScenarioResult.AddMetric(TEXT("ov.manual.Sequence"), static_cast<double>(TargetSequence));
				ScenarioResult.AddMetric(TEXT("ov.manual.ServerSends"), static_cast<double>(ServerVariable->GetPerfNetSendCount()));
				ScenarioResult.AddMetric(TEXT("ov.manual.ClientReceives"), static_cast<double>(ClientVariable->GetPerfNetReceiveCount()));
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("objectvariables"));
		return Result;
	};
	return D;
}

REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeObjectVariables_FullAuto());
REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeObjectVariables_Hybrid());
REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeObjectVariables_Manual());

#endif // WITH_DEV_AUTOMATION_TESTS
