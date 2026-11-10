#include "InsightMatrix/GorgeousInsightHarness.h"
#include "SharedTests/GorgeousObjectVariablePerfTestTypes.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "Engine/NetDriver.h"
#include "Engine/NetConnection.h"
#include "UObject/StrongObjectPtr.h"
#include "Misc/ScopeExit.h"

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
		Variable->RootNetworkConfig.ReplicationChannel = FName(*FString::Printf(TEXT("GauntletPolicy_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
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
			Result.AddError(TEXT("Gauntlet is required for this scenario."));
			return Result;
		}

		UWorld* World = ResolveWorld(Context);
		if (!World)
		{
			Result.AddError(TEXT("No world context available."));
			return Result;
		}

		UNetDriver* Driver = World->GetNetDriver();
		TArray<AGorgeousPlayerController*> Controllers;
		if (Driver && World->HasBegunPlay() && World->GetNetMode() != NM_Client)
			for (UNetConnection* Connection : Driver->ClientConnections)
				if (Connection && Connection->GetConnectionState() == USOCK_Open)
					if (auto* PC = Cast<AGorgeousPlayerController>(Connection->PlayerController)) Controllers.AddUnique(PC);
		if (Controllers.Num() < 2)
		{
			Result.AddError(TEXT("An authoritative begun world with two actually connected GorgeousPlayerControllers is required."));
			return Result;
		}
		AGorgeousPlayerController* Controller = Controllers[0];
		AGorgeousPlayerController* Requester = Controllers[1];
		TStrongObjectPtr<UGorgeousPerfAccessPolicyObjectVariable> KeepVariable(CreateAccessPolicyVariable(Controller, Label ? Label : TEXT("AccessPolicy")));
		UGorgeousPerfAccessPolicyObjectVariable* Variable = KeepVariable.Get();
		if (!Variable)
		{
			Result.AddError(TEXT("Failed to allocate access policy variable."));
			return Result;
		}

		auto* Stack = World->GetSubsystem<UGorgeousRootNetworkStackSubsystem>();
		if (!Stack) { Result.AddError(TEXT("Root network stack subsystem is unavailable.")); return Result; }
		const FName Channel = Variable->RootNetworkConfig.ReplicationChannel;
		Stack->RegisterVariable(Variable);
		Stack->SubscribeControllerToChannel(Requester, Channel);
		ON_SCOPE_EXIT { Stack->UnsubscribeControllerFromChannel(Requester, Channel); Stack->UnregisterVariable(Variable); };
		Variable->EnableCustomAccessPolicy(Requester);
		const int32 Repetitions = FCString::Strcmp(Label, TEXT("Timing")) == 0 ? 8 : 1;
		for (int32 Index = 0; Index < Repetitions; ++Index)
		{
			Variable->SetCustomAccessAllowed(true);
			if (!Stack->CanControllerAccess(Variable, Requester)) Result.AddError(TEXT("Current custom policy must immediately allow the connected nonowner."));
			Variable->SetCustomAccessAllowed(false);
			if (Stack->CanControllerAccess(Variable, Requester)) Result.AddError(TEXT("Changed custom policy must immediately deny the connected nonowner."));
		}
		if (Variable->GetAccessPolicyAllowCount() < Repetitions || Variable->GetAccessPolicyDenyCount() < Repetitions)
			Result.AddError(TEXT("Actual custom access-policy callbacks were not evaluated for both decisions."));

		Result.AddNote(FString::Printf(TEXT("Gauntlet role=%s | Controller=%s | Scenario=%s"),
			IsRunningDedicatedServer() ? TEXT("Server") : TEXT("Client"),
			*Controller->GetName(),
			Label ? Label : TEXT("AccessPolicy")));
		Result.AddMetric(TEXT("accesspolicy.allowedEvaluations"), Variable->GetAccessPolicyAllowCount());
		Result.AddMetric(TEXT("accesspolicy.deniedEvaluations"), Variable->GetAccessPolicyDenyCount());
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
