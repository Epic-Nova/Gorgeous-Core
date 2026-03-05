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

#include "AutoReplication/GorgeousAutoReplicationRPCTransporter.h"

#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "Engine/World.h"
#include "Engine/Player.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "ObjectVariables/GorgeousObjectVariable.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplicationTransporter, Log, All);

namespace GorgeousAutoReplicationTransporter_Private
{
	static bool IsReliableRoute(const EGorgeousAutoReplicationRPCType RouteType)
	{
		switch (RouteType)
		{
		case EGorgeousAutoReplicationRPCType::EReliableServer:
		case EGorgeousAutoReplicationRPCType::EReliableClient:
		case EGorgeousAutoReplicationRPCType::EReliableMulticast:
			return true;
		case EGorgeousAutoReplicationRPCType::EUnreliableServer:
		case EGorgeousAutoReplicationRPCType::EUnreliableClient:
		case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
		default:
			return false;
		}
	}
}

UGorgeousAutoReplicationRPCTransporter::UGorgeousAutoReplicationRPCTransporter()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	OwningMixin = nullptr;
}

void UGorgeousAutoReplicationRPCTransporter::SetClientTargetOverride(APlayerController* InPlayerController)
{
	PendingClientTargetOverride = InPlayerController;
}

void UGorgeousAutoReplicationRPCTransporter::ClearClientTargetOverride()
{
	PendingClientTargetOverride.Reset();
}

void UGorgeousAutoReplicationRPCTransporter::InitializeTransporter(FGorgeousAutoReplicationMixin* InOwningMixin)
{
	if (OwningMixin && OwningMixin != InOwningMixin)
	{
		return;
	}

	OwningMixin = InOwningMixin;
}

void UGorgeousAutoReplicationRPCTransporter::OnRegister()
{
	Super::OnRegister();
	if (!IsRegistered())
	{
		return;
	}

	if (!GetIsReplicated())
	{
		SetIsReplicated(true);
	}
}

bool UGorgeousAutoReplicationRPCTransporter::RouteRPC(const FGorgeousQueuedRPC& QueuedRPC)
{
	switch (QueuedRPC.Type)
	{
	case EGorgeousAutoReplicationRPCType::EReliableServer:
		return RouteServerBoundRPC(QueuedRPC, true);
	case EGorgeousAutoReplicationRPCType::EUnreliableServer:
		return RouteServerBoundRPC(QueuedRPC, false);
	case EGorgeousAutoReplicationRPCType::EReliableClient:
		return RouteClientBoundRPC(QueuedRPC, true);
	case EGorgeousAutoReplicationRPCType::EUnreliableClient:
		return RouteClientBoundRPC(QueuedRPC, false);
	case EGorgeousAutoReplicationRPCType::EReliableMulticast:
		return RouteMulticastRPC(QueuedRPC, true);
	case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
		return RouteMulticastRPC(QueuedRPC, false);
	default:
		break;
	}

	UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication RPC transporter received unsupported type."));
	return false;
}

