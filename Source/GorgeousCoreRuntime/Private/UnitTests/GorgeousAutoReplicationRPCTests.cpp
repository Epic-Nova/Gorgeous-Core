#include "CoreMinimal.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCPayloadLibrary.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationRPCGauntlet
{
	static const FName PlayerControllerEntryKey(TEXT("GorgeousPlayerController"));

	static UWorld* ResolveWorld(const FGorgeousInsightScenarioContext& Context)
	{
		if (Context.WorldContextObject)
		{
			return Context.WorldContextObject->GetWorld();
		}
		return GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	}

	static AGorgeousPlayerController* ResolveController(UWorld* World)
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

	static FGorgeousRPCPayload BuildPayload(UObject* Context, const TCHAR* HandlerName, int32 SequenceId)
	{
		FGorgeousRPCPayload Payload = UGorgeousAutoReplicationRPCPayloadLibrary::MakeAutoReplicationRPCPayload(HandlerName);
		UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCIntegerArgument(Context, Payload, TEXT("Sequence"), SequenceId);
		UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCStringArgument(Context, Payload, TEXT("Stamp"), FDateTime::UtcNow().ToString(TEXT("yyyy-MM-dd_HH-mm-ss")));
		return Payload;
	}

	static bool QueueRPC(AGorgeousPlayerController* Source, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, FGorgeousInsightScenarioResult& Result)
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

	static FGorgeousInsightScenarioResult RunGauntletRPC(const FGorgeousInsightScenarioContext& Context, const TCHAR* Label, EGorgeousAutoReplicationRPCType Type)
	{
		FGorgeousInsightScenarioResult Result;
		if (!FGorgeousInsightHarness::IsGauntletActive())
		{
			Result.AddWarning(TEXT("Gauntlet is required for this scenario."));
			return Result;
		}

		UWorld* World = ResolveWorld(Context);
		if (!World)
		{
			Result.AddError(TEXT("No world context available."));
			return Result;
		}

		AGorgeousPlayerController* Controller = ResolveController(World);
		if (!Controller)
		{
			Result.AddError(TEXT("No GorgeousPlayerController found in world."));
			return Result;
		}

		const int32 Sequence = Context.VariantIndex + 1;
		const FGorgeousRPCPayload Payload = BuildPayload(Controller, Label ? Label : TEXT("RPC"), Sequence);
		if (!QueueRPC(Controller, Type, Payload, Result))
		{
			return Result;
		}

		FGorgeousQueuedRPC Dequeued;
		if (UGorgeousCoreRuntimeGlobals::HasPendingAutoReplicationRPC(Controller) && UGorgeousCoreRuntimeGlobals::DequeuePendingAutoReplicationRPC(Controller, Dequeued))
		{
			Result.AddNote(FString::Printf(TEXT("Dequeued RPC sequence %d for %s"), Sequence, Label ? Label : TEXT("RPC")));
		}
		else
		{
			Result.AddWarning(TEXT("RPC queued but no pending entry was dequeued on this endpoint."));
		}

		Result.AddNote(FString::Printf(TEXT("Gauntlet role=%s | Controller=%s | Scenario=%s"),
			IsRunningDedicatedServer() ? TEXT("Server") : TEXT("Client"),
			*Controller->GetName(),
			Label ? Label : TEXT("RPC")));
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeRPC_Server()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.RPC.Server");
		D.DisplayName = TEXT("RPC (Gauntlet) Server");
		D.Description = TEXT("Gauntlet-only RPC sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("rpc") };
		D.Priority = 70;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletRPC(Context, TEXT("Server"), EGorgeousAutoReplicationRPCType::EReliableServer);
		};
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakeRPC_Client()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.RPC.Client");
		D.DisplayName = TEXT("RPC (Gauntlet) Client");
		D.Description = TEXT("Gauntlet-only RPC sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("rpc") };
		D.Priority = 65;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletRPC(Context, TEXT("Client"), EGorgeousAutoReplicationRPCType::EReliableClient);
		};
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakeRPC_Multicast()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.RPC.Multicast");
		D.DisplayName = TEXT("RPC (Gauntlet) Multicast");
		D.Description = TEXT("Gauntlet-only RPC sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("rpc") };
		D.Priority = 60;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletRPC(Context, TEXT("Multicast"), EGorgeousAutoReplicationRPCType::EReliableMulticast);
		};
		return D;
	}
}

REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationRPCGauntlet::MakeRPC_Server());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationRPCGauntlet::MakeRPC_Client());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationRPCGauntlet::MakeRPC_Multicast());

#endif // WITH_DEV_AUTOMATION_TESTS

