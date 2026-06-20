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

#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"

#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "InsightMatrix/GorgeousRPCDebugTracker.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplicationRelay, Log, All);

// Server-side map: RequestGuid → originating client PC (populated in ServerRelayRPC*_Implementation).
// Entries remain valid as long as the PC weak ptr is live; stale entries are lazily removed.
TMap<FGuid, TWeakObjectPtr<APlayerController>> UGorgeousAutoReplicationRPCRelayComponent::ClientInitiatedRPCOrigins;

// =============================================================================
// OV tree serialization helpers (file-private)
// =============================================================================
namespace
{
	// Properties that are handled structurally or are pure configuration, never emitted into
	// the per-node property list.
	static const TArray<FName> GOVSkippedPropertyNames =
	{
		FName(TEXT("VariableRegistry")),
		FName(TEXT("UniqueIdentifier")),
		FName(TEXT("bSupportsNetworking")),
		FName(TEXT("RootConfiguration")),
	};

	static bool ShouldSkipOVProperty(const FProperty* Prop)
	{
		if (GOVSkippedPropertyNames.Contains(Prop->GetFName()))
		{
			return true;
		}
		// Skip transient / editor-only / config, not meaningful as captured data
		if (Prop->HasAnyPropertyFlags(CPF_Transient | CPF_NonPIEDuplicateTransient | CPF_EditorOnly | CPF_Config))
		{
			return true;
		}
		// Skip UObject / class references, can't safely cross network without a PackageMap
		if (Prop->IsA<FObjectProperty>() ||
			Prop->IsA<FWeakObjectProperty>() ||
			Prop->IsA<FSoftObjectProperty>() ||
			Prop->IsA<FLazyObjectProperty>() ||
			Prop->IsA<FClassProperty>() ||
			Prop->IsA<FSoftClassProperty>())
		{
			return true;
		}
		return false;
	}