bool UGorgeousAutoReplicationRPCTransporter::RoutePropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	if (!OwningMixin)
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication transporter %s has no owning mixin while routing property payload for entry %s."),
			*GetName(), *Envelope.EntryKey.ToString());
		return false;
	}

	AActor* OwnerActor = GetOwner();
	UE_LOG(LogGorgeousAutoReplicationTransporter, Log, TEXT("RoutePropertyPayload: Transporter %s on actor %s routing entry %s (RouteType=%d, IsOwnerLocallyAuthoritative=%d, OwnerRole=%d)."),
		*GetName(), OwnerActor ? *OwnerActor->GetName() : TEXT("null"), *Envelope.EntryKey.ToString(),
		static_cast<int32>(RouteType), IsOwnerLocallyAuthoritative() ? 1 : 0,
		OwnerActor ? static_cast<int32>(OwnerActor->GetLocalRole()) : -1);

	const bool bReliable = GorgeousAutoReplicationTransporter_Private::IsReliableRoute(RouteType);

	switch (RouteType)
	{
	case EGorgeousAutoReplicationRPCType::EReliableServer:
	case EGorgeousAutoReplicationRPCType::EUnreliableServer:
	{
		if (IsOwnerLocallyAuthoritative())
		{
			UE_LOG(LogGorgeousAutoReplicationTransporter, Log, TEXT("RoutePropertyPayload: Owner is authoritative, delivering locally."));
			DeliverPropertyPayload(Envelope);
		}
		else if (bReliable)
		{
			UE_LOG(LogGorgeousAutoReplicationTransporter, Log, TEXT("RoutePropertyPayload: Calling ServerReceivePropertyPayloadReliable RPC."));
			ServerReceivePropertyPayloadReliable(Envelope, RouteType);
		}
		else
		{
			UE_LOG(LogGorgeousAutoReplicationTransporter, Log, TEXT("RoutePropertyPayload: Calling ServerReceivePropertyPayloadUnreliable RPC."));
			ServerReceivePropertyPayloadUnreliable(Envelope, RouteType);
		}
		return true;
	}
	case EGorgeousAutoReplicationRPCType::EReliableClient:
	case EGorgeousAutoReplicationRPCType::EUnreliableClient:
	{
		if (IsOwnerLocallyAuthoritative())
		{
			return ForwardPropertyPayloadToClient(Envelope, RouteType);
		}

		if (bReliable)
		{
			ServerReceivePropertyPayloadReliable(Envelope, RouteType);
		}
		else
		{
			ServerReceivePropertyPayloadUnreliable(Envelope, RouteType);
		}
		return true;
	}
	case EGorgeousAutoReplicationRPCType::EReliableMulticast:
	case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
	{
		if (IsOwnerLocallyAuthoritative())
		{
			ForwardPropertyPayloadToMulticast(Envelope, RouteType);
			return true;
		}

		if (bReliable)
		{
			ServerReceivePropertyPayloadReliable(Envelope, RouteType);
		}
		else
		{
			ServerReceivePropertyPayloadUnreliable(Envelope, RouteType);
		}
		return true;
	}
	default:
		break;
	}

	UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication transporter %s received unsupported property payload route."), *GetName());
	return false;
}

bool UGorgeousAutoReplicationRPCTransporter::IsLinkedToMixin(const FGorgeousAutoReplicationMixin* InOwningMixin) const
{
	return OwningMixin == InOwningMixin;
}

bool UGorgeousAutoReplicationRPCTransporter::RouteServerBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, const bool bReliable)
{
	if (IsOwnerLocallyAuthoritative())
	{
		DeliverRPCLocally(QueuedRPC);
		return true;
	}

	return bReliable ? (ServerReceiveRPCReliable(QueuedRPC), true) : (ServerReceiveRPCUnreliable(QueuedRPC), true);
}

bool UGorgeousAutoReplicationRPCTransporter::RouteClientBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, const bool bReliable)
{
	if (IsOwnerLocallyControlledClient())
	{
		DeliverRPCLocally(QueuedRPC);
		return true;
	}

	if (!IsOwnerLocallyAuthoritative())
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication RPC transporter cannot route client-bound RPC because the owner has no authority."));
		return false;
	}

	return ForwardRPCToClient(QueuedRPC, bReliable);
}

bool UGorgeousAutoReplicationRPCTransporter::RouteMulticastRPC(const FGorgeousQueuedRPC& QueuedRPC, const bool bReliable)
{
	if (!IsOwnerLocallyAuthoritative())
	{
		return bReliable ? (ServerReceiveRPCReliable(QueuedRPC), true) : (ServerReceiveRPCUnreliable(QueuedRPC), true);
	}

	ForwardRPCToMulticast(QueuedRPC, bReliable);
	return true;
}

