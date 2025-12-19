#include "Automation/GorgeousAutomationTestMatrix.h"
#include "Automation/AutoReplicationTestMatrixCore.h"

// Register a tiny example scenario demonstrating the auto-replication registration.
static FGorgeousAutomationScenarioDescriptor MakeExampleDescriptor()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.Example.Success");
	D.DisplayName = TEXT("AutoReplication: Example Success");
	D.Description = TEXT("A tiny example scenario that always succeeds and emits a metric.");
	D.Tags = { TEXT("example"), TEXT("smoke") };
	D.Priority = 0;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result;
		Result.AddNote(FString::Printf(TEXT("Ran example scenario for variant %d"), Context.VariantIndex));
		Result.AddMetric(TEXT("Example.Metric.Value"), 1.0, TEXT("units"));

		// Persist result using harness helper (best-effort stub)
		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("examples"));

		return Result;
	};
	return D;
}

REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeExampleDescriptor());