	// Recursively walk Node and its VariableRegistry children, appending one
	// FGorgeousSerializedOVNode per OV (depth-first, parent before children).
	static void AppendOVTreeNodes(const UGorgeousObjectVariable* Node,
		const FGuid& ParentGuid,
		TArray<FGorgeousSerializedOVNode>& OutNodes,
		FName InRegistryKey = NAME_None)
	{
		if (!Node)
		{
			return;
		}

		FGorgeousSerializedOVNode SerNode;
		SerNode.Identifier       = Node->UniqueIdentifier;
		SerNode.ParentIdentifier = ParentGuid;
		SerNode.ClassPath        = Node->GetClass()->GetPathName();
		SerNode.RegistryKey      = InRegistryKey;

		for (TFieldIterator<FProperty> PropIt(Node->GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
		{
			FProperty* Prop = *PropIt;
			if (ShouldSkipOVProperty(Prop))
			{
				continue;
			}

			FGorgeousOVPropertyCapture Capture;
			Capture.PropertyName = Prop->GetName();

			const void* PropPtr = Prop->ContainerPtrToValuePtr<void>(Node);
			Prop->ExportTextItem_Direct(Capture.ExportedValue, PropPtr, nullptr,
				const_cast<UGorgeousObjectVariable*>(Node), PPF_None, nullptr);

			SerNode.Properties.Add(MoveTemp(Capture));
		}

		OutNodes.Add(MoveTemp(SerNode));

		for (const TPair<FName, TObjectPtr<UGorgeousObjectVariable>>& Pair : Node->VariableRegistry)
		{
			AppendOVTreeNodes(Pair.Value.Get(), Node->UniqueIdentifier, OutNodes, Pair.Key);
		}
	}

	// Reconstruct an OV tree from the serialized node list.
	// Returns the root OV (the node whose ParentIdentifier is invalid), or nullptr.
	static UGorgeousObjectVariable* ReconstructOVTree(const TArray<FGorgeousSerializedOVNode>& Nodes)
	{
		if (Nodes.IsEmpty())
		{
			return nullptr;
		}

		TMap<FGuid, UGorgeousObjectVariable*> Created;
		Created.Reserve(Nodes.Num());

		// Pass 1: create all instances and restore property values
		for (const FGorgeousSerializedOVNode& Node : Nodes)
		{
			UClass* OVClass = FindObject<UClass>(nullptr, *Node.ClassPath);
			if (!OVClass)
			{
				OVClass = LoadObject<UClass>(nullptr, *Node.ClassPath);
			}
			if (!OVClass)
			{
				UE_LOG(LogGorgeousAutoReplicationRelay, Warning,
					TEXT("ReconstructOVTree: Could not find class '%s', node skipped."),
					*Node.ClassPath);
				continue;
			}

			UGorgeousObjectVariable* NewOV = NewObject<UGorgeousObjectVariable>(
				GetTransientPackage(), OVClass);
			if (!NewOV)
			{
				continue;
			}

			// Restore the original GUID so lookups by identifier still work
			NewOV->UniqueIdentifier = Node.Identifier;

			// Build a name→property map for O(1) import lookup
			TMap<FString, FProperty*> PropMap;
			for (TFieldIterator<FProperty> PropIt(NewOV->GetClass(), EFieldIteratorFlags::IncludeSuper);
				PropIt; ++PropIt)
			{
				PropMap.Add(PropIt->GetName(), *PropIt);
			}

			for (const FGorgeousOVPropertyCapture& Cap : Node.Properties)
			{
				FProperty** Found = PropMap.Find(Cap.PropertyName);
				if (!Found)
				{
					continue;
				}
				FProperty* Prop = *Found;
				if (ShouldSkipOVProperty(Prop))
				{
					continue;
				}

				void* PropPtr = Prop->ContainerPtrToValuePtr<void>(NewOV);
				Prop->ImportText_Direct(*Cap.ExportedValue, PropPtr, NewOV, PPF_None);
			}

			Created.Add(Node.Identifier, NewOV);
		}

		// Pass 2: wire up VariableRegistry parent↔child relationships
		for (const FGorgeousSerializedOVNode& Node : Nodes)
		{
			if (!Node.ParentIdentifier.IsValid())
			{
				continue;
			}
			UGorgeousObjectVariable** Parent = Created.Find(Node.ParentIdentifier);
			UGorgeousObjectVariable** Child  = Created.Find(Node.Identifier);
			if (Parent && Child)
			{
				const FName Key = !Node.RegistryKey.IsNone()
					? Node.RegistryKey
					: FName(*Node.Identifier.ToString().Left(8));
				(*Parent)->VariableRegistry.Add(Key, *Child);
			}
		}

		// Return the root node (invalid ParentIdentifier)
		for (const FGorgeousSerializedOVNode& Node : Nodes)
		{
			if (!Node.ParentIdentifier.IsValid())
			{
				if (UGorgeousObjectVariable** Root = Created.Find(Node.Identifier))
				{
					return *Root;
				}
			}
		}
		return nullptr;
	}
} // namespace

UGorgeousAutoReplicationRPCRelayComponent::UGorgeousAutoReplicationRPCRelayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGorgeousAutoReplicationRPCRelayComponent::RegisterClientInitiatedRPCOrigin(
	const FGuid& RequestGuid, APlayerController* OriginPC)
{
	if (!RequestGuid.IsValid() || !OriginPC)
	{
		return;
	}
	ClientInitiatedRPCOrigins.Add(RequestGuid, OriginPC);
}

void UGorgeousAutoReplicationRPCRelayComponent::TryRelayResultToClientInitiator(
	const FGorgeousAutoReplicationRPCResult& Result, EGorgeousRPCReadyState ReadyState)
{
	if (!Result.QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	// Lazily remove dead (GC'd) entries before looking up.
	for (auto It = ClientInitiatedRPCOrigins.CreateIterator(); It; ++It)
	{
		if (!It->Value.IsValid())
		{
			It.RemoveCurrent();
		}
	}

	TWeakObjectPtr<APlayerController>* OriginatorPtr = ClientInitiatedRPCOrigins.Find(Result.QueuedRPC.RequestGuid);
	if (!OriginatorPtr)
	{
		return;
	}

	APlayerController* OriginPC = OriginatorPtr->Get();
	if (!OriginPC)
	{
		ClientInitiatedRPCOrigins.Remove(Result.QueuedRPC.RequestGuid);
		return;
	}

	UGorgeousAutoReplicationRPCRelayComponent* Relay =
		OriginPC->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>();
	if (!Relay)
	{
		return;
	}

	FGorgeousAutoReplicationSerializedRPCResult Serialized;
	Relay->SerializeResult(Result, Serialized);
	Serialized.RelayedReadyState = ReadyState;
	Relay->ClientRelayAutoReplicationResult(Serialized);

	// For non-multicast types only one result is expected, clean up to avoid accumulation.
	const bool bIsMulticast =
		Result.QueuedRPC.Type == EGorgeousAutoReplicationRPCType::EReliableMulticast ||
		Result.QueuedRPC.Type == EGorgeousAutoReplicationRPCType::EUnreliableMulticast;
	if (!bIsMulticast)
	{
		ClientInitiatedRPCOrigins.Remove(Result.QueuedRPC.RequestGuid);
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::RelayResultToServer(const FGorgeousAutoReplicationRPCResult& Result,
	EGorgeousRPCReadyState ReadyState)
{
	if (!Result.QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		FGorgeousAutoReplicationSerializedRPCResult Serialized;
		SerializeResult(Result, Serialized);
		Serialized.RelayedReadyState = ReadyState;
		ServerRelayAutoReplicationResult(Serialized);
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::RelayResponderReadyStateToServer(const FGuid& RequestGuid, const FString& ResponderKey, EGorgeousRPCReadyState ReadyState)
{
	if (!RequestGuid.IsValid() || ResponderKey.IsEmpty())
	{
		return;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerRelayResponderReadyState(RequestGuid, ResponderKey, ReadyState);
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
	Result.QueuedRPC.RequestGuid     = SerializedResult.RequestGuid;
	Result.TargetKind               = SerializedResult.TargetKind;
	Result.Responder                = SerializedResult.Responder;
	Result.TargetVariableIdentifier = SerializedResult.TargetVariableIdentifier;

	// If the sender included a full OV tree, reconstruct it here so the receiving side
	// has the actual return OV (including all child entries in VariableRegistry) without
	// needing the OV to already exist in this machine's registry.
	if (SerializedResult.OVTree.Num() > 0)
	{
		if (UGorgeousObjectVariable* ReconstructedRoot = ReconstructOVTree(SerializedResult.OVTree))
		{
			Result.TargetVariable = ReconstructedRoot;
			UE_LOG(LogGorgeousAutoReplicationRelay, Verbose,
				TEXT("ServerRelayAutoReplicationResult: Reconstructed OV tree (%d nodes) for request %s."),
				SerializedResult.OVTree.Num(),
				*SerializedResult.RequestGuid.ToString());
		}
	}

	// Route to the correct server-side handler based on the ready state the client declared.
	if (SerializedResult.RelayedReadyState == EGorgeousRPCReadyState::ReadyForSingleResponderCallback)
	{
		// Interim signal: fire OnSingleResponderCompleted without completing the overall request.
		// The client will send a final Ready relay later.
		UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyDeferredSingleResponderCallback(
			SerializedResult.RequestGuid, Result);
	}
	else
	{
		UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyRequestCompleted(Result);
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelayPropertyPayload_Implementation(const FGorgeousAutoReplicationPropertyEnvelope& Envelope)
{
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayPropertyPayload: Server received property payload for entry %s with %d properties."), *Envelope.EntryKey.ToString(), Envelope.Payload.Properties.Num());

	if (!TargetMixin)
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayPropertyPayload: No target mixin set for entry %s."), *Envelope.EntryKey.ToString());
		return;
	}

	// Validate that the sending client is permitted to push property changes for this entry.
	APlayerController* SenderController = Cast<APlayerController>(GetOwner());
	if (!TargetMixin->CanControllerReceivePropertyPayload(Envelope.EntryKey, SenderController))
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning,
			TEXT("ServerRelayPropertyPayload: Blocked property payload for entry %s from controller %s due to access policy."),
			*Envelope.EntryKey.ToString(), SenderController ? *SenderController->GetName() : TEXT("<null>"));

		// Send back the server's authoritative value as a correction so the client's local
		// instance converges back to the server state instead of silently diverging.
		if (UGorgeousObjectVariable* TargetVar = TargetMixin->ResolveVariableForKey(Envelope.EntryKey))
		{
			FGorgeousAutoReplicationConditionContext CorrectionContext;
			CorrectionContext.bIsOwnerConnection = true;
			if (SenderController)
			{
				CorrectionContext.TargetController = Cast<AGorgeousPlayerController>(SenderController);
				if (UNetConnection* Conn = SenderController->GetNetConnection())
				{
					CorrectionContext.PackageMap = Conn->PackageMap;
				}
			}

			FGorgeousAutoReplicationPropertyPayload CorrectionPayload;
			if (TargetVar->BuildAutoReplicationPropertyPayload(CorrectionContext, CorrectionPayload))
			{
				FGorgeousAutoReplicationPropertyEnvelope CorrectionEnvelope;
				CorrectionEnvelope.EntryKey = Envelope.EntryKey;
				CorrectionEnvelope.Payload = CorrectionPayload;
				ClientRelayPropertyPayload(CorrectionEnvelope);
				UE_LOG(LogGorgeousAutoReplicationRelay, Log,
					TEXT("ServerRelayPropertyPayload: Sent authoritative correction for entry %s back to controller %s."),
					*Envelope.EntryKey.ToString(), SenderController ? *SenderController->GetName() : TEXT("<null>"));
			}
		}
		return;
	}

	TargetMixin->HandleTransportedPropertyPayload(Envelope);
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

	// Register the originating PC so results can be relayed back to the initiating client.
	if (QueuedRPC.RequestGuid.IsValid())
	{
		if (APlayerController* OriginPC = Cast<APlayerController>(GetOwner()))
		{
			RegisterClientInitiatedRPCOrigin(QueuedRPC.RequestGuid, OriginPC);
		}
	}

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
		// Client RPC needs to be forwarded through transporter
		TargetMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, nullptr);
		break;
	case EGorgeousAutoReplicationRPCType::EReliableMulticast:
	{
		// Route through the target mixin's own transporter with a single
		// ForwardRPCToMulticast call.  The previous per-GPS PlayerArray loop fired
		// the server-side handler once per PlayerState (3× with a listen-server +
		// 2 clients), producing duplicate "Server" tracker entries and incorrect
		// responder labels.  Using TargetMixin->RequestRPC once mirrors the path
		// taken by server-originated multicasts and is both correct and efficient.
		FGuid MulticastGuid = QueuedRPC.RequestGuid;
		TargetMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, &MulticastGuid);
		UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayRPCReliable: Forwarded reliable multicast RPC %s via target mixin."), *QueuedRPC.Key.ToString());
		break;
	}
	default:
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCReliable: Unexpected RPC type %d for RPC %s."), static_cast<int32>(QueuedRPC.Type), *QueuedRPC.Key.ToString());
		break;
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelayRPCUnreliable_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayRPCUnreliable: Server received RPC %s (Type=%d)."), *QueuedRPC.Key.ToString(), static_cast<int32>(QueuedRPC.Type));

	// Register the originating PC so results can be relayed back to the initiating client.
	if (QueuedRPC.RequestGuid.IsValid())
	{
		if (APlayerController* OriginPC = Cast<APlayerController>(GetOwner()))
		{
			RegisterClientInitiatedRPCOrigin(QueuedRPC.RequestGuid, OriginPC);
		}
	}

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
		// Client RPC needs to be forwarded through transporter
		TargetMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, nullptr);
		break;
	case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
	{
		// Same fix as the reliable path, route through target mixin's transporter
		// rather than the per-GPS PlayerArray loop.
		FGuid MulticastGuid = QueuedRPC.RequestGuid;
		TargetMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, &MulticastGuid);
		UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayRPCUnreliable: Forwarded unreliable multicast RPC %s via target mixin."), *QueuedRPC.Key.ToString());
		break;
	}
	default:
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCUnreliable: Unexpected RPC type %d for RPC %s."), static_cast<int32>(QueuedRPC.Type), *QueuedRPC.Key.ToString());
		break;
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelayResponderReadyState_Implementation(FGuid RequestGuid, const FString& ResponderKey, EGorgeousRPCReadyState ReadyState)
{
	// Update the debug tracker on the authority machine so the RPC Inspector panel
	// reflects the responder's latest state as soon as the server receives the relay.
	if (FGorgeousRPCDebugTracker::IsEnabled())
	{
		FGorgeousRPCDebugTracker::Get().OnRPCResponderReadyStateChanged(RequestGuid, ResponderKey, ReadyState);
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

void UGorgeousAutoReplicationRPCRelayComponent::ClientRelayAutoReplicationResult_Implementation(
	const FGorgeousAutoReplicationSerializedRPCResult& SerializedResult)
{
	if (!SerializedResult.RequestGuid.IsValid())
	{
		return;
	}

	// Reconstruct the result on the client side (mirrors ServerRelayAutoReplicationResult_Implementation).
	FGorgeousAutoReplicationRPCResult Result;
	Result.QueuedRPC.RequestGuid     = SerializedResult.RequestGuid;
	Result.TargetKind                = SerializedResult.TargetKind;
	Result.Responder                 = SerializedResult.Responder;
	Result.TargetVariableIdentifier  = SerializedResult.TargetVariableIdentifier;

	if (SerializedResult.OVTree.Num() > 0)
	{
		if (UGorgeousObjectVariable* ReconstructedRoot = ReconstructOVTree(SerializedResult.OVTree))
		{
			Result.TargetVariable = ReconstructedRoot;
		}
	}

	// Route to the correct client-side handler.
	if (SerializedResult.RelayedReadyState == EGorgeousRPCReadyState::ReadyForSingleResponderCallback)
	{
		UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyDeferredSingleResponderCallback(
			SerializedResult.RequestGuid, Result);
	}
	else
	{
		UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyRequestCompleted(Result);
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::SerializeResult(const FGorgeousAutoReplicationRPCResult& Result, FGorgeousAutoReplicationSerializedRPCResult& OutSerialized) const
{
	OutSerialized.RequestGuid            = Result.QueuedRPC.RequestGuid;
	OutSerialized.TargetKind             = Result.TargetKind;
	OutSerialized.Responder              = Result.Responder;
	OutSerialized.TargetVariableIdentifier = Result.TargetVariableIdentifier;

	// Capture the full OV tree rooted at the return OV so the receiver can reconstruct it
	// even when the OV does not exist in the remote registry.
	// We skip this if TargetVariable is a UGorgeousRPC_OV container (completion path on
	// the originating side), in that case we drill through to its first cached result's
	// TargetVariable, which is the actual leaf OV.
	UGorgeousObjectVariable* RootOV = Result.TargetVariable;
	if (UGorgeousRPC_OV* Container = Cast<UGorgeousRPC_OV>(RootOV))
	{
		RootOV = Container->GetCachedTargetVariable();
	}

	if (RootOV)
	{
		AppendOVTreeNodes(RootOV, FGuid{}, OutSerialized.OVTree);
	}
}

void UGorgeousAutoReplicationRPCRelayComponent::Automation_HandleRPC_WithReturnOV(
	UInteger_SOV* ReturnValue,
	int32 TestInputInt,
	const FString& TestInputString,
	int32 Sequence,
	const FString& Origin,
	const FString& Timestamp)
{
	constexpr int32 RPC_TRANSFORM_MULTIPLY = 7;
	constexpr int32 RPC_TRANSFORM_ADD = 42;

	if (ReturnValue)
	{
		ReturnValue->Value = TestInputInt * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD;
	}
	UE_LOG(LogGorgeousAutoReplicationRelay, Log,
		TEXT("[AutomationRPC] RelayComponent handler on %s: TestInputInt=%d -> ReturnOV.Value=%d seq=%d from=%s"),
		*GetName(), TestInputInt, ReturnValue ? ReturnValue->Value : -1, Sequence, *Origin);
}
