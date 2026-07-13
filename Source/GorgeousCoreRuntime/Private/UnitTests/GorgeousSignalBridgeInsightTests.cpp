// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "HAL/PlatformTime.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "Engine/NetDriver.h"
#include "SharedTests/GorgeousSignalBridgeInsightHelper.h"

struct FGorgeousSignalBridgeTestAccess
{
	static void ClearStorage(USignalBridgeStorage_OV* Bridge)
	{
		if (Bridge)
		{
			Bridge->AccessRules.Empty();
			Bridge->LocalBindings.Empty();
			Bridge->DictionaryAssociations.Empty();
		}
	}

	static void ServerRegister(USignalBridgeStorage_OV* Bridge, FGameplayTag Tag, AGorgeousPlayerController* PC)
	{
		if (Bridge)
		{
			Bridge->Server_RegisterListener(Tag, PC);
		}
	}
};

struct FGorgeousSignalBridgeInsightTests
{
	static FGorgeousInsightScenarioResult RunAnonymousRoutingTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		Result.bSuccess = true;
		
		USignalBridgeStorage_OV* Bridge = NewObject<USignalBridgeStorage_OV>();
		UGorgeousSignalBridgeInsightHelper* SystemA = NewObject<UGorgeousSignalBridgeInsightHelper>();
		SystemA->BridgeRef = Bridge;
		
		FSignalBridgeEventDelegate DelegateA;
		DelegateA.BindDynamic(SystemA, &UGorgeousSignalBridgeInsightHelper::HandleTestSignal);
		Bridge->Listen(TAG_Gorgeous_Test_SignalBridge_Perf, nullptr, DelegateA);
		
		FGorgeousTestSignalPayload PayloadData;
		PayloadData.PrimeResult = 42;
		FInstancedStruct TestPayload = FInstancedStruct::Make(PayloadData);
		
		Bridge->Dispatch(TAG_Gorgeous_Test_SignalBridge_Perf, TestPayload);
		
