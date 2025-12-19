#include "Automation/GorgeousAutomationTestMatrix.h"
#include "Automation/AutoReplicationTestMatrixCore.h"
#include "AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCPayloadLibrary.h"
#include "UnitTests/Harness/GorgeousServerClientHarness.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Misc/ScopeExit.h"
#include "UnitTests/Helpers/GorgeousNetworkEmulationHelpers.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationRPC
{
static constexpr double GHarnessWarmupSeconds = 0.5;
static constexpr double GRPCTimeoutSeconds = 5.0;
static constexpr double GHarnessTick = 1.0 / 120.0;
static const FName PlayerControllerEntryKey = "GorgeousPlayerController"; //Not sure if this is correct

struct FRPCTestActors
{
	FGorgeousServerClientHarness& Harness;
	AGorgeousPlayerController* ServerController = nullptr;
	AGorgeousPlayerController* ClientController = nullptr;
	double TickStepSeconds = GHarnessTick;
};

static void DrainControllerRPCs(AGorgeousPlayerController* Controller)
{
	if (!Controller)
	{
		return;
	}

	FGorgeousQueuedRPC Discarded;
	while (UGorgeousCoreRuntimeGlobals::HasPendingAutoReplicationRPC(Controller))
	{
		if (!UGorgeousCoreRuntimeGlobals::DequeuePendingAutoReplicationRPC(Controller, Discarded))
		{
			break;
		}
	}
}

static AGorgeousPlayerController* ResolveFirstController(const UWorld* World)
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

static FString DescribeControllers(const UWorld* World, const TCHAR* Label)
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

static bool InitializeHarnessActors(FRPCTestActors& Actors, FGorgeousAutomationScenarioResult& Result)
{
	Actors.ServerController = ResolveFirstController(Actors.Harness.GetServerWorld());
	Actors.ClientController = ResolveFirstController(Actors.Harness.GetClientWorld());
	if (!Actors.ServerController || !Actors.ClientController)
	{
		Result.AddError(TEXT("Failed to locate GorgeousPlayerController instances on both harness endpoints."));
		return false;
	}

	DrainControllerRPCs(Actors.ServerController);
	DrainControllerRPCs(Actors.ClientController);

#if WITH_DEV_AUTOMATION_TESTS
	Actors.ServerController->ResetAutomationRPCWitnessEntries();
	Actors.ClientController->ResetAutomationRPCWitnessEntries();
#endif
	return true;
}

static FGorgeousRPCPayload BuildPayload(UObject* Context, const TCHAR* HandlerName, int32 SequenceId)
{
	FGorgeousRPCPayload Payload = UGorgeousAutoReplicationRPCPayloadLibrary::MakeAutoReplicationRPCPayload(HandlerName);
	UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCIntegerArgument(Context, Payload, TEXT("Sequence"), SequenceId);
	UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCStringArgument(Context, Payload, TEXT("Stamp"), FDateTime::UtcNow().ToString(TEXT("yyyy-MM-dd_HH-mm-ss")));
	return Payload;
}

static bool QueueRPC(AGorgeousPlayerController* Source, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, FGorgeousAutomationScenarioResult& Result)
{
	if (!Source)
	{
		Result.AddError(TEXT("RPC request failed because the source controller is null."));
		return false;
	}

	if (!UGorgeousCoreRuntimeGlobals::RequestAutoReplicationRPC(Source, PlayerControllerEntryKey, Type, Payload))
	{
		Result.AddError(FString::Printf(TEXT("RequestAutoReplicationRPC failed for %s."), *Source->GetName()));
		return false;
	}

	return true;
}

static bool WaitForRPC(const FRPCTestActors& Actors, AGorgeousPlayerController* Target, FGorgeousQueuedRPC& OutRPC, const TCHAR* Label, FGorgeousAutomationScenarioResult& Result)
{
	if (!Target)
	{
		Result.AddError(TEXT("RPC wait target controller is null."));
		return false;
	}

	const double TickStep = Actors.TickStepSeconds;
	const int32 MaxIterations = FMath::CeilToInt(GRPCTimeoutSeconds / TickStep);
	int32 Iterations = 0;
	while (Iterations < MaxIterations)
	{
		if (UGorgeousCoreRuntimeGlobals::HasPendingAutoReplicationRPC(Target))
		{
			if (UGorgeousCoreRuntimeGlobals::DequeuePendingAutoReplicationRPC(Target, OutRPC))
			{
				const double WaitSeconds = Iterations * TickStep;
				Result.AddNote(FString::Printf(TEXT("Received %s RPC after %d ticks (%.2fs)."), Label, Iterations, WaitSeconds));
				Result.AddMetric(FString::Printf(TEXT("rpc.%sLatencyTicks"), Label), static_cast<double>(Iterations));
				return true;
			}
		}

		Actors.Harness.Tick(static_cast<float>(TickStep), 1);
		++Iterations;
	}

	Result.AddError(FString::Printf(TEXT("Timed out waiting for %s RPC after %.2fs."), Label, GRPCTimeoutSeconds));
	return false;
}

static void AppendHarnessMetrics(const FGorgeousServerClientHarness& Harness, FGorgeousAutomationScenarioResult& Result)
{
	const FGorgeousServerClientHarness::FCollectedStats Stats = Harness.GatherStats();
	Result.AddMetric(TEXT("net.TxKB"), Stats.OutgoingBytes / 1024.0);
	Result.AddMetric(TEXT("net.RxKB"), Stats.IncomingBytes / 1024.0);
}

#if WITH_DEV_AUTOMATION_TESTS
static bool RequireRPCWitness(const AGorgeousPlayerController* Controller, const FName HandlerName, const TCHAR* Label, FGorgeousAutomationScenarioResult& Result)
{
	if (!Controller)
	{
		Result.AddError(FString::Printf(TEXT("%s controller unavailable while validating RPC witness."), Label));
		return false;
	}

	const TArray<FGorgeousAutomationRPCWitnessEntry>& Entries = Controller->GetAutomationRPCWitnessEntries();
	const FGorgeousAutomationRPCWitnessEntry* Found = Entries.FindByPredicate([&](const FGorgeousAutomationRPCWitnessEntry& Entry)
	{
		return Entry.HandlerName == HandlerName;
	});

	if (!Found)
	{
		Result.AddError(FString::Printf(TEXT("%s controller never executed handler %s."), Label, *HandlerName.ToString()));
		return false;
	}

	const UEnum* RoleEnum = StaticEnum<ENetRole>();
	const FString RoleLabel = RoleEnum ? RoleEnum->GetNameStringByValue(Found->NetRole.GetValue()) : FString::FromInt(Found->NetRole.GetValue());
	const ENetMode NetModeValue = static_cast<ENetMode>(Found->NetMode.GetValue());
	const TCHAR* NetModeLabel;
	switch (NetModeValue)
	{
	case NM_Standalone: NetModeLabel = TEXT("NM_Standalone"); break;
	case NM_DedicatedServer: NetModeLabel = TEXT("NM_DedicatedServer"); break;
	case NM_ListenServer: NetModeLabel = TEXT("NM_ListenServer"); break;
	case NM_Client: NetModeLabel = TEXT("NM_Client"); break;
	default: NetModeLabel = TEXT("NM_Unknown"); break;
	}
	Result.AddNote(FString::Printf(TEXT("%s witnessed %s (Role=%s Mode=%s)."), Label, *HandlerName.ToString(), *RoleLabel, NetModeLabel));
	Result.AddMetric(FString::Printf(TEXT("rpc.%sWitnessRole"), Label), static_cast<double>(Found->NetRole.GetValue()));
	return true;
}
#else
static bool RequireRPCWitness(AGorgeousPlayerController*, const FName, const TCHAR*, FGorgeousAutomationScenarioResult&)
{
	return true;
}
#endif

static FGorgeousAutomationScenarioResult RunRPCScenario(const FGorgeousAutomationScenarioContext& Context,
	TFunctionRef<void(FRPCTestActors&, FGorgeousAutomationScenarioResult&)> ScenarioBody)
{
	FGorgeousAutomationScenarioResult Result;
	const FGorgeousNetworkEmulation::FRuntimeProfile SuiteRuntime = FGorgeousNetworkEmulation::EnsureSuitePresetApplied(&Result);
	FGorgeousNetworkEmulation::AppendRuntimeMetrics(SuiteRuntime, Result, TEXT("rpc.netemu."));

#if !WITH_SERVER_CODE
	Result.AddWarning(TEXT("WITH_SERVER_CODE is disabled; RPC scenarios cannot execute."));
	return Result;
#else
	FGorgeousServerClientHarnessOptions Options;
	Options.ConnectTimeoutSeconds = 30.0;
	Options.TickStepSeconds = GHarnessTick;
	FGorgeousServerClientHarness Harness;
	FString Error;
	if (!Harness.Initialize(Options, Error))
	{
		Result.AddError(FString::Printf(TEXT("Failed to initialize harness: %s"), *Error));
		return Result;
	}

	ON_SCOPE_EXIT
	{
		Harness.Shutdown();
	};

	const int32 WarmupIterations = FMath::Max(1, FMath::RoundToInt(static_cast<float>(GHarnessWarmupSeconds / Options.TickStepSeconds)));
	Harness.Tick(static_cast<float>(Options.TickStepSeconds), WarmupIterations);
	if (!Harness.HasLiveConnection())
	{
		Result.AddError(TEXT("Harness never established a live server/client connection."));
		return Result;
	}

	AppendControllerSummary(Harness, Result);
	AppendConnectionSummary(Harness, Result);

	FRPCTestActors Actors{ Harness, nullptr, nullptr, Options.TickStepSeconds };
	if (!InitializeHarnessActors(Actors, Result))
	{
		return Result;
	}

	ScenarioBody(Actors, Result);
	AppendHarnessMetrics(Harness, Result);
	return Result;
#endif // WITH_SERVER_CODE
}

static void ValidateReceivedRPC(const FGorgeousQueuedRPC& RPC, EGorgeousAutoReplicationRPCType ExpectedType, const TCHAR* Label, FGorgeousAutomationScenarioResult& Result)
{
	if (RPC.Type != ExpectedType)
	{
		Result.AddError(FString::Printf(TEXT("%s RPC arrived with unexpected route (%d)."), Label, static_cast<int32>(RPC.Type)));
	}
	if (RPC.Key != PlayerControllerEntryKey)
	{
		Result.AddError(FString::Printf(TEXT("%s RPC targeted unexpected key %s."), Label, *RPC.Key.ToString()));
	}
	Result.AddNote(FString::Printf(TEXT("%s payload handler: %s (%d arguments)."), Label, *RPC.Payload.HandlerName.ToString(), RPC.Payload.Arguments.Num()));
}
} // namespace GorgeousAutomationRPC

