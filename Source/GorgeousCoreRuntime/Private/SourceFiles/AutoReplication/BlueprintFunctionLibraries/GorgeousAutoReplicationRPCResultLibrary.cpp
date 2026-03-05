// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCResultLibrary.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"

// ─────────────────────────────────────────────────────────────────────────────
// Internal helper: build a responder description string from a handle
// ─────────────────────────────────────────────────────────────────────────────
namespace
{
	FString DescribeResponder(const FGorgeousAutoReplicationRPCResponderHandle& Responder)
	{
		if (!Responder.IsValid())
		{
			return TEXT("<unknown>");
		}
		return Responder.GetStableKey();
	}
}

// =============================================================================
// FGorgeousAutoReplicationRPCResult
// =============================================================================

UGorgeousObjectVariable* UGorgeousAutoReplicationRPCResultLibrary::GetTargetVariable(const FGorgeousAutoReplicationRPCResult& Result)
{
	return Result.TargetVariable;
}

bool UGorgeousAutoReplicationRPCResultLibrary::HasTargetVariable(const FGorgeousAutoReplicationRPCResult& Result)
{
	return Result.TargetVariable != nullptr;
}

bool UGorgeousAutoReplicationRPCResultLibrary::IsServerResult(const FGorgeousAutoReplicationRPCResult& Result)
{
	return Result.Responder.IsValid() && Result.Responder.bIsServer;
}

FGorgeousAutoReplicationRPCResponderHandle UGorgeousAutoReplicationRPCResultLibrary::GetResponder(const FGorgeousAutoReplicationRPCResult& Result)
{
	return Result.Responder;
}

FString UGorgeousAutoReplicationRPCResultLibrary::GetResponderDescription(const FGorgeousAutoReplicationRPCResult& Result)
{
	return DescribeResponder(Result.Responder);
}

FName UGorgeousAutoReplicationRPCResultLibrary::GetHandlerName(const FGorgeousAutoReplicationRPCResult& Result)
{
	return Result.QueuedRPC.Payload.HandlerName;
}

FName UGorgeousAutoReplicationRPCResultLibrary::GetRPCKey(const FGorgeousAutoReplicationRPCResult& Result)
{
	return Result.QueuedRPC.Key;
}

FString UGorgeousAutoReplicationRPCResultLibrary::GetTargetKindInfo(const FGorgeousAutoReplicationRPCResult& Result)
{
	const UEnum* Enum = StaticEnum<EGorgeousAutoReplicationTargetKind>();
	if (!Enum)
	{
		return TEXT("<unknown>");
	}
	FString Raw = Enum->GetNameStringByValue(static_cast<int64>(Result.TargetKind));
	// Strip the leading 'E' prefix present in all enum value names (e.g. "EOwner" → "Owner")
	if (Raw.StartsWith(TEXT("E")))
	{
		Raw.RemoveAt(0, 1);
	}
	return Raw;
}

// =============================================================================
// FGorgeousAutoReplicationRPCAsyncResult
// =============================================================================

FGorgeousAutoReplicationRPCResult UGorgeousAutoReplicationRPCResultLibrary::GetPrimaryResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return AsyncResult.Result;
}

UGorgeousObjectVariable* UGorgeousAutoReplicationRPCResultLibrary::GetPrimaryTargetVariable(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return AsyncResult.Result.TargetVariable;
}

FGorgeousAutoReplicationRPCResponderHandle UGorgeousAutoReplicationRPCResultLibrary::GetPrimaryResponder(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return AsyncResult.Result.Responder;
}

bool UGorgeousAutoReplicationRPCResultLibrary::GetResultForResponder(
	const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult,
	const FString& ResponderKey,
	FGorgeousAutoReplicationRPCResult& OutResult)
{
	if (const FGorgeousAutoReplicationRPCResult* Found = AsyncResult.ResultMap.Find(ResponderKey))
	{
		OutResult = *Found;
		return true;
	}
	OutResult = FGorgeousAutoReplicationRPCResult{};
	return false;
}

bool UGorgeousAutoReplicationRPCResultLibrary::HasResultForResponder(
	const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult,
	const FString& ResponderKey)
{
	return AsyncResult.ResultMap.Contains(ResponderKey);
}

bool UGorgeousAutoReplicationRPCResultLibrary::GetResultForResponderHandle(
	const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult,
	const FGorgeousAutoReplicationRPCResponderHandle& ResponderHandle,
	FGorgeousAutoReplicationRPCResult& OutResult)
{
	return GetResultForResponder(AsyncResult, ResponderHandle.GetStableKey(), OutResult);
}

bool UGorgeousAutoReplicationRPCResultLibrary::HasResultForResponderHandle(
	const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult,
	const FGorgeousAutoReplicationRPCResponderHandle& ResponderHandle)
{
	return HasResultForResponder(AsyncResult, ResponderHandle.GetStableKey());
}

