#include "Automation/GorgeousAutomationTestMatrix.h"
#include "Automation/AutoReplicationTestMatrixCore.h"
#include "UnitTests/Harness/GorgeousServerClientHarness.h"
#include "GorgeousObjectVariablePerfTestTypes.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "UnitTests/Helpers/GorgeousNetworkEmulationHelpers.h"
#include "Misc/ScopeExit.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationAccessPolicy
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

static UGorgeousPerfAccessPolicyObjectVariable* CreateAccessPolicyVariable(AGorgeousPlayerController* Owner, const TCHAR* DebugLabel)
{
	if (!Owner)
	{
		return nullptr;
	}

	UGorgeousPerfAccessPolicyObjectVariable* Variable = NewObject<UGorgeousPerfAccessPolicyObjectVariable>(Owner);
	if (!Variable)
	{
		return nullptr;
	}

	Variable->bSupportsNetworking = true;
	Variable->ReplicationMode = EGorgeousObjectVariableReplicationMode::EFullAutoReplication;
	Variable->RootNetworkConfig.bExposeThroughRootNetworkStack = true;
	Variable->RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Custom;
	Variable->RootNetworkConfig.ReplicationChannel = PlayerControllerEntryKey;
	Variable->SetAutoReplicationRespectAccessPolicy(true);
	Variable->EnsureSharedNetworkStackOwner(Owner);
	Variable->SetDisplayName(FString::Printf(TEXT("AccessPolicy_%s"), DebugLabel ? DebugLabel : TEXT("Var")));
	return Variable;
}

static bool AssignVariableToEntry(FScenarioContext& Context, UGorgeousPerfObjectVariable* Variable, FGorgeousAutomationScenarioResult& Result)
{
	if (!Variable || !Context.ServerController)
	{
		Result.AddError(TEXT("Invalid variable context during access-policy assignment."));
		return false;
	}

	if (!UGorgeousCoreRuntimeGlobals::SetNetGorgeousAutoReplicationValue(Context.ServerController, PlayerControllerEntryKey, Variable, Context.ServerController))
	{
		Result.AddError(TEXT("Failed to bind access-policy variable to the player controller entry."));
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
	Result.AddError(FString::Printf(TEXT("Timed out waiting for client sequence %d (observed %d)."), TargetSequence, ObservedSequence));
	return false;
}

static bool VerifyClientSequenceStable(FScenarioContext& Context, const int32 ExpectedMaxSequence, const double TimeoutSeconds, FGorgeousAutomationScenarioResult& Result)
{
	UGorgeousPerfObjectVariable* LatestClientVar = nullptr;
	const int32 MaxIterations = FMath::Max(1, FMath::CeilToInt(TimeoutSeconds / Context.TickStepSeconds));
	for (int32 Iteration = 0; Iteration < MaxIterations; ++Iteration)
	{
		TryGetClientPerfVariable(Context, LatestClientVar);
		if (LatestClientVar && LatestClientVar->GetPerfReplicationStimulusCount() > ExpectedMaxSequence)
		{
			Result.AddError(FString::Printf(TEXT("Client sequence advanced to %d despite deny policy (expected <= %d)."), LatestClientVar->GetPerfReplicationStimulusCount(), ExpectedMaxSequence));
			return false;
		}
		TickOnce(Context);
	}

	return true;
}

static void AppendHarnessMetrics(const FGorgeousServerClientHarness& Harness, FGorgeousAutomationScenarioResult& Result)
{
	const FGorgeousServerClientHarness::FCollectedStats Stats = Harness.GatherStats();
	Result.AddMetric(TEXT("accesspolicy.net.TxKB"), Stats.OutgoingBytes / 1024.0);
	Result.AddMetric(TEXT("accesspolicy.net.RxKB"), Stats.IncomingBytes / 1024.0);
}
} // namespace GorgeousAutomationAccessPolicy

using namespace GorgeousAutomationAccessPolicy;