using namespace GorgeousAutomationRPC;

static FGorgeousAutomationScenarioDescriptor MakeRPC_Server()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.RPC.Server");
	D.DisplayName = TEXT("RPC: Server-Initiated RPCs");
	D.Description = TEXT("Verifies server->client AutoReplication RPCs using the shared harness.");
	D.Tags = { TEXT("rpc"), TEXT("server"), TEXT("smoke") };
	D.Priority = 100;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunRPCScenario(Context,
			[](const FRPCTestActors& Actors, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				constexpr int32 SequenceId = 100;
				if (const FGorgeousRPCPayload Payload = BuildPayload(Actors.ServerController, TEXT("Automation_ServerToClient"), SequenceId);
					!QueueRPC(Actors.ServerController, EGorgeousAutoReplicationRPCType::EReliableClient, Payload, ScenarioResult))
				{
					return;
				}

				FGorgeousQueuedRPC Received;
				if (!WaitForRPC(Actors, Actors.ClientController, Received, TEXT("Client"), ScenarioResult))
				{
					return;
				}

				ValidateReceivedRPC(Received, EGorgeousAutoReplicationRPCType::EReliableClient, TEXT("Client"), ScenarioResult);
				RequireRPCWitness(Actors.ClientController, TEXT("Automation_ServerToClient"), TEXT("Client"), ScenarioResult);
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("rpc"));
		return Result;
	};
	return D;
}

