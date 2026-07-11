#include "InsightMatrix/GorgeousInsightHarness.h"
#include "SharedTests/GorgeousObjectVariablePerfTestTypes.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousAutomationAccessPolicyGauntlet
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
		Variable->SetDisplayName(FString(DebugLabel));
		return Variable;
	}

	static FGorgeousInsightScenarioResult RunGauntletAccessPolicy(const FGorgeousInsightScenarioContext& Context, const TCHAR* Label)
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

		UGorgeousPerfAccessPolicyObjectVariable* Variable = CreateAccessPolicyVariable(Controller, Label ? Label : TEXT("AccessPolicy"));
		if (!Variable)
		{
			Result.AddError(TEXT("Failed to allocate access policy variable."));
			return Result;
		}

		FGorgeousObjectVariableEntry& Entry = Controller->AdditionalGorgeousData.FindOrAdd(PlayerControllerEntryKey);
		Entry.DefaultValue = Variable;
		Entry.bReplicate = true;
		Entry.Handle.CacheValue(Variable);
		Controller->GetAutoReplicationMixin().TrySetReplicatedValue(PlayerControllerEntryKey, Variable);
		Controller->ForceNetUpdate();
		Variable->EnableCustomAccessPolicy(Controller);
		Variable->SetCustomAccessAllowed(true);
		Variable->InjectReplicationStimulus(static_cast<float>(FPlatformTime::Seconds()), FMath::FRand());

		Result.AddNote(FString::Printf(TEXT("Gauntlet role=%s | Controller=%s | Scenario=%s"),
			IsRunningDedicatedServer() ? TEXT("Server") : TEXT("Client"),
			*Controller->GetName(),
			Label ? Label : TEXT("AccessPolicy")));
		Result.AddMetric(TEXT("accesspolicy.stimulusCount"), static_cast<double>(Variable->GetPerfReplicationStimulusCount()));
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeAccessPolicy_AllowDeny()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.AccessPolicy.AllowDeny");
		D.DisplayName = TEXT("Access Policy (Gauntlet) Allow/Deny");
		D.Description = TEXT("Gauntlet-only access policy sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("accesspolicy") };
		D.Priority = 80;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletAccessPolicy(Context, TEXT("AllowDeny"));
		};
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakeAccessPolicy_Timing()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.AccessPolicy.Timing");
		D.DisplayName = TEXT("Access Policy (Gauntlet) Timing");
		D.Description = TEXT("Gauntlet-only timing sanity check (local world)." );
		D.Tags = { TEXT("gauntlet"), TEXT("accesspolicy"), TEXT("timing") };
		D.Priority = 75;
		D.Runner = [](const FGorgeousInsightScenarioContext& Context)
		{
			return RunGauntletAccessPolicy(Context, TEXT("Timing"));
		};
		return D;
	}
}

REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationAccessPolicyGauntlet::MakeAccessPolicy_AllowDeny());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousAutomationAccessPolicyGauntlet::MakeAccessPolicy_Timing());

#endif // WITH_DEV_AUTOMATION_TESTS
