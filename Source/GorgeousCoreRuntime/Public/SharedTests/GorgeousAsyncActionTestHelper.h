// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "UObject/Object.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousAsyncActionTestHelper.generated.h"
//<-------------------------------------------------------------------------->

//#if WITH_DEV_AUTOMATION_TESTS

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Async Action Test Helper
| Functional Name: UGorgeousAsyncActionTestHelper
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Captures every delegate callback fired by
| UGorgeousAutoReplicationRPCRequestAsyncAction so that synchronous test
| code can inspect the delivery count & payload content.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/GorgeousAsyncActionTestHelper",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Examples/"
		)
)
class UGorgeousAsyncActionTestHelper : public UObject
{
	GENERATED_BODY()

public:

	// ── Counters ────────────────────────────────────────────────────────────

	/** How many times OnSingleResponderCompleted was broadcast. */
	int32 SingleResponderCallCount = 0;

	/** How many times OnCompleted was broadcast. */
	int32 CompletedCallCount = 0;

	/** How many times OnFailed was broadcast. */
	int32 FailedCallCount = 0;

	// ── Captured payloads ───────────────────────────────────────────────────

	/** Every per-responder payload in the order they arrived. */
	TArray<FGorgeousAutoReplicationRPCAsyncResult> ReceivedSingleResults;

	/** The last OnCompleted payload. */
	FGorgeousAutoReplicationRPCAsyncResult LastCompletedResult;

	/** The last OnFailed payload. */
	FGorgeousAutoReplicationRPCAsyncResult LastFailedResult;

	// ── Delegate handlers (UFUNCTION required for dynamic binding) ────────────

	UFUNCTION()
	void HandleSingleResponder(const FGorgeousAutoReplicationRPCAsyncResult& ResponderResult)
	{
		++SingleResponderCallCount;
		ReceivedSingleResults.Add(ResponderResult);
	}

	UFUNCTION()
	void HandleCompleted(const FGorgeousAutoReplicationRPCAsyncResult& Result)
	{
		++CompletedCallCount;
		LastCompletedResult = Result;
	}

	UFUNCTION()
	void HandleFailed(const FGorgeousAutoReplicationRPCAsyncResult& Result)
	{
		++FailedCallCount;
		LastFailedResult = Result;
	}

	// ── Helpers ─────────────────────────────────────────────────────────────

	/** Resets all counters and captured data. */
	void Reset()
	{
		SingleResponderCallCount = 0;
		CompletedCallCount = 0;
		FailedCallCount = 0;
		ReceivedSingleResults.Reset();
		LastCompletedResult = FGorgeousAutoReplicationRPCAsyncResult();
		LastFailedResult = FGorgeousAutoReplicationRPCAsyncResult();
	}

	/**
	 * Binds all three delegate pins on the supplied async action. Call BEFORE Activate().
	 * Safe to call multiple times (previous bindings are NOT removed, use Reset() for data only).
	 */
	void BindTo(UGorgeousAutoReplicationRPCRequestAsyncAction* Action)
	{
		if (!Action) return;

		Action->OnSingleResponderCompleted.AddDynamic(this, &UGorgeousAsyncActionTestHelper::HandleSingleResponder);
		Action->OnCompleted.AddDynamic(this, &UGorgeousAsyncActionTestHelper::HandleCompleted);
		Action->OnFailed.AddDynamic(this, &UGorgeousAsyncActionTestHelper::HandleFailed);
	}

	/** Returns true if at least one per-responder callback reported bIsLastResult == true. */
	bool HasReceivedLastResponder() const
	{
		for (const FGorgeousAutoReplicationRPCAsyncResult& R : ReceivedSingleResults)
		{
			if (R.bIsLastResult) return true;
		}
		return false;
	}

	/** Returns the highest TotalReceivedResponders seen across per-responder callbacks. */
	int32 GetPeakReceivedCount() const
	{
		int32 Peak = 0;
		for (const FGorgeousAutoReplicationRPCAsyncResult& R : ReceivedSingleResults)
		{
			Peak = FMath::Max(Peak, R.TotalReceivedResponders);
		}
		return Peak;
	}
};

//#endif // WITH_DEV_AUTOMATION_TESTS