static FGorgeousAutomationScenarioDescriptor MakeRPC_Client()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.RPC.Client");
	D.DisplayName = TEXT("RPC: Client->Server RPCs");
	D.Description = TEXT("Validates client-originated RPCs reach the server through the harness.");
	D.Tags = { TEXT("rpc"), TEXT("client") };
	D.Priority = 90;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunRPCScenario(Context,
			[](const FRPCTestActors& Actors, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				constexpr int32 SequenceId = 200;
				if (const FGorgeousRPCPayload Payload = BuildPayload(Actors.ClientController, TEXT("Automation_ClientToServer"), SequenceId);
					!QueueRPC(Actors.ClientController, EGorgeousAutoReplicationRPCType::EReliableServer, Payload, ScenarioResult))
				{
					return;
				}

				FGorgeousQueuedRPC Received;
				if (!WaitForRPC(Actors, Actors.ServerController, Received, TEXT("Server"), ScenarioResult))
				{
					return;
				}

				ValidateReceivedRPC(Received, EGorgeousAutoReplicationRPCType::EReliableServer, TEXT("Server"), ScenarioResult);
				RequireRPCWitness(Actors.ServerController, TEXT("Automation_ClientToServer"), TEXT("Server"), ScenarioResult);
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("rpc"));
		return Result;
	};
	return D;
}