static FGorgeousAutomationScenarioResult RunAccessPolicyScenario(const FGorgeousAutomationScenarioContext& Context,
	TFunctionRef<void(FScenarioContext&, FGorgeousAutomationScenarioResult&)> ScenarioBody)
{
	FGorgeousAutomationScenarioResult Result;
	const FGorgeousNetworkEmulation::FRuntimeProfile SuiteRuntime = FGorgeousNetworkEmulation::EnsureSuitePresetApplied(&Result);
	FGorgeousNetworkEmulation::AppendRuntimeMetrics(SuiteRuntime, Result, TEXT("accesspolicy.netemu."));

#if !WITH_SERVER_CODE
	Result.AddWarning(TEXT("WITH_SERVER_CODE is disabled; access policy scenarios are skipped."));
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
	AppendHarnessMetrics(ScenarioContext.Harness, Result);
	return Result;
#endif // WITH_SERVER_CODE
}

static FGorgeousAutomationScenarioDescriptor MakeAccessPolicy_AllowDeny()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.AccessPolicy.AllowDeny");
	D.DisplayName = TEXT("AccessPolicy: Allow/Deny Mix");
	D.Description = TEXT("Validates that custom access policies permit or deny replication as configured.");
	D.Tags = { TEXT("accesspolicy"), TEXT("security"), TEXT("smoke") };
	D.Priority = 100;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunAccessPolicyScenario(Context,
			[](FScenarioContext& ScenarioContext, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				UGorgeousPerfAccessPolicyObjectVariable* ServerVariable = CreateAccessPolicyVariable(ScenarioContext.ServerController, TEXT("AllowDeny"));
				if (!ServerVariable)
				{
					ScenarioResult.AddError(TEXT("Failed to allocate access-policy test variable."));
					return;
				}

				if (!AssignVariableToEntry(ScenarioContext, ServerVariable, ScenarioResult))
				{
					return;
				}

				ServerVariable->EnableCustomAccessPolicy(ScenarioContext.ServerController);
				ServerVariable->SetCustomAccessAllowed(true);

				for (int32 StimulusIndex = 0; StimulusIndex < 2; ++StimulusIndex)
				{
					InjectStimulus(ServerVariable);
					TickForSeconds(ScenarioContext, GStimulusIntervalSeconds);
				}

				const int32 AllowedSequence = ServerVariable->GetPerfReplicationStimulusCount();
				UGorgeousPerfObjectVariable* ClientVariable = nullptr;
				if (!WaitForClientSequence(ScenarioContext, AllowedSequence, GReplicationTimeoutSeconds, ScenarioResult, ClientVariable))
				{
					return;
				}

				ScenarioResult.AddNote(FString::Printf(TEXT("Allow stage synced sequence %d."), AllowedSequence));
				ScenarioResult.AddMetric(TEXT("accesspolicy.allow.Sequence"), static_cast<double>(AllowedSequence));
				ScenarioResult.AddMetric(TEXT("accesspolicy.allow.ServerSends"), static_cast<double>(ServerVariable->GetPerfNetSendCount()));
				ScenarioResult.AddMetric(TEXT("accesspolicy.allow.ClientReceives"), ClientVariable ? static_cast<double>(ClientVariable->GetPerfNetReceiveCount()) : 0.0);

				const int32 AllowEvaluations = ServerVariable->GetAccessPolicyAllowCount();
				const int32 DenyEvaluations = ServerVariable->GetAccessPolicyDenyCount();

				ServerVariable->SetCustomAccessAllowed(false);
				const int32 DenySequenceTarget = ServerVariable->GetPerfReplicationStimulusCount();
				InjectStimulus(ServerVariable);
				TickForSeconds(ScenarioContext, GStimulusIntervalSeconds);
				if (!VerifyClientSequenceStable(ScenarioContext, DenySequenceTarget, GReplicationTimeoutSeconds, ScenarioResult))
				{
					return;
				}

				ScenarioResult.AddNote(FString::Printf(TEXT("Deny stage suppressed replication beyond sequence %d."), DenySequenceTarget));
				ScenarioResult.AddMetric(TEXT("accesspolicy.deny.Sequence"), static_cast<double>(DenySequenceTarget));
				ScenarioResult.AddMetric(TEXT("accesspolicy.eval.Allowed"), static_cast<double>(AllowEvaluations));
				ScenarioResult.AddMetric(TEXT("accesspolicy.eval.Denied"), static_cast<double>(ServerVariable->GetAccessPolicyDenyCount() - DenyEvaluations));
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("accesspolicy"));
		return Result;
	};
	return D;
}

