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

#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"

#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "Engine/World.h"

UGorgeousAutoReplicationRPCRelayComponent::UGorgeousAutoReplicationRPCRelayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGorgeousAutoReplicationRPCRelayComponent::RelayResultToServer(const FGorgeousAutoReplicationRPCResult& Result)
{
	if (!Result.QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		FGorgeousAutoReplicationSerializedRPCResult Serialized;
		SerializeResult(Result, Serialized);
		ServerRelayAutoReplicationResult(Serialized);
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelayAutoReplicationResult_Implementation(const FGorgeousAutoReplicationSerializedRPCResult& SerializedResult)
{
	if (!SerializedResult.RequestGuid.IsValid())
	{
		return;
	}

	FGorgeousAutoReplicationRPCResult Result;
	Result.QueuedRPC.RequestGuid = SerializedResult.RequestGuid;
	Result.TargetKind = SerializedResult.TargetKind;
	Result.Responder = SerializedResult.Responder;
	Result.TargetVariableIdentifier = SerializedResult.TargetVariableIdentifier;

	UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyRequestCompleted(Result);
}

void UGorgeousAutoReplicationRPCRelayComponent::SerializeResult(const FGorgeousAutoReplicationRPCResult& Result, FGorgeousAutoReplicationSerializedRPCResult& OutSerialized) const
{
	OutSerialized.RequestGuid = Result.QueuedRPC.RequestGuid;
	OutSerialized.TargetKind = Result.TargetKind;
	OutSerialized.Responder = Result.Responder;
	OutSerialized.TargetVariableIdentifier = Result.TargetVariableIdentifier;
}
