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

#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplicationRelay, Log, All);

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

bool UGorgeousAutoReplicationRPCRelayComponent::RelayPropertyPayloadToServer(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, FGorgeousAutoReplicationMixin* InTargetMixin)
{
	if (!GetOwner())
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("RelayPropertyPayloadToServer: No owner actor."));
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		// We're on the server, deliver directly
		if (InTargetMixin)
		{
			UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("RelayPropertyPayloadToServer: Delivering locally on server for entry %s."), *Envelope.EntryKey.ToString());
			InTargetMixin->HandleTransportedPropertyPayload(Envelope);
			return true;
		}
		return false;
	}

	// We're on the client, relay via RPC
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("RelayPropertyPayloadToServer: Client relaying property payload for entry %s to server."), *Envelope.EntryKey.ToString());
	ServerRelayPropertyPayload(Envelope);
	return true;
}

bool UGorgeousAutoReplicationRPCRelayComponent::RelayPropertyPayloadToClient(const FGorgeousAutoReplicationPropertyEnvelope& Envelope)
{
	if (!GetOwner())
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("RelayPropertyPayloadToClient: No owner actor."));
		return false;
	}

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("RelayPropertyPayloadToClient: Cannot relay to client from non-authority."));
		return false;
	}

	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("RelayPropertyPayloadToClient: Server relaying property payload for entry %s to owning client."), *Envelope.EntryKey.ToString());
	ClientRelayPropertyPayload(Envelope);
	return true;
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

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelayPropertyPayload_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope)
{
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayPropertyPayload: Server received property payload for entry %s with %d properties."), *Envelope.EntryKey.ToString(), Envelope.Payload.Properties.Num());

	if (TargetMixin)
	{
		TargetMixin->HandleTransportedPropertyPayload(Envelope);
	}
	else
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayPropertyPayload: No target mixin set for entry %s."), *Envelope.EntryKey.ToString());
	}
}

bool UGorgeousAutoReplicationRPCRelayComponent::RelayRPCToServer(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable, FGorgeousAutoReplicationMixin* InTargetMixin)
{
	if (!GetOwner())
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("RelayRPCToServer: No owner actor."));
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		// We're on the server, deliver directly via mixin
		if (InTargetMixin)
		{
			UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("RelayRPCToServer: Delivering RPC %s locally on server."), *QueuedRPC.Key.ToString());
			InTargetMixin->HandleTransportedRPC(QueuedRPC);
			return true;
		}
		return false;
	}

	// We're on the client, relay via RPC
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("RelayRPCToServer: Client relaying RPC %s to server (reliable=%d)."), *QueuedRPC.Key.ToString(), bReliable ? 1 : 0);
	if (bReliable)
	{
		ServerRelayRPCReliable(QueuedRPC);
	}
	else
	{
		ServerRelayRPCUnreliable(QueuedRPC);
	}
	return true;
}

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelayRPCReliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayRPCReliable: Server received RPC %s (Type=%d)."), *QueuedRPC.Key.ToString(), static_cast<int32>(QueuedRPC.Type));

	if (!TargetMixin)
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCReliable: No target mixin set for RPC %s."), *QueuedRPC.Key.ToString());
		return;
	}

	// Check the RPC type and route accordingly
	switch (QueuedRPC.Type)
	{
	case EGorgeousAutoReplicationRPCType::EReliableServer:
		// Server-bound RPC, deliver locally
		TargetMixin->HandleTransportedRPC(QueuedRPC);
		break;
	case EGorgeousAutoReplicationRPCType::EReliableClient:
	case EGorgeousAutoReplicationRPCType::EReliableMulticast:
		// Client or Multicast RPC needs to be forwarded through transporter
		TargetMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, nullptr);
		break;
	default:
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCReliable: Unexpected RPC type %d for RPC %s."), static_cast<int32>(QueuedRPC.Type), *QueuedRPC.Key.ToString());
		break;
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelayRPCUnreliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayRPCUnreliable: Server received RPC %s (Type=%d)."), *QueuedRPC.Key.ToString(), static_cast<int32>(QueuedRPC.Type));

	if (!TargetMixin)
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCUnreliable: No target mixin set for RPC %s."), *QueuedRPC.Key.ToString());
		return;
	}

	// Check the RPC type and route accordingly
	switch (QueuedRPC.Type)
	{
	case EGorgeousAutoReplicationRPCType::EUnreliableServer:
		// Server-bound RPC, deliver locally
		TargetMixin->HandleTransportedRPC(QueuedRPC);
		break;
	case EGorgeousAutoReplicationRPCType::EUnreliableClient:
	case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
		// Client or Multicast RPC needs to be forwarded through transporter
		TargetMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, nullptr);
		break;
	default:
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCUnreliable: Unexpected RPC type %d for RPC %s."), static_cast<int32>(QueuedRPC.Type), *QueuedRPC.Key.ToString());
		break;
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::ClientRelayPropertyPayload_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope)
{
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ClientRelayPropertyPayload: Client received property payload for entry %s with %d properties."), *Envelope.EntryKey.ToString(), Envelope.Payload.Properties.Num());

	if (TargetMixin)
	{
		TargetMixin->HandleTransportedPropertyPayload(Envelope);
	}
	else
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ClientRelayPropertyPayload: No target mixin set for entry %s."), *Envelope.EntryKey.ToString());
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::SerializeResult(const FGorgeousAutoReplicationRPCResult& Result, FGorgeousAutoReplicationSerializedRPCResult& OutSerialized) const
{
	OutSerialized.RequestGuid = Result.QueuedRPC.RequestGuid;
	OutSerialized.TargetKind = Result.TargetKind;
	OutSerialized.Responder = Result.Responder;
	OutSerialized.TargetVariableIdentifier = Result.TargetVariableIdentifier;
}
