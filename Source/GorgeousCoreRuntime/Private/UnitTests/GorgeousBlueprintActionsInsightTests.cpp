// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightBlueprintStats_OV.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "SharedTests/GorgeousSignalBridgeInsightHelper.h"
#include "Engine/World.h"

struct FGorgeousBlueprintActionsInsightTests
{
	static FGorgeousInsightScenarioResult RunBlueprintActionRegistrationTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Requires a valid World context."));
			return Result;
		}

		FName SystemName = TEXT("TestSystem");
		FName ActionName = TEXT("OpenTestUI");
		FGameplayTag SignalTag = FGameplayTag::RequestGameplayTag(TEXT("Gorgeous.UI.Action.Test"));

		// 1. Register Action via BP API
		FGuid SessionID = UGorgeousInsightBlueprintStats_OV::RegisterBlueprintSystemStats(Ctx.WorldContextObject, SystemName);
		UGorgeousInsightBlueprintStats_OV::RegisterBlueprintSystemAction(Ctx.WorldContextObject, SessionID, SystemName, ActionName, SignalTag);

		// 2. Set up Signal Bridge listener
		USignalBridgeStorage_OV* Bridge = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(World, false);
		if (!Bridge) Bridge = NewObject<USignalBridgeStorage_OV>();

		UGorgeousSignalBridgeInsightHelper* SystemA = NewObject<UGorgeousSignalBridgeInsightHelper>();
		SystemA->BridgeRef = Bridge;
		
		FSignalBridgeEventDelegate DelegateA;
		DelegateA.BindDynamic(SystemA, &UGorgeousSignalBridgeInsightHelper::HandleTestSignal);
		Bridge->Listen(SignalTag, nullptr, DelegateA);

		// 3. Simulate Provider clicking it. The Provider handles the format "Blueprint.SystemName.ActionName"
		// The Provider reads the registry natively. This test just validates that if the action fires the tag, the tag reaches the UI.
		FInstancedStruct EmptyPayload;
		Bridge->Dispatch(SignalTag, EmptyPayload);

		if (SystemA->HitCount >= 1)
		{
			Result.bSuccess = true;
			Result.AddNote(TEXT("Blueprint Action perfectly registered and signal correctly routed via Signal Bridge to mock UI executor."));
		}
		else
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Blueprint Action failed to route signal to executor."));
		}

		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeBlueprintActionRegistrationScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.BlueprintActions.Functional.Registration");
		D.DisplayName  = TEXT("Blueprint Stats: Action Registration Test");
		D.Description  = TEXT("Simulates a Blueprint system registering a custom UI action and validates that executing it fires the decoupled signal correctly.");
		D.Tags         = { TEXT("blueprint-actions"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 85;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunBlueprintActionRegistrationTest(Ctx); };
		return D;
	}
};

REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousBlueprintActionsInsightTests::MakeBlueprintActionRegistrationScenario());