bool UGorgeousAutoReplicationRPCTransporter::ForwardRPCToClient(const FGorgeousQueuedRPC& QueuedRPC, const bool bReliable)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication RPC transporter cannot locate owning actor while routing client RPC."));
		return false;
	}

	APlayerController* TargetController = nullptr;

	// 1. When the owning actor IS a PlayerController (e.g. a PC subclass receiving a
	//    server→client RPC), use it directly — GetNetOwningPlayer() is often null for
	//    a PC instance that lives on the server side before the connection is fully set.
	if (APlayerController* DirectPC = Cast<APlayerController>(OwnerActor))
	{
		TargetController = DirectPC;
	}

	// 2. Fall back to the UPlayer → PlayerController resolution path.
	if (!TargetController && OwnerActor->GetNetOwningPlayer())
	{
		TargetController = OwnerActor->GetNetOwningPlayer()->GetPlayerController(GetWorld());
	}

	// 3. Broadcast the override event so the caller can provide an explicit target.
	if (!TargetController)
	{
		OnResolveClientTarget.Broadcast(this, QueuedRPC);
		if (PendingClientTargetOverride.IsValid())
		{
			TargetController = PendingClientTargetOverride.Get();
			PendingClientTargetOverride.Reset();
		}
	}

	if (!TargetController)
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication RPC %s could not resolve a client target on %s."), *QueuedRPC.Key.ToString(), *OwnerActor->GetName());
		return false;
	}

	if (QueuedRPC.RequestGuid.IsValid())
	{
		const FGorgeousAutoReplicationRPCResponderHandle Handle = FGorgeousAutoReplicationRPCResponderHandle::FromController(TargetController);
		if (Handle.IsValid())
		{
			UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterExpectedResponder(QueuedRPC.RequestGuid, Handle);
		}
	}

	UGorgeousAutoReplicationRPCTransporter* TargetTransporter = TargetController->FindComponentByClass<UGorgeousAutoReplicationRPCTransporter>();
	if (!TargetTransporter)
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("PlayerController %s has no AutoReplication transporter. RPC %s dropped."), *TargetController->GetName(), *QueuedRPC.Key.ToString());
		return false;
	}

	if (OwningMixin && !OwningMixin->CanControllerReceiveRPC(QueuedRPC, TargetController))
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("Blocked AutoReplication RPC %s because controller %s lacks access to key %s."), *QueuedRPC.Payload.HandlerName.ToString(), *TargetController->GetName(), *QueuedRPC.Key.ToString());
		return false;
	}

	if (bReliable)
	{
		TargetTransporter->ClientReceiveRPCReliable(QueuedRPC);
	}
	else
	{
		TargetTransporter->ClientReceiveRPCUnreliable(QueuedRPC);
	}

	OnRPCForwarded.Broadcast(QueuedRPC, TargetController);
	return true;
}

void UGorgeousAutoReplicationRPCTransporter::ForwardRPCToMulticast(const FGorgeousQueuedRPC& QueuedRPC, const bool bReliable)
{
	if (QueuedRPC.RequestGuid.IsValid())
	{
		if (UWorld* OwnerWorld = GetWorld())
		{
			for (FConstPlayerControllerIterator It = OwnerWorld->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PlayerController = It->Get();
				if (!PlayerController)
				{
					continue;
				}

				if (OwningMixin && !OwningMixin->CanControllerReceiveRPC(QueuedRPC, PlayerController))
				{
					continue;
				}

				const FGorgeousAutoReplicationRPCResponderHandle Handle = FGorgeousAutoReplicationRPCResponderHandle::FromController(PlayerController);
				if (Handle.IsValid())
				{
					UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterExpectedResponder(QueuedRPC.RequestGuid, Handle);
				}
			}
		}
	}

	if (bReliable)
	{
		MulticastReceiveRPCReliable(QueuedRPC);
	}
	else
	{
		MulticastReceiveRPCUnreliable(QueuedRPC);
	}

	OnRPCForwarded.Broadcast(QueuedRPC, GetOwner());
}

bool UGorgeousAutoReplicationRPCTransporter::ForwardPropertyPayloadToClient(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	if (!IsOwnerLocallyAuthoritative())
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication transporter %s cannot forward property payload %s to client because the owner lacks authority."),
			*GetName(), *Envelope.EntryKey.ToString());
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication transporter %s has no owning actor while routing property payload %s."),
			*GetName(), *Envelope.EntryKey.ToString());
		return false;
	}

	APlayerController* TargetController = nullptr;
	if (OwnerActor->GetNetOwningPlayer())
	{
		TargetController = OwnerActor->GetNetOwningPlayer()->GetPlayerController(GetWorld());
	}

	if (!TargetController)
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication transporter %s cannot resolve client target while routing property payload %s."),
			*GetName(), *Envelope.EntryKey.ToString());
		return false;
	}

	if (OwningMixin && !OwningMixin->CanControllerReceivePropertyPayload(Envelope.EntryKey, TargetController))
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Verbose, TEXT("Blocked AutoReplication property payload %s because controller %s lacks channel access."),
			*Envelope.EntryKey.ToString(), *TargetController->GetName());
		return false;
	}

	if (GorgeousAutoReplicationTransporter_Private::IsReliableRoute(RouteType))
	{
		ClientReceivePropertyPayloadReliable(Envelope, RouteType);
	}
	else
	{
		ClientReceivePropertyPayloadUnreliable(Envelope, RouteType);
	}

	return true;
}

