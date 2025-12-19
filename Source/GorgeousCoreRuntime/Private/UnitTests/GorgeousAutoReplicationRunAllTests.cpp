#include "Automation/GorgeousAutomationTestMatrix.h"
#include "Automation/AutoReplicationTestMatrixCore.h"
#include "UnitTests/Helpers/GorgeousNetworkEmulationHelpers.h"
#include "Misc/Parse.h"

#if WITH_DEV_AUTOMATION_TESTS

static FGorgeousAutomationScenarioDescriptor MakeAutoReplication_RunAll()
{
	FGorgeousAutomationScenarioDescriptor D;
	D.ScenarioName = TEXT("AutoReplication.RunAll");
	D.DisplayName = TEXT("AutoReplication: Run Entire Suite");
	D.Description = TEXT("Executes every other registered AutoReplication scenario sequentially.");
	D.Tags = { TEXT("suite"), TEXT("runall") };
	D.Priority = 200;
	D.bEnabledByDefault = true;
	D.Runner = [](const FGorgeousAutomationScenarioContext& Context) -> FGorgeousAutomationScenarioResult
	{
		FGorgeousAutomationScenarioResult Result;

		int32 RequestedNetProfile = INDEX_NONE;
		if (!Context.ParameterString.IsEmpty())
		{
			FParse::Value(*Context.ParameterString, TEXT("NetProfile="), RequestedNetProfile);
		}

		FGorgeousNetworkEmulation::SetSuiteDefaultPreset(RequestedNetProfile);
		const FGorgeousNetworkEmulation::FRuntimeProfile SuiteRuntime = FGorgeousNetworkEmulation::EnsureSuitePresetApplied(&Result);
		FGorgeousNetworkEmulation::AppendRuntimeMetrics(SuiteRuntime, Result, TEXT("suite.netemu."));

		const TArray<FGorgeousAutomationScenarioDescriptor> Registered = FGorgeousAutomationTestMatrix::GetRegisteredScenarios();
		int32 ExecutedCount = 0;
		int32 FailureCount = 0;

		for (const FGorgeousAutomationScenarioDescriptor& Descriptor : Registered)
		{
			if (Descriptor.ScenarioName == Context.Descriptor.ScenarioName)
			{
				continue; // Skip the run-all scenario itself to avoid recursion
			}

			FGorgeousAutomationScenarioContext ChildContext(Context.Request, Context.ParameterString, Context.VariantIndex, Context.Test, Descriptor);
			Context.AddInfo(FString::Printf(TEXT("[RunAll] >>> %s"), *ChildContext.BuildScenarioLabel()));

			const FGorgeousAutomationScenarioResult ChildResult = FGorgeousAutomationTestMatrix::ExecuteScenario(Descriptor, ChildContext);
			++ExecutedCount;

			const FString SummaryLine = FString::Printf(TEXT("%s -> %s (Errors=%d | Warnings=%d | Notes=%d | Metrics=%d)"),
				*ChildContext.BuildScenarioLabel(),
				ChildResult.bSuccess ? TEXT("PASS") : TEXT("FAIL"),
				ChildResult.Errors.Num(),
				ChildResult.Warnings.Num(),
				ChildResult.Notes.Num(),
				ChildResult.Metrics.Num());
			Result.AddNote(SummaryLine);

			if (!Descriptor.bEnabledByDefault)
			{
				Result.AddWarning(FString::Printf(TEXT("%s was disabled by default but executed via RunAll."), *ChildContext.BuildScenarioLabel()));
			}

			if (!ChildResult.bSuccess)
			{
				++FailureCount;
				Result.AddError(FString::Printf(TEXT("%s failed. Inspect its output for details."), *ChildContext.BuildScenarioLabel()));
			}
		}

		if (ExecutedCount == 0)
		{
			Result.AddWarning(TEXT("RunAll did not find any peer scenarios to execute."));
		}
		else if (FailureCount == 0)
		{
			Result.AddNote(FString::Printf(TEXT("All %d child scenarios succeeded."), ExecutedCount));
		}
		else
		{
			Result.AddWarning(FString::Printf(TEXT("%d/%d child scenarios failed."), FailureCount, ExecutedCount));
		}

		FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(Context.Descriptor, Result, TEXT("suite"));
		return Result;
	};
	return D;
}

REGISTER_GORGEOUS_AUTOMATION_SCENARIO(MakeAutoReplication_RunAll());

#endif // WITH_DEV_AUTOMATION_TESTS
