// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "QualityOfLife/GorgeousGameInstance.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "Algo/Sort.h"

TMap<FGuid, UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState> UGorgeousAutoReplicationRPCRequestAsyncAction::PendingRequests;

UGorgeousAutoReplicationRPCRequestAsyncAction* UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(UObject* Context, FName Key, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, EGorgeousAutoReplicationTargetKind TargetKind)
{
	UGorgeousAutoReplicationRPCRequestAsyncAction* Action = NewObject<UGorgeousAutoReplicationRPCRequestAsyncAction>();
	Action->WeakContext = Context;
	Action->RequestKey = Key;
	Action->RequestType = Type;
	Action->RequestPayload = Payload;
	Action->RequestTargetKind = TargetKind;
	Action->ResultContainer = nullptr;
	Action->CachedResults.Reset();
	Action->CachedResultMap.Reset();
	Action->bActivated = false;
	Action->InternalResultContainer = nullptr;
	return Action;
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::Activate()
{
	if (bActivated)
	{
		return;
	}
	bActivated = true;

	UObject* Context = WeakContext.Get();
	if (!Context)
	{
		FailRequest();
		return;
	}

	FGorgeousAutoReplicationMixin* AutoReplicationMixin = ResolveAutoReplicationMixin(Context);
	if (!AutoReplicationMixin || !AutoReplicationMixin->IsNetworkingEnabled())
	{
		FailRequest();
		return;
	}

	RequestGuid = FGuid::NewGuid();
	RegisterPendingRequest();

	if (!AutoReplicationMixin->RequestRPC(RequestKey, RequestType, RequestPayload, RequestTargetKind, &RequestGuid))
	{
		FailRequest();
		return;
	}
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyRequestCompleted(const FGorgeousAutoReplicationRPCResult& Result)
{
	if (!Result.QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(Result.QueuedRPC.RequestGuid);
	if (!PendingState || PendingState->bCompleted)
	{
		return;
	}

	if (!PendingState->Action.IsValid())
	{
		PendingRequests.Remove(Result.QueuedRPC.RequestGuid);
		return;
	}

	FGorgeousAutoReplicationRPCResult ResultCopy = Result;
	if (UGorgeousAutoReplicationRPCRequestAsyncAction* Action = PendingState->Action.Get())
	{
		Action->ApplyRequestMetadata(ResultCopy);
		Action->ResolveResultPointers(ResultCopy);
	}

	const FString ResponderKey = BuildResponderKey(ResultCopy.Responder);
	PendingState->CollectedResults.Add(ResponderKey, ResultCopy);
	if (!ResponderKey.IsEmpty())
	{
		PendingState->ExpectedResponders.Add(ResponderKey);
	}

	TryCompletePendingRequest(ResultCopy.QueuedRPC.RequestGuid);
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::BeginDestroy()
{
	if (RequestGuid.IsValid())
	{
		PendingRequests.Remove(RequestGuid);
	}

	if (IsRooted())
	{
		RemoveFromRoot();
	}

	Super::BeginDestroy();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::FailRequest()
{
	FGorgeousQueuedRPC FailedCopy;
	FailedCopy.Key = RequestKey;
	FailedCopy.Type = RequestType;
	FailedCopy.Payload = RequestPayload;
	FailedCopy.RequestGuid = RequestGuid;
	FailedCopy.TargetKind = RequestTargetKind;

	FGorgeousAutoReplicationRPCResult FailedResult;
	FailedResult.QueuedRPC = FailedCopy;
	FailedResult.TargetKind = RequestTargetKind;
	FailedResult.TargetVariable = nullptr;
	FailedResult.TargetOwner = nullptr;
	CachedResults.Reset();
	CachedResults.Add(FailedResult);
	CachedResultMap.Reset();
	if (UGorgeousRPC_OV* Container = GetOrCreateResultContainer())
	{
		Container->CaptureResult(FailedResult);
		CachedResults[0].TargetVariable = Container;
	}

	OnFailed.Broadcast(BuildAsyncResultPayload());
	if (RequestGuid.IsValid())
	{
		PendingRequests.Remove(RequestGuid);
	}
	RequestGuid.Invalidate();
	if (IsRooted())
	{
		RemoveFromRoot();
	}
	SetReadyToDestroy();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterPendingRequest()
{
	if (!RequestGuid.IsValid())
	{
		return;
	}

	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState& PendingState = PendingRequests.FindOrAdd(RequestGuid);
	PendingState.Action = this;
	PendingState.bCompleted = false;
	PendingState.CollectedResults.Reset();
	PendingState.ExpectedResponders.Reset();
	AddToRoot();
	TryCompletePendingRequest(RequestGuid);
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterExpectedResponder(const FGuid& RequestGuid, const FGorgeousAutoReplicationRPCResponderHandle& Responder)
{
	if (!RequestGuid.IsValid() || !Responder.IsValid())
	{
		return;
	}

	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(RequestGuid);
	if (!PendingState)
	{
		return;
	}

	const FString ResponderKey = BuildResponderKey(Responder);
	if (!ResponderKey.IsEmpty())
	{
		PendingState->ExpectedResponders.Add(ResponderKey);
	}

	TryCompletePendingRequest(RequestGuid);
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::CompleteRequest(const TMap<FString, FGorgeousAutoReplicationRPCResult>& Results)
{
	if (Results.Num() == 0)
	{
		FailRequest();
		return;
	}

	CachedResults.Reset();
	CachedResults.Reserve(Results.Num());

	const FGorgeousAutoReplicationRPCResult* ServerResult = Results.Find(TEXT("Server"));
	if (ServerResult)
	{
		CachedResults.Add(*ServerResult);
	}

	TArray<FString> OrderedKeys;
	Results.GenerateKeyArray(OrderedKeys);
	OrderedKeys.Sort();
	for (const FString& Key : OrderedKeys)
	{
		if (Key == TEXT("Server"))
		{
			continue;
		}
		if (const FGorgeousAutoReplicationRPCResult* ResultPtr = Results.Find(Key))
		{
			CachedResults.Add(*ResultPtr);
		}
	}

	if (CachedResults.Num() == 0)
	{
		FailRequest();
		return;
	}
	if (UGorgeousRPC_OV* Container = GetOrCreateResultContainer())
	{
		Container->CaptureResultSet(Results, CachedResults);
		CachedResults[0].TargetVariable = Container;
	}

	CachedResultMap = Results;

	const UEnum* TypeEnum = StaticEnum<EGorgeousAutoReplicationRPCType>();
	const FString ExpectedType = TypeEnum ? TypeEnum->GetNameStringByValue(static_cast<int64>(RequestType)) : FString(TEXT("<unknown>"));
	const FGorgeousAutoReplicationRPCResult& PrimaryResult = CachedResults[0];
	const FString CompletedType = TypeEnum ? TypeEnum->GetNameStringByValue(static_cast<int64>(PrimaryResult.QueuedRPC.Type)) : FString(TEXT("<unknown>"));
	ensureMsgf(PrimaryResult.QueuedRPC.Type == RequestType,
		TEXT("Request AutoReplication RPC type mismatch. Expected %s but completed with %s for key %s."),
		*ExpectedType,
		*CompletedType,
		*PrimaryResult.QueuedRPC.Key.ToString());

	OnCompleted.Broadcast(BuildAsyncResultPayload());

	RequestGuid.Invalidate();
	if (IsRooted())
	{
		RemoveFromRoot();
	}
	SetReadyToDestroy();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::ResolveResultPointers(FGorgeousAutoReplicationRPCResult& Result)
{
	if (!Result.TargetVariable && Result.TargetVariableIdentifier.IsValid())
	{
		if (UGorgeousObjectVariable* ResolvedVariable = UGorgeousRootObjectVariable::FindVariableByIdentifier(Result.TargetVariableIdentifier))
		{
			Result.TargetVariable = ResolvedVariable;
		}
	}

	if (!Result.TargetOwner && Result.TargetVariable)
	{
		Result.TargetOwner = Result.TargetVariable->GetAutoReplicationOwner();
	}
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::ApplyRequestMetadata(FGorgeousAutoReplicationRPCResult& Result)
{
	Result.QueuedRPC.Key = RequestKey;
	Result.QueuedRPC.Type = RequestType;
	Result.QueuedRPC.Payload = RequestPayload;
	Result.QueuedRPC.TargetKind = RequestTargetKind;
	Result.QueuedRPC.RequestGuid = RequestGuid;

	if (Result.TargetKind == EGorgeousAutoReplicationTargetKind::EAuto)
	{
		Result.TargetKind = RequestTargetKind;
	}
}

FString UGorgeousAutoReplicationRPCRequestAsyncAction::BuildResponderKey(const FGorgeousAutoReplicationRPCResponderHandle& Responder)
{
	return Responder.IsValid() ? Responder.GetStableKey() : FString();
}

void UGorgeousAutoReplicationRPCRequestAsyncAction::TryCompletePendingRequest(const FGuid& RequestGuid)
{
	UGorgeousAutoReplicationRPCRequestAsyncAction::FGorgeousAutoReplicationPendingRequestState* PendingState = PendingRequests.Find(RequestGuid);
	if (!PendingState || PendingState->bCompleted)
	{
		return;
	}

	if (!PendingState->Action.IsValid())
	{
		PendingRequests.Remove(RequestGuid);
		return;
	}

	if (PendingState->CollectedResults.Num() == 0)
	{
		return;
	}

	bool bHasOutstandingResponder = false;
	for (const FString& ExpectedKey : PendingState->ExpectedResponders)
	{
		if (ExpectedKey.IsEmpty())
		{
			continue;
		}

		if (!PendingState->CollectedResults.Contains(ExpectedKey))
		{
			bHasOutstandingResponder = true;
			break;
		}
	}

	if (bHasOutstandingResponder)
	{
		return;
	}

	PendingState->bCompleted = true;
	const TMap<FString, FGorgeousAutoReplicationRPCResult> CompletedResults = PendingState->CollectedResults;
	if (UGorgeousAutoReplicationRPCRequestAsyncAction* Action = PendingState->Action.Get())
	{
		Action->CompleteRequest(CompletedResults);
	}
	PendingRequests.Remove(RequestGuid);
}

UGorgeousRPC_OV* UGorgeousAutoReplicationRPCRequestAsyncAction::GetOrCreateResultContainer()
{
	if (ResultContainer)
	{
		return ResultContainer;
	}

	if (!InternalResultContainer)
	{
		InternalResultContainer = NewObject<UGorgeousRPC_OV>(this);
	}

	return InternalResultContainer.Get();
}

FGorgeousAutoReplicationRPCAsyncResult UGorgeousAutoReplicationRPCRequestAsyncAction::BuildAsyncResultPayload() const
{
	FGorgeousAutoReplicationRPCAsyncResult Payload;
	Payload.Result = GetResolvedResult();
	Payload.ResultSet = CachedResults;
	Payload.ResultMap = CachedResultMap;
	Payload.ResultContainer = ResultContainer ? ResultContainer : InternalResultContainer.Get();
	return Payload;
}

const FGorgeousAutoReplicationRPCResult& UGorgeousAutoReplicationRPCRequestAsyncAction::GetResolvedResult() const
{
	if (CachedResults.Num() > 0)
	{
		return CachedResults[0];
	}

	static const FGorgeousAutoReplicationRPCResult EmptyResult;
	return EmptyResult;
}

FGorgeousAutoReplicationMixin* UGorgeousAutoReplicationRPCRequestAsyncAction::ResolveAutoReplicationMixin(UObject* Context)
{
	auto ResolveFromContext = [](UObject* Object) -> FGorgeousAutoReplicationMixin*
	{
		if (!Object)
		{
			return nullptr;
		}

		if (UGorgeousGameInstance* GameInstance = Cast<UGorgeousGameInstance>(Object))
		{
			return &GameInstance->GetAutoReplicationMixin();
		}
		if (AGorgeousGameMode* GameMode = Cast<AGorgeousGameMode>(Object))
		{
			return &GameMode->GetAutoReplicationMixin();
		}
		if (AGorgeousGameState* GameState = Cast<AGorgeousGameState>(Object))
		{
			return &GameState->GetAutoReplicationMixin();
		}
		if (AGorgeousPlayerController* PlayerController = Cast<AGorgeousPlayerController>(Object))
		{
			return &PlayerController->GetAutoReplicationMixin();
		}
		if (AGorgeousPlayerState* PlayerState = Cast<AGorgeousPlayerState>(Object))
		{
			return &PlayerState->GetAutoReplicationMixin();
		}
		if (AGorgeousWorldSettings* WorldSettings = Cast<AGorgeousWorldSettings>(Object))
		{
			return &WorldSettings->GetAutoReplicationMixin();
		}

		return nullptr;
	};

	if (FGorgeousAutoReplicationMixin* DirectMixin = ResolveFromContext(Context))
	{
		return DirectMixin;
	}

	if (AGorgeousGameState* WorldGameState = Cast<AGorgeousGameState>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(Context, AGorgeousGameState::StaticClass())))
	{
		return &WorldGameState->GetAutoReplicationMixin();
	}
	if (AGorgeousGameMode* WorldGameMode = Cast<AGorgeousGameMode>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(Context, AGorgeousGameMode::StaticClass())))
	{
		return &WorldGameMode->GetAutoReplicationMixin();
	}
	if (AGorgeousWorldSettings* WorldSettings = UGorgeousCoreRuntimeGlobals::GetGorgeousWorldSettings(Context))
	{
		return &WorldSettings->GetAutoReplicationMixin();
	}

	return nullptr;
}

