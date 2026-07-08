// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "UnitTests/Helpers/GorgeousSignalBridgeInsightHelper.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"

void UGorgeousSignalBridgeInsightHelper::HandleTestSignal(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	// Simulate heavy calculation: Calculate primes up to a given number
	int32 PrimeCount = 0;
	const int32 MaxNum = 500; // Large enough to be heavy but not cause a timeout for 10000 iterations
	
	for (int32 Num = 2; Num <= MaxNum; ++Num)
	{
		bool bIsPrime = true;
		for (int32 i = 2; i * i <= Num; ++i)
		{
			if (Num % i == 0)
			{
				bIsPrime = false;
				break;
			}
		}
		if (bIsPrime)
		{
			PrimeCount++;
		}
	}
	HitCount += PrimeCount;

	// Random chance (e.g., 20%) to dispatch a result back
	if (BridgeRef && FMath::FRand() < 0.20f)
	{
		FGorgeousTestSignalPayload ResultPayload;
		ResultPayload.PrimeResult = PrimeCount;
		BridgeRef->Dispatch(TAG_Gorgeous_Test_SignalBridge_PerfResult, FInstancedStruct::Make(ResultPayload));
	}
}

void UGorgeousSignalBridgeInsightHelper::HandleTestResult(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	// Log the actual successful reception!
	ResultsDispatched++;
	
	// Evaluate the returned payload
	if (const FGorgeousTestSignalPayload* ResultPayload = Payload.GetPtr<FGorgeousTestSignalPayload>())
	{
		TotalVerifiedCalculations += ResultPayload->PrimeResult;
	}
}