static FGorgeousAutomationScenarioDescriptor MakeRPC_Multicast()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.RPC.Multicast");
	D.DisplayName = TEXT("RPC: Multicast");
	D.Description = TEXT("Ensures server multicast AutoReplication RPCs reach both server- and client-side controllers.");
	D.Tags = { TEXT("rpc"), TEXT("multicast") };
	D.Priority = 85;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result = RunRPCScenario(Context,
			[](const FRPCTestActors& Actors, FGorgeousAutomationScenarioResult& ScenarioResult)
			{
				constexpr int32 SequenceId = 300;
				if (const FGorgeousRPCPayload Payload = BuildPayload(Actors.ServerController, TEXT("Automation_Multicast"), SequenceId);
					!QueueRPC(Actors.ServerController, EGorgeousAutoReplicationRPCType::EReliableMulticast, Payload, ScenarioResult))
				{
					return;
				}

				if (FGorgeousQueuedRPC ClientRPC; WaitForRPC(Actors, Actors.ClientController, ClientRPC, TEXT("ClientMulticast"), ScenarioResult))
				{
					ValidateReceivedRPC(ClientRPC, EGorgeousAutoReplicationRPCType::EReliableMulticast, TEXT("ClientMulticast"), ScenarioResult);
					RequireRPCWitness(Actors.ClientController, TEXT("Automation_Multicast"), TEXT("ClientMulticast"), ScenarioResult);
				}

				if (FGorgeousQueuedRPC ServerRPC; WaitForRPC(Actors, Actors.ServerController, ServerRPC, TEXT("ServerMulticast"), ScenarioResult))
				{
					ValidateReceivedRPC(ServerRPC, EGorgeousAutoReplicationRPCType::EReliableMulticast, TEXT("ServerMulticast"), ScenarioResult);
					RequireRPCWitness(Actors.ServerController, TEXT("Automation_Multicast"), TEXT("ServerMulticast"), ScenarioResult);
				}
			});

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("rpc"));
		return Result;
	};
	return D;
}

REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeRPC_Server());
REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeRPC_Client());
REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeRPC_Multicast());

#endif // WITH_DEV_AUTOMATION_TESTS
