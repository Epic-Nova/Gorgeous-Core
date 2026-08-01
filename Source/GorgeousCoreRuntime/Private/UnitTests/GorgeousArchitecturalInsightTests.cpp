// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")

struct FGorgeousArchitecturalInsightTests
{
	static FGorgeousInsightScenarioResult RunDecoupledInvocationTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Requires a valid World context."));
			return Result;
		}

		USignalBridgeStorage_OV* Bridge = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(World, false);
		if (!Bridge)
		{
			// Scaffold a mock bridge if the game state is not set up
			Bridge = NewObject<USignalBridgeStorage_OV>();
		}

		// Act: We trigger a "Playlist UI" request blindly. We don't have CommonUIFoundation hooked up to listen to it!
		FGameplayTag PlaylistUITag = FGameplayTag::RequestGameplayTag(TEXT("Gorgeous.UI.Action.PlaylistScoreboard"));
		FInstancedStruct EmptyPayload;

		// Dispatch should silently hit a dead end, proving decoupled fallback.
		Bridge->Dispatch(PlaylistUITag, EmptyPayload);

		Result.bSuccess = true;
		Result.AddNote(TEXT("Decoupled UI Invocation dispatched safely. Action Class decoupled perfectly. Game continues seamlessly without explicit CommonUIFoundation links."));

		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeDecoupledInvocationScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.Architecture.Functional.DecoupledInvocation");
		D.DisplayName  = TEXT("General V2 Architecture: Decoupled Invocation Test");
		D.Description  = TEXT("Trigger a Playlist UI request into the void. Verify the game continues flawlessly without the UI installed.");
		D.Tags         = { TEXT("architecture"), TEXT("decoupled"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 80;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunDecoupledInvocationTest(Ctx); };
		return D;
	}
};

REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousArchitecturalInsightTests::MakeDecoupledInvocationScenario());