static FGorgeousAutomationScenarioDescriptor MakeAccessPolicy_Timing()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.AccessPolicy.Timing");
	D.DisplayName = TEXT("AccessPolicy: Timing/Performance" );
	D.Description = TEXT("Measures allow/deny latency and evaluation counters under policy toggles.");
	D.Tags = { TEXT("accesspolicy"), TEXT("timing") };
	D.Priority = 90;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunAccessPolicyScenario(Context,
			[](FScenarioContext& ScenarioContext, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				UGorgeousPerfAccessPolicyObjectVariable* ServerVariable = CreateAccessPolicyVariable(ScenarioContext.ServerController, TEXT("Timing"));
				if (!ServerVariable)
				{
					ScenarioResult.AddError(TEXT("Failed to allocate timing access-policy variable."));
					return;
				}

				if (!AssignVariableToEntry(ScenarioContext, ServerVariable, ScenarioResult))
				{
					return;
				}

				ServerVariable->EnableCustomAccessPolicy(ScenarioContext.ServerController);
				ServerVariable->ResetAccessPolicyCounters();
				UGorgeousPerfObjectVariable* ClientVariable = nullptr;

				const double AllowStart = FPlatformTime::Seconds();
				ServerVariable->SetCustomAccessAllowed(true);
				InjectStimulus(ServerVariable);
				const int32 AllowSequence = ServerVariable->GetPerfReplicationStimulusCount();
				if (!WaitForClientSequence(ScenarioContext, AllowSequence, GReplicationTimeoutSeconds, ScenarioResult, ClientVariable))
				{
					return;
				}
				const double AllowDurationMs = (FPlatformTime::Seconds() - AllowStart) * 1000.0;

				ServerVariable->SetCustomAccessAllowed(false);
				const double DenyStart = FPlatformTime::Seconds();
				const int32 DenySequence = ServerVariable->GetPerfReplicationStimulusCount();
				InjectStimulus(ServerVariable);
				TickForSeconds(ScenarioContext, GStimulusIntervalSeconds);
				if (!VerifyClientSequenceStable(ScenarioContext, DenySequence, GReplicationTimeoutSeconds, ScenarioResult))
				{
					return;
				}
				const double DenyDurationMs = (FPlatformTime::Seconds() - DenyStart) * 1000.0;

				ServerVariable->SetCustomAccessAllowed(true);
				InjectStimulus(ServerVariable);
				const int32 ReopenSequence = ServerVariable->GetPerfReplicationStimulusCount();
				if (!WaitForClientSequence(ScenarioContext, ReopenSequence, GReplicationTimeoutSeconds, ScenarioResult, ClientVariable))
				{
					return;
				}

				ScenarioResult.AddMetric(TEXT("accesspolicy.timing.AllowMs"), AllowDurationMs);
				ScenarioResult.AddMetric(TEXT("accesspolicy.timing.DenyMs"), DenyDurationMs);
				ScenarioResult.AddMetric(TEXT("accesspolicy.timing.AllowCount"), static_cast<double>(ServerVariable->GetAccessPolicyAllowCount()));
				ScenarioResult.AddMetric(TEXT("accesspolicy.timing.DenyCount"), static_cast<double>(ServerVariable->GetAccessPolicyDenyCount()));
				ScenarioResult.AddMetric(TEXT("accesspolicy.timing.ReopenSequence"), static_cast<double>(ReopenSequence));
				ScenarioResult.AddNote(FString::Printf(TEXT("Allow %.2f ms | Deny %.2f ms | Evaluations A=%d D=%d"),
					AllowDurationMs,
					DenyDurationMs,
					ServerVariable->GetAccessPolicyAllowCount(),
					ServerVariable->GetAccessPolicyDenyCount()));
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("accesspolicy"));
		return Result;
	};
	return D;
}

REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeAccessPolicy_AllowDeny());
REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeAccessPolicy_Timing());

#endif // WITH_DEV_AUTOMATION_TESTS
