#include "CoreMinimal.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "GorgeousObjectVariablePerfTestTypes.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationObjectVariablesGauntlet
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

	static UGorgeousPerfObjectVariable* CreatePerfVariable(AGorgeousPlayerController* Owner, const TCHAR* DebugLabel, EGorgeousObjectVariableReplicationMode Mode)
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
		Variable->RootNetworkConfig.bExposeThroughRootNetworkStack = true;
		Variable->RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
		Variable->RootNetworkConfig.ReplicationChannel = PlayerControllerEntryKey;
		Variable->bUseSharedNetworkStack = true;
		Variable->EnsureSharedNetworkStackOwner(Owner);
		Variable->SetDisplayName(FString(DebugLabel));
		return Variable;
	}

	static FGorgeousInsightScenarioResult RunGauntletObjectVariables(const FGorgeousInsightScenarioContext& Context, const TCHAR* Label, EGorgeousObjectVariableReplicationMode Mode)
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

		UGorgeousPerfObjectVariable* Variable = CreatePerfVariable(Controller, Label ? Label : TEXT("PerfVar"), Mode);
		if (!Variable)
		{
			Result.AddError(TEXT("Failed to allocate perf object variable."));
			return Result;
		}

		FGorgeousObjectVariableEntry& Entry = Controller->AdditionalGorgeousData.FindOrAdd(PlayerControllerEntryKey);
		Entry.DefaultValue = Variable;
		Entry.bReplicate = true;
		Entry.Handle.CacheValue(Variable);
		Controller->GetAutoReplicationMixin().TrySetReplicatedValue(PlayerControllerEntryKey, Variable);
		Controller->ForceNetUpdate();
		Variable->InjectReplicationStimulus(static_cast<float>(FPlatformTime::Seconds()), FMath::FRand());

		Result.AddNote(FString::Printf(TEXT("Gauntlet role=%s | Controller=%s | Scenario=%s"),
			IsRunningDedicatedServer() ? TEXT("Server") : TEXT("Client"),
			*Controller->GetName(),
			Label ? Label : TEXT("ObjectVariables")));
		Result.AddMetric(TEXT("objectvariables.stimulusCount"), static_cast<double>(Variable->GetPerfReplicationStimulusCount()));
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeObjectVariables_FullAuto()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.ObjectVariables.FullAuto");
		D.DisplayName = TEXT("ObjectVariables (Gauntlet) Full Auto");
		D.Description = TEXT("Gauntlet-only object variable sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("objectvariables"), TEXT("auto") };
		D.Priority = 80;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletObjectVariables(Context, TEXT("FullAuto"), EGorgeousObjectVariableReplicationMode::EFullAutoReplication);
		};
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakeObjectVariables_Hybrid()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.ObjectVariables.Hybrid");
		D.DisplayName = TEXT("ObjectVariables (Gauntlet) Hybrid");
		D.Description = TEXT("Gauntlet-only object variable sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("objectvariables"), TEXT("hybrid") };
		D.Priority = 75;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletObjectVariables(Context, TEXT("Hybrid"), EGorgeousObjectVariableReplicationMode::EHybrid);
		};
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakeObjectVariables_Manual()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.ObjectVariables.Manual");
		D.DisplayName = TEXT("ObjectVariables (Gauntlet) Manual");
		D.Description = TEXT("Gauntlet-only object variable sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("objectvariables"), TEXT("manual") };
		D.Priority = 70;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletObjectVariables(Context, TEXT("Manual"), EGorgeousObjectVariableReplicationMode::EManual);
		};
		return D;
	}
}

REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationObjectVariablesGauntlet::MakeObjectVariables_FullAuto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationObjectVariablesGauntlet::MakeObjectVariables_Hybrid());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationObjectVariablesGauntlet::MakeObjectVariables_Manual());

#endif // WITH_DEV_AUTOMATION_TESTS