TArray<FGorgeousAutoReplicationRPCResult> UGorgeousAutoReplicationRPCResultLibrary::GetAllResults(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return AsyncResult.ResultSet;
}

TArray<FString> UGorgeousAutoReplicationRPCResultLibrary::GetAllResponderKeys(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	TArray<FString> Keys;
	AsyncResult.ResultMap.GenerateKeyArray(Keys);
	return Keys;
}

int32 UGorgeousAutoReplicationRPCResultLibrary::GetResultCount(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return AsyncResult.ResultSet.Num();
}

UGorgeousRPC_OV* UGorgeousAutoReplicationRPCResultLibrary::GetResultContainer(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return AsyncResult.ResultContainer;
}

bool UGorgeousAutoReplicationRPCResultLibrary::IsLastResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return AsyncResult.bIsLastResult;
}

FString UGorgeousAutoReplicationRPCResultLibrary::GetResponderProgress(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	if (AsyncResult.TotalExpectedResponders <= 0)
	{
		return FString::Printf(TEXT("%d/??"), AsyncResult.TotalReceivedResponders);
	}
	return FString::Printf(TEXT("%d/%d"),
		AsyncResult.TotalReceivedResponders,
		AsyncResult.TotalExpectedResponders);
}

float UGorgeousAutoReplicationRPCResultLibrary::GetResponderProgressFraction(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	if (AsyncResult.TotalExpectedResponders <= 0)
	{
		return 1.0f;
	}
	return static_cast<float>(AsyncResult.TotalReceivedResponders)
		/ static_cast<float>(AsyncResult.TotalExpectedResponders);
}

// =============================================================================
// FGorgeousAutoReplicationRPCValueResult  (slim path)
// =============================================================================

// Internal: build a slim result from a raw result.
// When TargetVariable is a UGorgeousRPC_OV (i.e. the completion path has already
// replaced the leaf OV with the container), drill one level deeper so ReturnOV
// always holds the actual handler-written OV, never the container itself.
FGorgeousAutoReplicationRPCValueResult UGorgeousAutoReplicationRPCResultLibrary::ToValueResult(const FGorgeousAutoReplicationRPCResult& Result)
{
	FGorgeousAutoReplicationRPCValueResult Out;
	Out.Responder = Result.Responder;
	if (UGorgeousRPC_OV* Container = Cast<UGorgeousRPC_OV>(Result.TargetVariable))
	{
		// OnCompleted path: TargetVariable was overwritten with the container.
		// Drill through to the actual handler-written OV.
		Out.RPCContainer = Container;
		Out.ReturnOV = Container->GetCachedTargetVariable();

		// The container's cached TargetVariable can also be null when the result was
		// relayed from a remote machine: the pointer doesn't survive the relay RPC but
		// the TargetVariableIdentifier (GUID) does.  Retry the registry lookup here —
		// by the time ToValueResult is called from Blueprint the variable is usually
		// registered even if it wasn't yet during NotifyRequestCompleted.
		if (!Out.ReturnOV)
		{
			const TArray<FGorgeousAutoReplicationRPCResult>& Cached = Container->GetCachedResults();
			if (Cached.Num() > 0 && Cached[0].TargetVariableIdentifier.IsValid())
			{
				Out.ReturnOV = UGorgeousRootObjectVariable::FindVariableByIdentifier(Cached[0].TargetVariableIdentifier);
			}
		}
	}
	else
	{
		// OnSingleResponderCompleted path: TargetVariable is already the leaf OV
		// (or null if the handler returned nothing).
		Out.ReturnOV = Result.TargetVariable;

		// Same relay timing issue as above: pointer null but GUID available.
		// Re-attempt the registry lookup now that the variable may be registered.
		if (!Out.ReturnOV && Result.TargetVariableIdentifier.IsValid())
		{
			Out.ReturnOV = UGorgeousRootObjectVariable::FindVariableByIdentifier(Result.TargetVariableIdentifier);
		}
	}
	return Out;
}

FGorgeousAutoReplicationRPCValueResult UGorgeousAutoReplicationRPCResultLibrary::GetPrimaryValueResult(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	return ToValueResult(AsyncResult.Result);
}

TArray<FGorgeousAutoReplicationRPCValueResult> UGorgeousAutoReplicationRPCResultLibrary::GetAllValueResults(const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult)
{
	TArray<FGorgeousAutoReplicationRPCValueResult> Out;
	Out.Reserve(AsyncResult.ResultSet.Num());
	for (const FGorgeousAutoReplicationRPCResult& R : AsyncResult.ResultSet)
	{
		Out.Add(ToValueResult(R));
	}
	return Out;
}
