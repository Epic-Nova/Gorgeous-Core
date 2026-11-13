#include "CoreMinimal.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "Engine/NetConnection.h"
#include "ReplicationGraph.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationNetworkingGauntlet
{
	static FGorgeousInsightScenarioResult RunNetworkingScenario(const FGorgeousInsightScenarioContext& Context, const TCHAR* Label)
	{
		FGorgeousInsightScenarioResult Result;
		if (!FGorgeousInsightHarness::IsGauntletActive())
		{
			Result.AddError(TEXT("Gauntlet is required for this scenario."));
			return Result;
		}
		UWorld* World = Context.WorldContextObject ? Context.WorldContextObject->GetWorld() : (GEngine ? GEngine->GetCurrentPlayWorld() : nullptr);
		UNetDriver* Driver = World ? World->GetNetDriver() : nullptr;
		if (!World || !World->HasBegunPlay() || !Driver || World->GetNetMode() == NM_Standalone)
		{
			Result.AddError(TEXT("A begun, networked game world with a real NetDriver is required."));
			return Result;
		}
		int32 Connected = 0;
		if (Driver->ServerConnection && Driver->ServerConnection->GetConnectionState() == USOCK_Open) ++Connected;
		for (UNetConnection* Connection : Driver->ClientConnections)
			if (Connection && Connection->GetConnectionState() == USOCK_Open) ++Connected;
		if (Connected == 0) Result.AddError(TEXT("No actual open remote connection is available."));
		if (FCString::Strcmp(Label, TEXT("Iris")) == 0)
		{
#if UE_WITH_IRIS
			if (!Driver->GetReplicationSystem()) Result.AddError(TEXT("The actual NetDriver has no native Iris ReplicationSystem."));
#else
			Result.AddError(TEXT("This engine target does not compile native Iris support."));
#endif
		}
		else if (!Cast<UReplicationGraph>(Driver->GetReplicationDriver()))
		{
			Result.AddError(TEXT("The actual NetDriver has no Replication Graph driver."));
		}
		Result.AddMetric(TEXT("networking.openRemoteConnections"), Connected);
		Result.AddNote(FString::Printf(TEXT("Actual NetDriver=%s | ReplicationDriver=%s"), *Driver->GetClass()->GetPathName(), *GetPathNameSafe(Driver->GetReplicationDriver())));

		Result.AddNote(FString::Printf(TEXT("Gauntlet role=%s | Scenario=%s"),
			IsRunningDedicatedServer() ? TEXT("Server") : TEXT("Client"),
			Label ? Label : TEXT("Networking")));
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeNetworking_Iris()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Networking.Iris");
		D.DisplayName = TEXT("Networking (Gauntlet) Iris");
		D.Description = TEXT("Gauntlet-only networking sanity check (Iris backend)." );
		D.Tags = { TEXT("gauntlet"), TEXT("networking"), TEXT("iris") };
		D.Priority = 80;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunNetworkingScenario(Context, TEXT("Iris"));
		};
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakeNetworking_Graph()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Networking.Graph");
		D.DisplayName = TEXT("Networking (Gauntlet) Replication Graph");
		D.Description = TEXT("Gauntlet-only networking sanity check (Replication Graph backend)." );
		D.Tags = { TEXT("gauntlet"), TEXT("networking"), TEXT("graph") };
		D.Priority = 75;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunNetworkingScenario(Context, TEXT("Graph"));
		};
		return D;
	}
}

REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationNetworkingGauntlet::MakeNetworking_Iris());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationNetworkingGauntlet::MakeNetworking_Graph());

#endif // WITH_DEV_AUTOMATION_TESTS