void UGorgeousAutoReplicationRPCTransporter::ForwardPropertyPayloadToMulticast(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	if (!IsOwnerLocallyAuthoritative())
	{
		UE_LOG(LogGorgeousAutoReplicationTransporter, Warning, TEXT("AutoReplication transporter %s cannot multicast property payload %s because the owner lacks authority."),
			*GetName(), *Envelope.EntryKey.ToString());
		return;
	}

	if (OwningMixin)
	{
		UGorgeousObjectVariable* TargetVariable = OwningMixin->ResolveVariableForKey(Envelope.EntryKey);
		if (TargetVariable && TargetVariable->IsRootNetworkStackEnabled())
		{
			if (UWorld* VariableWorld = TargetVariable->GetVariableWorld())
			{
				if (UGorgeousRootNetworkStackSubsystem* RootNetworkStack = UGorgeousRootNetworkStackSubsystem::Get(VariableWorld))
				{
					const FName ChannelName = TargetVariable->GetEffectiveNetworkChannel();
					if (!RootNetworkStack->HasChannelSubscribers(ChannelName))
					{
						UE_LOG(LogGorgeousAutoReplicationTransporter, VeryVerbose,
							TEXT("Skipping AutoReplication property payload %s multicast because channel %s has no subscribers."),
							*Envelope.EntryKey.ToString(), *ChannelName.ToString());
						return;
					}
				}

				// UE multicast RPCs deliver to ALL connections unconditionally — the engine provides no
				// per-connection filtering hook. Pivot to per-PC unicasts via each PC's relay component
				// so that CanControllerReceivePropertyPayload is enforced for every individual client,
				// identical to the access enforcement done in TryServerReplicateProperties.
				for (FConstPlayerControllerIterator It = VariableWorld->GetPlayerControllerIterator(); It; ++It)
				{
					APlayerController* PC = It->Get();
					if (!PC || PC->IsLocalController())
					{
						continue;
					}

					if (!OwningMixin->CanControllerReceivePropertyPayload(Envelope.EntryKey, PC))
					{
						UE_LOG(LogGorgeousAutoReplicationTransporter, Verbose,
							TEXT("Blocked AutoReplication multicast payload %s for controller %s due to access policy."),
							*Envelope.EntryKey.ToString(), *PC->GetName());
						continue;
					}

					if (UGorgeousAutoReplicationRPCRelayComponent* RelayComponent = PC->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>())
					{
						RelayComponent->RelayPropertyPayloadToClient(Envelope);
					}
				}
				// Do not fall through to the UE multicast RPC — all eligible clients were served above.
				return;
			}
		}
	}

	// No root network stack restrictions — all connections are eligible. Use the UE multicast RPC.
	if (GorgeousAutoReplicationTransporter_Private::IsReliableRoute(RouteType))
	{
		MulticastReceivePropertyPayloadReliable(Envelope, RouteType);
	}
	else
	{
		MulticastReceivePropertyPayloadUnreliable(Envelope, RouteType);
	}
}

bool UGorgeousAutoReplicationRPCTransporter::IsOwnerLocallyAuthoritative() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

bool UGorgeousAutoReplicationRPCTransporter::IsOwnerLocallyControlledClient() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (OwnerActor->HasAuthority())
	{
		return false;
	}

	if (const APawn* PawnOwner = Cast<APawn>(OwnerActor))
	{
		return PawnOwner->IsLocallyControlled();
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(OwnerActor))
	{
		return PlayerController->IsLocalController();
	}

	return OwnerActor->GetLocalRole() == ROLE_AutonomousProxy;
}

void UGorgeousAutoReplicationRPCTransporter::DeliverRPCLocally(const FGorgeousQueuedRPC& QueuedRPC)
{
	if (OwningMixin)
	{
		OwningMixin->HandleTransportedRPC(QueuedRPC);
		OnRPCForwarded.Broadcast(QueuedRPC, OwningMixin->ResolveOwnerObject());
	}
}

void UGorgeousAutoReplicationRPCTransporter::DeliverPropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope)
{
	if (OwningMixin)
	{
		OwningMixin->HandleTransportedPropertyPayload(Envelope);
	}
}

void UGorgeousAutoReplicationRPCTransporter::ServerReceiveRPCReliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	if (!OwningMixin)
	{
		return;
	}

	switch (QueuedRPC.Type)
	{
	case EGorgeousAutoReplicationRPCType::EReliableServer:
		DeliverRPCLocally(QueuedRPC);
		break;
	case EGorgeousAutoReplicationRPCType::EReliableClient:
		ForwardRPCToClient(QueuedRPC, true);
		break;
	case EGorgeousAutoReplicationRPCType::EReliableMulticast:
		ForwardRPCToMulticast(QueuedRPC, true);
		break;
	default:
		break;
	}
}

