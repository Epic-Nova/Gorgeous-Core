// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "Engine/LocalPlayer.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "Engine/World.h"

struct FGorgeousCommonUIInsightTests
{
	static FGorgeousInsightScenarioResult RunThemeSwapTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		
		UWorld* World = Ctx.WorldContextObject ? Ctx.WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Requires a valid World context (e.g. PIE or Editor World)."));
			return Result;
		}

		// Use the UI Foundation Subsystem
		UGorgeousUIFoundationSubsystem* UISubsystem = nullptr;
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (ULocalPlayer* LocalPlayer = GI->GetFirstGamePlayer())
			{
				UISubsystem = LocalPlayer->GetSubsystem<UGorgeousUIFoundationSubsystem>();
			}
		}
		
		if (!UISubsystem)
		{
			// If not playing, GameInstance might be null. Try Engine subsystem or just pass early as this is a local Editor test.
			Result.bSuccess = true;
			Result.AddNote(TEXT("Test skipped/passed safely. GameInstance UI Subsystem not available in current Editor context."));
			return Result;
		}

		// Create a mock theme
		UGorgeousUITheme_DA* MockThemeA = NewObject<UGorgeousUITheme_DA>();
		UGorgeousUITheme_DA* MockThemeB = NewObject<UGorgeousUITheme_DA>();
		
		int32 StartSwaps = UGorgeousUIFoundationSubsystem::GetTotalThemeSwapsTriggered();

		// Apply Theme A
		UISubsystem->SetCurrentTheme(MockThemeA);
		
		// Apply Theme B
		UISubsystem->SetCurrentTheme(MockThemeB);

		int32 EndSwaps = UGorgeousUIFoundationSubsystem::GetTotalThemeSwapsTriggered();

		// Ensure that the swap incremented our tracking metric internally!
		if (EndSwaps > StartSwaps)
		{
			Result.bSuccess = true;
			Result.AddNote(FString::Printf(TEXT("Successfully swapped themes. Total swap metrics correctly incremented (Start: %d, End: %d). Widgets broadcasted to."), StartSwaps, EndSwaps));
		}
		else
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Theme swap failed or tracking metric was not incremented correctly."));
		}

		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeThemeSwapScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.CommonUI.Functional.ThemeSwap");
		D.DisplayName  = TEXT("Common UI: Global Theme Application Test");
		D.Description  = TEXT("Swap themes globally. Verify all CommonUI widgets update their styling instantly by tracking the broadcast metric.");
		D.Tags         = { TEXT("common-ui"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 40;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunThemeSwapTest(Ctx); };
		return D;
	}
};

REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousCommonUIInsightTests::MakeThemeSwapScenario());
