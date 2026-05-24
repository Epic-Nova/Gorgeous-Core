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

#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"

UGorgeousRPC_OV::UGorgeousRPC_OV()
	: CachedResults()
	, CachedResultMap()
	, bHasCachedResult(false)
{
}

void UGorgeousRPC_OV::CaptureResult(const FGorgeousAutoReplicationRPCResult& InResult)
{
	CachedResults.Reset();
	CachedResults.Add(InResult);
	bHasCachedResult = true;
	CachedResultMap.Reset();
	const FString ResponderKey = InResult.Responder.IsValid() ? InResult.Responder.GetStableKey() : TEXT("Server");
	CachedResultMap.Add(ResponderKey, InResult);
}

void UGorgeousRPC_OV::CaptureResultSet(const TMap<FString, FGorgeousAutoReplicationRPCResult>& InResults, const TArray<FGorgeousAutoReplicationRPCResult>& OrderedResults)
{
	CachedResultMap = InResults;
	CachedResults = OrderedResults;
	bHasCachedResult = CachedResults.Num() > 0;
	if (bHasCachedResult)
	{
		const int32 OwnerIndex = CachedResults.IndexOfByPredicate([](const FGorgeousAutoReplicationRPCResult& Result)
		{
			return Result.TargetOwner != nullptr;
		});
		if (OwnerIndex > 0)
		{
			CachedResults.Swap(0, OwnerIndex);
		}
	}
}

void UGorgeousRPC_OV::ResetResult()
{
	CachedResults.Reset();
	CachedResultMap.Reset();
	bHasCachedResult = false;
}

bool UGorgeousRPC_OV::GetArgumentContainerByName(const FName ArgumentName, FGorgeousRPCArgumentContainer& OutContainer) const
{
	if (!bHasCachedResult || ArgumentName.IsNone())
	{
		return false;
	}
	
	const FGorgeousAutoReplicationRPCResult& PrimaryResult = GetCachedResult();
	const FGorgeousRPCArgumentContainer* FoundArgument = PrimaryResult.QueuedRPC.Payload.Arguments.FindByPredicate(
		[&](const FGorgeousRPCArgumentContainer& ArgumentContainer)
		{
			return ArgumentContainer.ArgumentName == ArgumentName;
		}
	);
	if (FoundArgument)
	{
		OutContainer = *FoundArgument;
		return true;
	}

	return false;
}

bool UGorgeousRPC_OV::GetResultByConnectionKey(const FString& ConnectionKey, FGorgeousAutoReplicationRPCResult& OutResult) const
{
	if (const FGorgeousAutoReplicationRPCResult* FoundResult = CachedResultMap.Find(ConnectionKey))
	{
		OutResult = *FoundResult;
		return true;
	}

	return false;
}

const FGorgeousAutoReplicationRPCResult& UGorgeousRPC_OV::GetCachedResult() const
{
	return HasResult() ? CachedResults[0] : GetEmptyResult();
}

const FGorgeousAutoReplicationRPCResult& UGorgeousRPC_OV::GetEmptyResult()
{
	static const FGorgeousAutoReplicationRPCResult EmptyResult;
	return EmptyResult;
}