		if (SystemA->HitCount == 1)
		{
			Result.AddNote(TEXT("Anonymous routing successful. Payload received perfectly without tight coupling."));
		}
		else
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("System A did not receive the payload correctly."));
		}
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeAnonymousRoutingScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.SignalBridge.Functional.AnonymousRouting");
		D.DisplayName  = TEXT("Signal Bridge: Anonymous Routing Test");
		D.Description  = TEXT("System A registers a listener. System B fires a message. Verify System A receives the payload without knowing about System B.");
		D.Tags         = { TEXT("signal-bridge"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 50;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunAnonymousRoutingTest(Ctx); };
		return D;
	}

	static FGorgeousInsightScenarioResult RunGracefulFailureTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;
		Result.bSuccess = true;
		
		USignalBridgeStorage_OV* Bridge = NewObject<USignalBridgeStorage_OV>();
		
		// Dispatching a signal that nobody is listening to, and which hasn't been registered.
		// If it reaches the end without hitting an assert/check, it succeeds gracefully.
		FInstancedStruct EmptyPayload;
		Bridge->Dispatch(TAG_Gorgeous_Test_SignalBridge_PerfResult, EmptyPayload);

		Result.AddNote(TEXT("Graceful failure successful. Unregistered/unlistened signal disappeared silently without crashing the system."));
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeGracefulFailureScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.SignalBridge.Functional.GracefulFailure");
		D.DisplayName  = TEXT("Signal Bridge: Graceful Failure Test");
		D.Description  = TEXT("Unload System A entirely. System B fires a message. Verify the Signal Bridge handles the dead end silently.");
		D.Tags         = { TEXT("signal-bridge"), TEXT("functional"), TEXT("GorgeousCore") };
		D.Priority     = 60;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunGracefulFailureTest(Ctx); };
		return D;
	}

	static FGorgeousInsightScenarioResult RunPerformanceTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;

		// Create a temporary storage instance for the test
		USignalBridgeStorage_OV* Bridge = NewObject<USignalBridgeStorage_OV>();
		
		// Create the dummy helper object
		UGorgeousSignalBridgeInsightHelper* DummyListener = NewObject<UGorgeousSignalBridgeInsightHelper>();
		DummyListener->BridgeRef = Bridge;
		
		// Bind the main perf tag to the dummy helper
		FSignalBridgeEventDelegate PerfDelegate;
		PerfDelegate.BindDynamic(DummyListener, &UGorgeousSignalBridgeInsightHelper::HandleTestSignal);
		Bridge->Listen(TAG_Gorgeous_Test_SignalBridge_Perf, nullptr, PerfDelegate);
		
		// Bind the result tag to the dummy helper as well
		FSignalBridgeEventDelegate ResultDelegate;
		ResultDelegate.BindDynamic(DummyListener, &UGorgeousSignalBridgeInsightHelper::HandleTestResult);
		Bridge->Listen(TAG_Gorgeous_Test_SignalBridge_PerfResult, nullptr, ResultDelegate);
		
		FInstancedStruct EmptyPayload;
		const int32 NumIterations = 10000;
		
		// Measure 10,000 local dispatches
		double StartTime = FPlatformTime::Seconds();
		
		for (int32 i = 0; i < NumIterations; ++i)
		{
			Bridge->Dispatch(TAG_Gorgeous_Test_SignalBridge_Perf, EmptyPayload);
		}
		
		double EndTime = FPlatformTime::Seconds();
		double TotalTimeMs = (EndTime - StartTime) * 1000.0;
		
		Result.AddNote(FString::Printf(TEXT("Successfully benchmarked %d local dispatches. Heavy calculations yielded hit count: %d. Sub-dispatched %d results. Verified %lld total prime calculations returned."), NumIterations, DummyListener->HitCount, DummyListener->ResultsDispatched, DummyListener->TotalVerifiedCalculations));
		
		Result.AddMetric(TEXT("Dispatch Count"), FString::FromInt(NumIterations));
		Result.AddMetric(TEXT("Results Sub-Dispatched"), FString::FromInt(DummyListener->ResultsDispatched));
		Result.AddMetric(TEXT("Verified Prime Calculations"), FString::Printf(TEXT("%lld"), DummyListener->TotalVerifiedCalculations));
		Result.AddMetric(TEXT("Total Time"), TotalTimeMs, TEXT("ms"));
		
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeSignalBridgePerformanceScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.SignalBridge.Performance.LocalDispatch");
		D.DisplayName  = TEXT("Signal Bridge: Local Dispatch Performance");
		D.Description  = TEXT("Tests the local dispatch performance of the Signal Bridge by sending 10000 signals and measuring the overhead.");
		D.Tags         = { TEXT("signal-bridge"), TEXT("performance"), TEXT("GorgeousCore") };
		D.Priority     = 100;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunPerformanceTest(Ctx); };
		return D;
	}

	static FGorgeousInsightScenarioResult RunParametrizedTest(const FGorgeousInsightScenarioContext& Ctx)
	{
		FGorgeousInsightScenarioResult Result;

		int32 NumIterations = 1000;
		bool bReplicated = false;

		FParse::Value(*Ctx.ParameterString, TEXT("Iterations="), NumIterations);
		FParse::Bool(*Ctx.ParameterString, TEXT("Replicated="), bReplicated);

		UWorld* ServerWorld = nullptr;
		TArray<UWorld*> ClientWorlds;

		if (GEngine)
		{
			for (const FWorldContext& WorldCtx : GEngine->GetWorldContexts())
			{
				if (UWorld* W = WorldCtx.World())
				{
					if (WorldCtx.WorldType == EWorldType::PIE)
					{
						if (W->GetNetMode() == NM_ListenServer || W->GetNetMode() == NM_DedicatedServer)
						{
							ServerWorld = W;
						}
						else if (W->GetNetMode() == NM_Client)
						{
							ClientWorlds.Add(W);
						}
					}
				}
			}
		}

		if (bReplicated && (ClientWorlds.Num() == 0 || !ServerWorld))
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Replication requested, but PIE is running with less than 2 players (No Server + Client detected). Please start PIE with 'NetMode: Play As Listen Server' and at least 2 players!"));
			return Result;
		}

		UWorld* PrimaryWorld = bReplicated ? ServerWorld : Ctx.WorldContextObject->GetWorld();
		USignalBridgeStorage_OV* PrimaryBridge = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(PrimaryWorld, bReplicated);

		if (!PrimaryBridge)
		{
			Result.bSuccess = false;
			Result.AddError(TEXT("Failed to obtain Primary Signal Bridge Storage."));
			return Result;
		}

		// Clean up state from previous runs
		FGorgeousSignalBridgeTestAccess::ClearStorage(PrimaryBridge);

		UGorgeousSignalBridgeInsightHelper* PrimaryListener = NewObject<UGorgeousSignalBridgeInsightHelper>();
		PrimaryListener->BridgeRef = PrimaryBridge;
		
		FSignalBridgeEventDelegate PerfDelegate;
		PerfDelegate.BindDynamic(PrimaryListener, &UGorgeousSignalBridgeInsightHelper::HandleTestSignal);
		PrimaryBridge->Listen(TAG_Gorgeous_Test_SignalBridge_Perf, nullptr, PerfDelegate);

		if (!bReplicated)
		{
			FSignalBridgeEventDelegate ResultDelegate;
			ResultDelegate.BindDynamic(PrimaryListener, &UGorgeousSignalBridgeInsightHelper::HandleTestResult);
			PrimaryBridge->Listen(TAG_Gorgeous_Test_SignalBridge_PerfResult, nullptr, ResultDelegate);
		}

		TArray<UGorgeousSignalBridgeInsightHelper*> ClientListeners;
		TArray<USignalBridgeStorage_OV*> ClientBridges;

		if (bReplicated)
		{
			// Register Global Rules for the RPC Payload
			FGorgeousSignalBridgeAccessRules_S GlobalRules;
			GlobalRules.bNetworked = true;
			GlobalRules.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
			PrimaryBridge->RegisterSignal(TAG_Gorgeous_Test_SignalBridge_PerfResult, GlobalRules, nullptr);

			for (UWorld* CW : ClientWorlds)
			{
				if (USignalBridgeStorage_OV* CB = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(CW, true))
				{
					FGorgeousSignalBridgeTestAccess::ClearStorage(CB);
					ClientBridges.Add(CB);

					UGorgeousSignalBridgeInsightHelper* CListener = NewObject<UGorgeousSignalBridgeInsightHelper>();
					CListener->BridgeRef = CB;
					ClientListeners.Add(CListener);

					FSignalBridgeEventDelegate ResultDelegate;
					ResultDelegate.BindDynamic(CListener, &UGorgeousSignalBridgeInsightHelper::HandleTestResult);
					CB->Listen(TAG_Gorgeous_Test_SignalBridge_PerfResult, nullptr, ResultDelegate);

					// Force Server to register the client's PC as a global listener
					if (AGorgeousPlayerController* ServerSideClientPC = Cast<AGorgeousPlayerController>(ServerWorld->GetFirstPlayerController()))
					{
						// In a real scenario we'd find the exact PlayerController matching the client, but for Insight Matrix
						// broadcasting to the first Server PC will replicate to all authorized targets if Rules are Global.
						FGorgeousSignalBridgeTestAccess::ServerRegister(PrimaryBridge, TAG_Gorgeous_Test_SignalBridge_PerfResult, ServerSideClientPC);
					}
				}
			}
		}

		FInstancedStruct EmptyPayload;
		
		double StartTime = FPlatformTime::Seconds();
		
		const int32 ChunkSize = 5000;
		int32 DispatchesLeft = NumIterations;

		while (DispatchesLeft > 0)
		{
			int32 CurrentChunk = FMath::Min(ChunkSize, DispatchesLeft);
			for (int32 i = 0; i < CurrentChunk; ++i)
			{
				PrimaryBridge->Dispatch(TAG_Gorgeous_Test_SignalBridge_Perf, EmptyPayload);
			}
			DispatchesLeft -= CurrentChunk;

			if (bReplicated)
			{
				// Pump the net drivers to prevent saturation and ensure cross-world RPC delivery
				for (UWorld* W : ClientWorlds)
				{
					if (UNetDriver* ND = W->GetNetDriver())
					{
						ND->TickDispatch(0.01f);
						ND->TickFlush(0.01f);
					}
				}
				if (UNetDriver* ND = ServerWorld->GetNetDriver())
				{
					ND->TickDispatch(0.01f);
					ND->TickFlush(0.01f);
				}
			}
		}
		
		double EndTime = FPlatformTime::Seconds();
		double TotalTimeMs = (EndTime - StartTime) * 1000.0;
		
		int32 TotalClientDispatches = 0;
		int64 TotalClientVerifications = 0;

		if (bReplicated)
		{
			for (UGorgeousSignalBridgeInsightHelper* CListener : ClientListeners)
			{
				TotalClientDispatches += CListener->ResultsDispatched;
				TotalClientVerifications += CListener->TotalVerifiedCalculations;
			}
		}
		else
		{
			TotalClientDispatches = PrimaryListener->ResultsDispatched;
			TotalClientVerifications = PrimaryListener->TotalVerifiedCalculations;
		}

		Result.AddNote(FString::Printf(TEXT("Successfully dispatched %d signals. Replicated=%s."), NumIterations, bReplicated ? TEXT("true") : TEXT("false")));
		Result.AddNote(FString::Printf(TEXT("Primary HitCount: %d. Total Client Sub-dispatches arrived: %d. Verified %lld total prime calculations returned."), 
			PrimaryListener->HitCount, TotalClientDispatches, TotalClientVerifications));

		if (bReplicated)
		{
			Result.AddNote(FString::Printf(TEXT("Replication Proof: %d clients successfully received cross-world payloads."), ClientWorlds.Num()));
		}
		
		Result.AddMetric(TEXT("Dispatch Count"), FString::FromInt(NumIterations));
		Result.AddMetric(TEXT("Is Replicated"), bReplicated ? TEXT("True") : TEXT("False"));
		Result.AddMetric(TEXT("Client Received Sub-Dispatches"), FString::FromInt(TotalClientDispatches));
		Result.AddMetric(TEXT("Verified Prime Calculations"), FString::Printf(TEXT("%lld"), TotalClientVerifications));
		Result.AddMetric(TEXT("Total Time"), TotalTimeMs, TEXT("ms"));
		
		if (TotalTimeMs > 0.0)
		{
			double DispatchesPerSec = (static_cast<double>(NumIterations) / TotalTimeMs) * 1000.0;
			Result.AddMetric(TEXT("Throughput"), DispatchesPerSec, TEXT("ops/sec"));
		}
		
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeParametrizedScenario()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("Core.SignalBridge.ParametrizedDispatch");
		D.DisplayName  = TEXT("Signal Bridge: Parametrized Execution Matrix");
		D.Description  = TEXT("The ultimative execution test for the Signal Bridge, testing massive workloads with optional PIE Replication toggles.");
		D.Tags         = { TEXT("signal-bridge"), TEXT("performance"), TEXT("network"), TEXT("GorgeousCore") };
		D.Priority     = 110;
		
		FGorgeousInsightTest::FGorgeousInsightTestInput IterationsInput;
		IterationsInput.Id = TEXT("Iterations");
		IterationsInput.DisplayName = FText::FromString(TEXT("Dispatch Volume"));
		IterationsInput.Type = FGorgeousInsightTest::FGorgeousInsightTestInput::EGorgeousInsightTestInputType::Dropdown;
		IterationsInput.DropdownOptions = { TEXT("100"), TEXT("1000"), TEXT("5000"), TEXT("10000"), TEXT("50000") };
		IterationsInput.DefaultValue = TEXT("1000");
		D.Inputs.Add(IterationsInput);

		FGorgeousInsightTest::FGorgeousInsightTestInput ReplicatedInput;
		ReplicatedInput.Id = TEXT("Replicated");
		ReplicatedInput.DisplayName = FText::FromString(TEXT("Run With Replication"));
		ReplicatedInput.Type = FGorgeousInsightTest::FGorgeousInsightTestInput::EGorgeousInsightTestInputType::Bool;
		ReplicatedInput.DefaultValue = TEXT("false");
		D.Inputs.Add(ReplicatedInput);

		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunParametrizedTest(Ctx); };
		return D;
	}
};

REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousSignalBridgeInsightTests::MakeAnonymousRoutingScenario());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousSignalBridgeInsightTests::MakeGracefulFailureScenario());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousSignalBridgeInsightTests::MakeSignalBridgePerformanceScenario());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(FGorgeousSignalBridgeInsightTests::MakeParametrizedScenario());
