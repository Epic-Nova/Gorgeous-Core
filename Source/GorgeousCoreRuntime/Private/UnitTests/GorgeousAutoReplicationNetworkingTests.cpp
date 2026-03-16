#include "CoreMinimal.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationNetworkingGauntlet
{
	static FGorgeousInsightScenarioResult RunNetworkingScenario(const FGorgeousInsightScenarioContext& Context, const TCHAR* Label)
	{
		FGorgeousInsightScenarioResult Result;
		if (!FGorgeousInsightHarness::IsGauntletActive())
		{
			Result.AddWarning(TEXT("Gauntlet is required for this scenario."));
			return Result;
		}

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