void UGorgeousAutoReplicationRPCTransporter::ServerReceiveRPCUnreliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	if (!OwningMixin)
	{
		return;
	}

	switch (QueuedRPC.Type)
	{
	case EGorgeousAutoReplicationRPCType::EUnreliableServer:
		DeliverRPCLocally(QueuedRPC);
		break;
	case EGorgeousAutoReplicationRPCType::EUnreliableClient:
		ForwardRPCToClient(QueuedRPC, false);
		break;
	case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
		ForwardRPCToMulticast(QueuedRPC, false);
		break;
	default:
		break;
	}
}

void UGorgeousAutoReplicationRPCTransporter::ClientReceiveRPCReliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	DeliverRPCLocally(QueuedRPC);
}

void UGorgeousAutoReplicationRPCTransporter::ClientReceiveRPCUnreliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	DeliverRPCLocally(QueuedRPC);
}

void UGorgeousAutoReplicationRPCTransporter::MulticastReceiveRPCReliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	DeliverRPCLocally(QueuedRPC);
}

void UGorgeousAutoReplicationRPCTransporter::MulticastReceiveRPCUnreliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	DeliverRPCLocally(QueuedRPC);
}

void UGorgeousAutoReplicationRPCTransporter::ServerReceivePropertyPayloadReliable_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	UE_LOG(LogGorgeousAutoReplicationTransporter, Log, TEXT("Server received reliable property payload for entry %s with %d properties."), *Envelope.EntryKey.ToString(), Envelope.Payload.Properties.Num());

	// For server-bound route types the payload is delivered locally without passing through
	// ForwardPropertyPayloadToClient / ForwardPropertyPayloadToMulticast, which are the two other
	// sites that call CanControllerReceivePropertyPayload. Validate the sending client here so an
	// unauthorized client cannot push data to a variable it does not own or lacks channel access to.
	if (RouteType == EGorgeousAutoReplicationRPCType::EReliableServer
		|| RouteType == EGorgeousAutoReplicationRPCType::EUnreliableServer)
	{
		APlayerController* SenderController = nullptr;
		if (AActor* OwnerActor = GetOwner())
		{
			if (UPlayer* NetPlayer = OwnerActor->GetNetOwningPlayer())
			{
				SenderController = NetPlayer->GetPlayerController(GetWorld());
			}
		}

		if (OwningMixin && !OwningMixin->CanControllerReceivePropertyPayload(Envelope.EntryKey, SenderController))
		{
			UE_LOG(LogGorgeousAutoReplicationTransporter, Warning,
				TEXT("Blocked server-bound property payload %s from controller %s due to access policy."),
				*Envelope.EntryKey.ToString(), SenderController ? *SenderController->GetName() : TEXT("<null>"));
			return;
		}
	}

	RoutePropertyPayload(Envelope, RouteType);
}

void UGorgeousAutoReplicationRPCTransporter::ServerReceivePropertyPayloadUnreliable_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	UE_LOG(LogGorgeousAutoReplicationTransporter, Log, TEXT("Server received unreliable property payload for entry %s with %d properties."), *Envelope.EntryKey.ToString(), Envelope.Payload.Properties.Num());

	// Same sender validation as the reliable variant — see comment above.
	if (RouteType == EGorgeousAutoReplicationRPCType::EReliableServer
		|| RouteType == EGorgeousAutoReplicationRPCType::EUnreliableServer)
	{
		APlayerController* SenderController = nullptr;
		if (AActor* OwnerActor = GetOwner())
		{
			if (UPlayer* NetPlayer = OwnerActor->GetNetOwningPlayer())
			{
				SenderController = NetPlayer->GetPlayerController(GetWorld());
			}
		}

		if (OwningMixin && !OwningMixin->CanControllerReceivePropertyPayload(Envelope.EntryKey, SenderController))
		{
			UE_LOG(LogGorgeousAutoReplicationTransporter, Warning,
				TEXT("Blocked server-bound property payload %s from controller %s due to access policy."),
				*Envelope.EntryKey.ToString(), SenderController ? *SenderController->GetName() : TEXT("<null>"));
			return;
		}
	}

	RoutePropertyPayload(Envelope, RouteType);
}

void UGorgeousAutoReplicationRPCTransporter::ClientReceivePropertyPayloadReliable_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	DeliverPropertyPayload(Envelope);
}

void UGorgeousAutoReplicationRPCTransporter::ClientReceivePropertyPayloadUnreliable_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	DeliverPropertyPayload(Envelope);
}

void UGorgeousAutoReplicationRPCTransporter::MulticastReceivePropertyPayloadReliable_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	DeliverPropertyPayload(Envelope);
}

void UGorgeousAutoReplicationRPCTransporter::MulticastReceivePropertyPayloadUnreliable_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType)
{
	DeliverPropertyPayload(Envelope);
}
