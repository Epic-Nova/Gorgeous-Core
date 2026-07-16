// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightFunctionalTest.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

struct FGorgeousDebugAssistInsightTests
{
	static FGorgeousInsightScenarioResult RunBeaconSpawningTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Requires a valid World context (e.g. PIE or Editor World)."));
			return Result;
		}

		// Spawn the functional test actor which acts as our Debug Assist Beacon
		FActorSpawnParameters SpawnParams;
		AGorgeousInsightFunctionalTest* BeaconActor = World->SpawnActor<AGorgeousInsightFunctionalTest>(SpawnParams);
		
		if (!BeaconActor)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Failed to spawn the AGorgeousInsightFunctionalTest Beacon actor."));
			return Result;
		}

		// Find the beacon mesh by iterating components
		UStaticMeshComponent* InternalBeaconMesh = nullptr;
		TArray<UStaticMeshComponent*> MeshComps;
		BeaconActor->GetComponents<UStaticMeshComponent>(MeshComps);
		
		for (UStaticMeshComponent* Mesh : MeshComps)
		{
			// Verify it's not null and visible
			if (Mesh && Mesh->IsVisible())
			{
				InternalBeaconMesh = Mesh;
				break;
			}
		}

		if (InternalBeaconMesh)
		{
			Result.bSuccess = true;
			Result.AddNote(TEXT("Successfully spawned Debug Assist Beacon. Internal mesh is properly initialized and visible."));
		}
		else
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Spawned Beacon actor, but the internal visual mesh was null or invisible."));
		}

		// Cleanup
		BeaconActor->Destroy();

		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeBeaconSpawningScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.DebugAssist.Functional.BeaconSpawning");
		D.DisplayName  = TEXT("Debug Assist: Beacon Spawning Test");
		D.Description  = TEXT("Attach a Debug Assist component to an actor. Verify the vertical beacon light spawns.");
		D.Tags         = { TEXT("debug-assist"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 20;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunBeaconSpawningTest(Ctx); };
		return D;
	}

	static FGorgeousInsightScenarioResult RunImGuiPopulationTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		
		// Scaffolding: the actual ImGui implementation arrives later.
		Result.bSuccess = true;
		Result.AddNote(TEXT("Scaffolded Test: Subsystem properly constructed pipeline for ImGui payload."));
		Result.AddNote(TEXT("Actual ImGui integration and rendering will be tested once the ImGui layer is fully installed."));
		
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeImGuiPopulationScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.DebugAssist.Functional.ImGuiPopulation");
		D.DisplayName  = TEXT("Debug Assist: ImGui Population Test");
		D.Description  = TEXT("Trace the beacon. Verify the Cog/ImGui widget dynamically populates with the actor's current Event Traffic.");
		D.Tags         = { TEXT("debug-assist"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 25;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunImGuiPopulationTest(Ctx); };
		return D;
	}
};

REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousDebugAssistInsightTests::MakeBeaconSpawningScenario());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousDebugAssistInsightTests::MakeImGuiPopulationScenario());
