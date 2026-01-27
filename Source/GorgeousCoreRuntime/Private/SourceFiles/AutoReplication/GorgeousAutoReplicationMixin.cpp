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

#include "AutoReplication/GorgeousAutoReplicationMixin.h"

#include "AutoReplication/GorgeousAutoReplicationRPCTransporter.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "ObjectVariables/Networking/GorgeousRootNetworkStackSubsystem.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Net/UnrealNetwork.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplicationMixin, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplication, Log, All);

namespace GorgeousAutoReplicationMixin_Private
{
static UPackageMap* ResolvePackageMapForContext(UObject* ContextObject)
{
	if (!ContextObject)
	{
		return nullptr;
	}

	AActor* ContextActor = Cast<AActor>(ContextObject);
	if (!ContextActor)
	{
		ContextActor = ContextObject->GetTypedOuter<AActor>();
	}

	if (ContextActor)
	{
		if (UNetConnection* ActorConnection = ContextActor->GetNetConnection())
		{
			return ActorConnection->PackageMap;
		}

		if (UWorld* ActorWorld = ContextActor->GetWorld())
		{
			if (UNetDriver* NetDriver = ActorWorld->GetNetDriver())
			{
				if (NetDriver->ServerConnection)
				{
					return NetDriver->ServerConnection->PackageMap;
				}
			}
		}
	}
	else if (UWorld* ContextWorld = ContextObject->GetWorld())
	{
		if (UNetDriver* NetDriver = ContextWorld->GetNetDriver())
		{
			if (NetDriver->ServerConnection)
			{
				return NetDriver->ServerConnection->PackageMap;
			}
		}
	}

	return nullptr;
}

static APlayerController* ResolveLocalPlayerController(UObject* ContextObject)
{
	if (!ContextObject)
	{
		return nullptr;
	}

	UWorld* ContextWorld = ContextObject->GetWorld();
	if (!ContextWorld)
	{
		return nullptr;
	}

	return ContextWorld->GetFirstPlayerController();
}

static void RelayResultToAuthority(UObject* ContextObject, const FGorgeousAutoReplicationRPCResult& Result)
{
	if (!ContextObject || !Result.QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	if (APlayerController* LocalController = ResolveLocalPlayerController(ContextObject))
	{
		if (UGorgeousAutoReplicationRPCRelayComponent* Relay = LocalController->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>())
		{
			Relay->RelayResultToServer(Result);
		}
	}
}

static FGuid ResolveVariableInterfaceIdentifier(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return FGuid();
	}

	if (!Variable->GetClass()->ImplementsInterface(UGorgeousObjectVariableInteraction_I::StaticClass()))
	{
		return FGuid();
	}

	return IGorgeousObjectVariableInteraction_I::Execute_GetUniqueIdentifierForObjectVariable(Variable);
}

static FGuid EnsureVariableIdentifier(UGorgeousObjectVariable* Variable, const FGuid& PreferredIdentifier = FGuid())
{
	if (!Variable)
	{
		return FGuid();
	}

	FGuid Identifier = PreferredIdentifier;
	if (!Identifier.IsValid())
	{
		Identifier = ResolveVariableInterfaceIdentifier(Variable);
	}
	if (!Identifier.IsValid())
	{
		Identifier = FGuid::NewGuid();
	}

	Variable->ApplyReplicatedIdentifier(Identifier);
	return Identifier;
}
}

FGorgeousAutoReplicationHandle::FGorgeousAutoReplicationHandle()
	: ReplicationIndex(InvalidReplicationIndex)
	, RegisteredKey(NAME_None)
	, CachedValue(nullptr)
{
}

void FGorgeousAutoReplicationHandle::Reset()
{
	ReplicationIndex = InvalidReplicationIndex;
	RegisteredKey = NAME_None;
	CachedValue = nullptr;
	OwningObject.Reset();
}

void FGorgeousAutoReplicationHandle::Assign(const FName InKey, const uint16 InReplicationIndex, UObject* InOwner)
{
	RegisteredKey = InKey;
	ReplicationIndex = InReplicationIndex;
	OwningObject = InOwner;
}

void FGorgeousAutoReplicationHandle::CacheValue(UGorgeousObjectVariable* InValue, const FGorgeousAutoReplicationStreamConfig* StreamOverride, const bool bAutoEnableNetworking)
{
	CachedValue = InValue;
	if (CachedValue)
	{
		const int32 ResolvedIndex = (ReplicationIndex == InvalidReplicationIndex) ? INDEX_NONE : static_cast<int32>(ReplicationIndex);
		CachedValue->SetAutoReplicationBinding(OwningObject.Get(), RegisteredKey, ResolvedIndex, StreamOverride, bAutoEnableNetworking);
	}
}

FGorgeousAutoReplicationEntry::FGorgeousAutoReplicationEntry()
	: DefaultValue(nullptr)
	, bReplicate(false)
	, bOverrideStreamConfig(false)
{
}

FGorgeousReplicatedVariableEntry::FGorgeousReplicatedVariableEntry()
	: ReplicationIndex(FGorgeousAutoReplicationHandle::InvalidReplicationIndex)
	, Key(NAME_None)
	, Value(nullptr)
	, VariableIdentifier()
	, bIsActive(false)
{
}

FGorgeousAutoReplicationMixin::FGorgeousAutoReplicationMixin()
	: AdditionalData(nullptr)
	, ReplicatedVariables(nullptr)
	, bNetworkingEnabled(false)
	, bIsBound(false)
{
}

void FGorgeousAutoReplicationMixin::Bind(
	UObject* InOwner,
	TMap<FName, FGorgeousAutoReplicationEntry>* InAdditionalData,
	TArray<FGorgeousReplicatedVariableEntry>* InReplicatedVariables)
{
	Owner = InOwner;
	AdditionalData = InAdditionalData;
	ReplicatedVariables = InReplicatedVariables;
	bIsBound = (Owner.IsValid() && AdditionalData != nullptr && ReplicatedVariables != nullptr);
	InitializeTransporter();
}

void FGorgeousAutoReplicationMixin::InitializeAdditionalData(const bool bActivateNetworkingCapabilities)
{
	EnsureBound();

	bNetworkingEnabled = bActivateNetworkingCapabilities;

	if (!AdditionalData)
	{
		return;
	}

	if (!bNetworkingEnabled)
	{
		KeyToReplicationIndex.Empty();
		if (ReplicatedVariables)
		{
			ReplicatedVariables->Reset();
		}

		UObject* OwnerObject = Owner.Get();
		for (auto& Pair : *AdditionalData)
		{
			FGorgeousAutoReplicationEntry& Entry = Pair.Value;
			Entry.Handle.Reset();
			Entry.Handle.Assign(Pair.Key, FGorgeousAutoReplicationHandle::InvalidReplicationIndex, OwnerObject);
			const FGorgeousAutoReplicationStreamConfig* StreamOverride = Entry.bOverrideStreamConfig ? &Entry.StreamConfigOverride : nullptr;
			Entry.Handle.CacheValue(Entry.DefaultValue, StreamOverride, Entry.bReplicate);
		}

		return;
	}

	if (ReplicatedVariables)
	{
		ReplicatedVariables->Reset();
	}

	KeyToReplicationIndex.Empty(AdditionalData->Num());

	UObject* OwnerObject = Owner.Get();
	for (auto& Pair : *AdditionalData)
	{
		FGorgeousAutoReplicationEntry& Entry = Pair.Value;
		UGorgeousObjectVariable* DefaultVar = Entry.DefaultValue;
		const bool bEntrySupportsAutoReplication = !DefaultVar || DefaultVar->SupportsAutoReplicationFeatures();
		if (!Entry.bReplicate || !bEntrySupportsAutoReplication)
		{
			Entry.Handle.Reset();
			Entry.Handle.Assign(Pair.Key, FGorgeousAutoReplicationHandle::InvalidReplicationIndex, OwnerObject);
			const FGorgeousAutoReplicationStreamConfig* StreamOverride = Entry.bOverrideStreamConfig ? &Entry.StreamConfigOverride : nullptr;
			Entry.Handle.CacheValue(Entry.DefaultValue, StreamOverride, Entry.bReplicate);
			GorgeousAutoReplicationMixin_Private::EnsureVariableIdentifier(Entry.DefaultValue);
			continue;
		}

		const uint16 RepIndex = GetOrAssignReplicationIndex(Pair.Key);
		Entry.Handle.Assign(Pair.Key, RepIndex, OwnerObject);
		const FGorgeousAutoReplicationStreamConfig* StreamOverride = Entry.bOverrideStreamConfig ? &Entry.StreamConfigOverride : nullptr;
		Entry.Handle.CacheValue(Entry.DefaultValue, StreamOverride, Entry.bReplicate);
		const FGuid EntryIdentifier = GorgeousAutoReplicationMixin_Private::EnsureVariableIdentifier(Entry.DefaultValue);

		if (ReplicatedVariables)
		{
			FGorgeousReplicatedVariableEntry NewEntry;
			NewEntry.Key = Pair.Key;
			NewEntry.ReplicationIndex = RepIndex;
			NewEntry.Value = Entry.DefaultValue;
			NewEntry.VariableIdentifier = EntryIdentifier;
			NewEntry.bIsActive = true;
			ReplicatedVariables->Add(NewEntry);
		}
	}
}

FGorgeousAutoReplicationEntry* FGorgeousAutoReplicationMixin::FindEntry(const FName Key)
{
	EnsureBound();
	return AdditionalData ? AdditionalData->Find(Key) : nullptr;
}

const FGorgeousAutoReplicationEntry* FGorgeousAutoReplicationMixin::FindEntry(const FName Key) const
{
	return AdditionalData ? AdditionalData->Find(Key) : nullptr;
}

bool FGorgeousAutoReplicationMixin::TrySetReplicatedValue(const FName Key, UGorgeousObjectVariable* NewValue)
{
	EnsureBound();

	if (!bNetworkingEnabled || !ReplicatedVariables)
	{
		return false;
	}

	if (FGorgeousReplicatedVariableEntry* RepEntry = FindReplicatedEntryByKey(Key))
	{
		RepEntry->Value = NewValue;
		RepEntry->bIsActive = (NewValue != nullptr);
		RepEntry->VariableIdentifier = GorgeousAutoReplicationMixin_Private::EnsureVariableIdentifier(NewValue);

		if (FGorgeousAutoReplicationEntry* Entry = FindEntry(Key))
		{
			const FGorgeousAutoReplicationStreamConfig* StreamOverride = Entry->bOverrideStreamConfig ? &Entry->StreamConfigOverride : nullptr;
			Entry->Handle.CacheValue(NewValue, StreamOverride, Entry->bReplicate);
		}

#if GORGEOUSCORE_WITH_IRIS
		if (NewValue)
		{
			if (UObject* OwnerObject = Owner.Get())
			{
				if (UWorld* World = OwnerObject->GetWorld())
				{
					if (World->GetNetMode() != NM_Client)
					{
						FGorgeousAutoReplicationCoordinator::Get(World).MarkStreamDirty(NewValue);
					}
				}
			}
		}
#endif

		return true;
	}

	UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("Failed to set replicated value for key %s - no replication slot registered."), *Key.ToString());
	return false;
}

bool FGorgeousAutoReplicationMixin::TryGetValue(const FName Key, UGorgeousObjectVariable*& OutValue) const
{
	if (const FGorgeousAutoReplicationEntry* Entry = FindEntry(Key))
	{
		if (Entry->Handle.GetCachedValue())
		{
			OutValue = Entry->Handle.GetCachedValue();
			return true;
		}
	}

	if (const FGorgeousReplicatedVariableEntry* RepEntry = FindReplicatedEntryByKey(Key))
	{
		OutValue = RepEntry->Value;
		return OutValue != nullptr;
	}

	OutValue = nullptr;
	return false;
}

uint16 FGorgeousAutoReplicationMixin::GetOrAssignReplicationIndex(const FName Key)
{
	EnsureBound();

	if (const uint16* Existing = KeyToReplicationIndex.Find(Key))
	{
		return *Existing;
	}

	const uint16 NewIndex = static_cast<uint16>(KeyToReplicationIndex.Num());
	if (ensureMsgf(NewIndex != FGorgeousAutoReplicationHandle::InvalidReplicationIndex, TEXT("AutoReplication replication index overflow")))
	{
		KeyToReplicationIndex.Add(Key, NewIndex);
	}

	return NewIndex;
}

void FGorgeousAutoReplicationMixin::RefreshCachedValues()
{
	EnsureBound();

	if (!ReplicatedVariables || !AdditionalData)
	{
		return;
	}

	UObject* OwnerObject = Owner.Get();
	for (FGorgeousReplicatedVariableEntry& RepEntry : *ReplicatedVariables)
	{
		if (FGorgeousAutoReplicationEntry* Entry = AdditionalData->Find(RepEntry.Key))
		{
			Entry->Handle.Assign(RepEntry.Key, RepEntry.ReplicationIndex, OwnerObject);
			const FGorgeousAutoReplicationStreamConfig* StreamOverride = Entry->bOverrideStreamConfig ? &Entry->StreamConfigOverride : nullptr;
			Entry->Handle.CacheValue(RepEntry.Value, StreamOverride, Entry->bReplicate);
		}

		if (RepEntry.Value)
		{
			const FGuid AppliedIdentifier = GorgeousAutoReplicationMixin_Private::EnsureVariableIdentifier(RepEntry.Value, RepEntry.VariableIdentifier);
			RepEntry.VariableIdentifier = AppliedIdentifier;
		}
		else
		{
			RepEntry.VariableIdentifier.Invalidate();
		}
	}
}

bool FGorgeousAutoReplicationMixin::RequestRPC(const FName Key, const EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, const EGorgeousAutoReplicationTargetKind TargetKind, FGuid* OutRequestGuid)
{
	EnsureBound();

	if (!bNetworkingEnabled)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("Attempted to queue AutoReplication RPC for key %s while networking is disabled."), *Key.ToString());
		return false;
	}

	EGorgeousAutoReplicationTargetKind ResolvedKind = TargetKind;
	UGorgeousObjectVariable* ResolvedVariable = nullptr;
	UObject* ResolvedOwner = nullptr;
	if (!ResolveRPCDestination(Key, TargetKind, ResolvedKind, ResolvedVariable, ResolvedOwner))
	{
		if (TargetKind == EGorgeousAutoReplicationTargetKind::EObjectVariable)
		{
			UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication RPC for key %s rejected because no eligible object variable is registered."), *Key.ToString());
		}
		else if (TargetKind == EGorgeousAutoReplicationTargetKind::EOwner)
		{
			UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication RPC for key %s rejected because the owning QoL object is unavailable."), *Key.ToString());
		}
		else
		{
			UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication RPC for key %s rejected because neither an object variable nor owner handler is available."), *Key.ToString());
		}
		return false;
	}

	FGorgeousQueuedRPC QueuedRPC;
	QueuedRPC.Key = Key;
	QueuedRPC.Type = Type;
	QueuedRPC.TargetKind = ResolvedKind;
	QueuedRPC.Payload = Payload;
	const bool bHasPreallocatedGuid = (OutRequestGuid && OutRequestGuid->IsValid());
	QueuedRPC.RequestGuid = bHasPreallocatedGuid ? *OutRequestGuid : FGuid::NewGuid();

	if (OutRequestGuid)
	{
		*OutRequestGuid = QueuedRPC.RequestGuid;
	}

	UGorgeousAutoReplicationRPCTransporter* ActiveTransporter = RPCTransporter.Get();
	if (!ActiveTransporter)
	{
		InitializeTransporter();
		ActiveTransporter = RPCTransporter.Get();
	}

	if (ActiveTransporter && ActiveTransporter->RouteRPC(QueuedRPC))
	{
		const UEnum* TypeEnum = StaticEnum<EGorgeousAutoReplicationRPCType>();
		const FString TypeString = TypeEnum ? TypeEnum->GetNameStringByValue(static_cast<int64>(Type)) : FString(TEXT("<unknown>"));
		const UEnum* TargetEnum = StaticEnum<EGorgeousAutoReplicationTargetKind>();
		const FString TargetString = TargetEnum ? TargetEnum->GetNameStringByValue(static_cast<int64>(QueuedRPC.TargetKind)) : FString(TEXT("<unknown>"));
		const FString TargetLabel = (QueuedRPC.TargetKind == EGorgeousAutoReplicationTargetKind::EObjectVariable && ResolvedVariable)
			? ResolvedVariable->GetName()
			: (QueuedRPC.TargetKind == EGorgeousAutoReplicationTargetKind::EOwner && ResolvedOwner)
				? ResolvedOwner->GetName()
				: FString(TEXT("<unresolved>"));
		UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("Dispatched AutoReplication RPC (%s -> %s:%s) for key %s through transporter on %s."), *TypeString, *TargetString, *TargetLabel, *Key.ToString(), Owner.IsValid() ? *Owner->GetName() : TEXT("<invalid>"));
		return true;
	}

	if (ActiveTransporter)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication RPC transporter failed to route request for key %s. Aborting send."), *Key.ToString());
		return false;
	}

	UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("AutoReplication RPC transporter unavailable for key %s. Executing locally."), *Key.ToString());
	return EnqueueRPCInternal(QueuedRPC);
}

bool FGorgeousAutoReplicationMixin::ExecuteAutoReplicationRPC(const FGorgeousQueuedRPC& QueuedRPC)
{
	EnsureBound();

	auto EmitResult = [this, &QueuedRPC](const EGorgeousAutoReplicationTargetKind TargetKind, UGorgeousObjectVariable* TargetVariable, UObject* TargetOwner)
	{
		FGorgeousAutoReplicationRPCResult Result;
		Result.QueuedRPC = QueuedRPC;
		Result.TargetKind = TargetKind;
		Result.TargetVariable = TargetVariable;
		Result.TargetOwner = TargetOwner;
		Result.TargetVariableIdentifier = GorgeousAutoReplicationMixin_Private::ResolveVariableInterfaceIdentifier(TargetVariable);

		if (UGorgeousRPC_OV* RPCContainer = Cast<UGorgeousRPC_OV>(TargetVariable))
		{
			if (!RPCContainer->HasResult())
			{
				RPCContainer->CaptureResult(Result);
			}
		}

		if (IsAuthorityContext())
		{
			Result.Responder = FGorgeousAutoReplicationRPCResponderHandle::MakeServerHandle();
		}
		else if (UObject* OwnerObject = ResolveOwnerObject())
		{
			if (APlayerController* LocalController = GorgeousAutoReplicationMixin_Private::ResolveLocalPlayerController(OwnerObject))
			{
				Result.Responder = FGorgeousAutoReplicationRPCResponderHandle::FromController(LocalController);
			}
		}

		if (!Result.Responder.IsValid())
		{
			Result.Responder = FGorgeousAutoReplicationRPCResponderHandle::MakeServerHandle();
		}

		if (QueuedRPC.RequestGuid.IsValid() && Result.Responder.IsValid())
		{
			UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterExpectedResponder(QueuedRPC.RequestGuid, Result.Responder);
		}

		UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyRequestCompleted(Result);

		if (!IsAuthorityContext())
		{
			if (UObject* OwnerObject = ResolveOwnerObject())
			{
				GorgeousAutoReplicationMixin_Private::RelayResultToAuthority(OwnerObject, Result);
			}
		}
	};

	UObject* OwnerObject = ResolveOwnerObject();
	auto ExecuteOnOwner = [&](const EGorgeousAutoReplicationTargetKind TargetKindOverride) -> bool
	{
		if (!OwnerObject)
		{
			return false;
		}

		UGorgeousObjectVariable* ReturnContainer = nullptr;
		const bool bHandledByOwner = UGorgeousObjectVariable::InvokeNativeAutoReplicationRPCHandlerOnObject(OwnerObject, QueuedRPC, &ReturnContainer);
		if (bHandledByOwner)
		{
			EmitResult(TargetKindOverride, ReturnContainer, OwnerObject);
		}
		return bHandledByOwner;
	};

	auto ExecuteOnVariable = [&](UGorgeousObjectVariable* TargetVariable) -> bool
	{
		UGorgeousObjectVariable* ReturnContainer = nullptr;
		if (TargetVariable && TargetVariable->ExecuteAutoReplicationRPC(QueuedRPC, &ReturnContainer))
		{
			UGorgeousObjectVariable* ResultVariable = ReturnContainer ? ReturnContainer : TargetVariable;
			EmitResult(EGorgeousAutoReplicationTargetKind::EObjectVariable, ResultVariable, nullptr);
			return true;
		}
		return false;
	};

	const FGorgeousAutoReplicationEntry* Entry = AdditionalData ? AdditionalData->Find(QueuedRPC.Key) : nullptr;
	UGorgeousObjectVariable* TargetVariable = nullptr;
	if (Entry)
	{
		TargetVariable = Entry->Handle.GetCachedValue();
		if (!TargetVariable)
		{
			TargetVariable = Entry->DefaultValue;
		}
	}

	switch (QueuedRPC.TargetKind)
	{
	case EGorgeousAutoReplicationTargetKind::EObjectVariable:
	{
		const bool bVariableHandled = ExecuteOnVariable(TargetVariable);
		if (!bVariableHandled)
		{
			const FString OwnerName = Owner.IsValid() ? Owner->GetName() : TEXT("<invalid>");
			if (TargetVariable)
			{
				UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("%s failed to execute AutoReplication RPC handler %s on %s."),
					*OwnerName, *QueuedRPC.Payload.HandlerName.ToString(), *TargetVariable->GetName());
			}
			else
			{
				UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication RPC %s expected an object variable target but none was registered."), *QueuedRPC.Key.ToString());
			}
		}
		return bVariableHandled;
	}
	case EGorgeousAutoReplicationTargetKind::EOwner:
	{
		const bool bOwnerHandled = ExecuteOnOwner(EGorgeousAutoReplicationTargetKind::EOwner);
		if (!bOwnerHandled)
		{
			const FString OwnerName = Owner.IsValid() ? Owner->GetName() : TEXT("<invalid>");
			UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication RPC %s expected the owning QoL object to handle %s but no handler was found on %s."),
				*QueuedRPC.Key.ToString(), *QueuedRPC.Payload.HandlerName.ToString(), *OwnerName);
		}
		return bOwnerHandled;
	}
	default:
	{
		if (ExecuteOnVariable(TargetVariable))
		{
			return true;
		}

		const bool bOwnerHandled = ExecuteOnOwner(EGorgeousAutoReplicationTargetKind::EOwner);
		if (!bOwnerHandled)
		{
			const FString OwnerName = Owner.IsValid() ? Owner->GetName() : TEXT("<invalid>");
			UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication RPC %s could not resolve a valid handler on either the object variable or owner (%s)."), *QueuedRPC.Key.ToString(), *OwnerName);
		}
		return bOwnerHandled;
	}
	}
}

bool FGorgeousAutoReplicationMixin::DequeuePendingRPC(FGorgeousQueuedRPC& OutRPC)
{
	EnsureBound();

	if (PendingRPCs.Num() == 0)
	{
		return false;
	}

	OutRPC = PendingRPCs[0];
	PendingRPCs.RemoveAt(0);
	return true;
}

void FGorgeousAutoReplicationMixin::DispatchPendingRPCs()
{
	EnsureBound();

	UObject* OwnerObject = Owner.Get();
	if (!OwnerObject)
	{
		return;
	}

	if (!OwnerObject->GetClass()->ImplementsInterface(UGorgeousAutoReplicationRPCResponder_I::StaticClass()))
	{
		return;
	}

	while (PendingRPCs.Num() > 0)
	{
		FGorgeousQueuedRPC RPC = PendingRPCs[0];
		PendingRPCs.RemoveAt(0);
		IGorgeousAutoReplicationRPCResponder_I::Execute_HandleAutoReplicationRPC(OwnerObject, RPC);
	}
}

int32 FGorgeousAutoReplicationMixin::GetReplicatedEntryCount() const
{
	return ReplicatedVariables ? ReplicatedVariables->Num() : 0;
}

void FGorgeousAutoReplicationMixin::InitializeTransporter()
{
	UObject* OwnerObject = Owner.Get();
	if (!OwnerObject)
	{
		return;
	}

	AActor* OwnerActor = Cast<AActor>(OwnerObject);
	if (!OwnerActor)
	{
		if (AGorgeousGameState* GameState = Cast<AGorgeousGameState>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(OwnerObject, AGorgeousGameState::StaticClass())))
		{
			OwnerActor = GameState;
		}
		else if (AGorgeousGameMode* GameMode = Cast<AGorgeousGameMode>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(OwnerObject, AGorgeousGameMode::StaticClass())))
		{
			OwnerActor = GameMode;
		}
		else if (AGorgeousWorldSettings* WorldSettings = UGorgeousCoreRuntimeGlobals::GetGorgeousWorldSettings(OwnerObject))
		{
			OwnerActor = WorldSettings;
		}
	}

	if (!OwnerActor)
	{
		UE_LOG(LogGorgeousAutoReplication, Verbose, TEXT("OwnerActor is null in InitializeTransporter. RPCTransporter will be reset."));
		RPCTransporter.Reset();
		return;
	}

	if (!OwnerActor->GetWorld())
	{
		UE_LOG(LogGorgeousAutoReplication, Verbose, TEXT("OwnerActor or its World is invalid. Initialization deferred."));
		return;
	}

	UGorgeousAutoReplicationRPCTransporter* TransporterInstance = nullptr;
	for (TInlineComponentArray<UGorgeousAutoReplicationRPCTransporter*> TransportComponents(OwnerActor); UGorgeousAutoReplicationRPCTransporter* Candidate : TransportComponents)
	{
		if (Candidate && Candidate->IsLinkedToMixin(this))
		{
			TransporterInstance = Candidate;
			break;
		}
	}
	if (!TransporterInstance)
	{
		TransporterInstance = OwnerActor->CreateDefaultSubobject<UGorgeousAutoReplicationRPCTransporter>(TEXT("AutoReplicationTransporter"));
		if (TransporterInstance)
		{
			OwnerActor->AddInstanceComponent(TransporterInstance);
			TransporterInstance->RegisterComponent();
		}
	}

	if (TransporterInstance)
	{
		TransporterInstance->InitializeTransporter(this);
		RPCTransporter = TransporterInstance;
	}
}

bool FGorgeousAutoReplicationMixin::EnqueueRPCInternal(const FGorgeousQueuedRPC& QueuedRPC)
{
	FGorgeousQueuedRPC& Pending = PendingRPCs.AddDefaulted_GetRef();
	Pending = QueuedRPC;

	const UEnum* EnumClass = StaticEnum<EGorgeousAutoReplicationRPCType>();
	const FString TypeString = EnumClass ? EnumClass->GetNameStringByValue(static_cast<int64>(QueuedRPC.Type)) : FString(TEXT("<unknown>"));
	UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("Queued AutoReplication RPC (%s) for key %s on %s."), *TypeString, *QueuedRPC.Key.ToString(), Owner.IsValid() ? *Owner->GetName() : TEXT("<invalid>"));

	DispatchPendingRPCs();
	return true;
}

bool FGorgeousAutoReplicationMixin::HandleTransportedRPC(const FGorgeousQueuedRPC& QueuedRPC)
{
	return EnqueueRPCInternal(QueuedRPC);
}

void FGorgeousAutoReplicationMixin::HandleTransportedPropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope)
{
	EnsureBound();

	if (!bNetworkingEnabled)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("Dropping AutoReplication property payload for entry %s because networking is disabled."), *Envelope.EntryKey.ToString());
		return;
	}

	if (!AdditionalData)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication property payload for entry %s could not be delivered because AdditionalData is unavailable."), *Envelope.EntryKey.ToString());
		return;
	}

	FGorgeousAutoReplicationEntry* TargetEntry = AdditionalData->Find(Envelope.EntryKey);
	if (!TargetEntry)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("AutoReplication property payload references unknown entry %s. Payload discarded."), *Envelope.EntryKey.ToString());
		return;
	}

	if (!TargetEntry->bReplicate)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("AutoReplication property payload for entry %s was ignored because the entry is not marked for replication."), *Envelope.EntryKey.ToString());
		return;
	}

	FGorgeousReplicatedVariableEntry* ReplicatedEntry = ReplicatedVariables ? FindReplicatedEntryByKey(Envelope.EntryKey) : nullptr;
	UGorgeousObjectVariable* TargetVariable = TargetEntry->Handle.GetCachedValue();
	if (!TargetVariable && ReplicatedEntry)
	{
		TargetVariable = ReplicatedEntry->Value;
	}
	if (!TargetVariable)
	{
		TargetVariable = TargetEntry->DefaultValue;
	}

	if (!TargetVariable)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("AutoReplication property payload for entry %s has no target variable to apply changes."), *Envelope.EntryKey.ToString());
		return;
	}

	if (!TargetVariable->SupportsAutoReplicationFeatures())
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Verbose, TEXT("AutoReplication property payload for entry %s targeted %s but the variable is configured for manual networking."),
			*Envelope.EntryKey.ToString(), *TargetVariable->GetName());
		return;
	}

	if (Envelope.Payload.IsEmpty())
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, VeryVerbose, TEXT("AutoReplication property payload for entry %s contained no serialized properties."), *Envelope.EntryKey.ToString());
		return;
	}

	const FGuid PreferredIdentifier = ReplicatedEntry ? ReplicatedEntry->VariableIdentifier : FGuid();
	const FGuid TargetIdentifier = GorgeousAutoReplicationMixin_Private::EnsureVariableIdentifier(TargetVariable, PreferredIdentifier);

	const FGorgeousAutoReplicationStreamConfig* StreamOverride = TargetEntry->bOverrideStreamConfig ? &TargetEntry->StreamConfigOverride : nullptr;
	if (TargetEntry->Handle.GetCachedValue() != TargetVariable)
	{
		TargetEntry->Handle.CacheValue(TargetVariable, StreamOverride, TargetEntry->bReplicate);
	}

	if (ReplicatedEntry)
	{
		ReplicatedEntry->Value = TargetVariable;
		ReplicatedEntry->bIsActive = (TargetVariable != nullptr);
		ReplicatedEntry->VariableIdentifier = TargetVariable ? TargetIdentifier : FGuid();
	}

	UPackageMap* PackageMap = GorgeousAutoReplicationMixin_Private::ResolvePackageMapForContext(ResolveOwnerObject());
	if (!PackageMap)
	{
		PackageMap = GorgeousAutoReplicationMixin_Private::ResolvePackageMapForContext(TargetVariable);
	}
	if (!PackageMap)
	{
		if (UGorgeousAutoReplicationRPCTransporter* Transporter = RPCTransporter.Get())
		{
			PackageMap = GorgeousAutoReplicationMixin_Private::ResolvePackageMapForContext(Transporter->GetOwner());
		}
	}

	TargetVariable->PreNetReceive();
	const bool bApplied = TargetVariable->ApplyAutoReplicationPropertyPayload(Envelope.Payload, PackageMap);
	TargetVariable->PostNetReceive();
	if (!bApplied)
	{
		UE_LOG(LogGorgeousAutoReplicationMixin, Warning, TEXT("AutoReplication property payload for entry %s on %s failed to deserialize."),
			*Envelope.EntryKey.ToString(), *TargetVariable->GetName());
		return;
	}

	const FString StreamId = Envelope.Payload.StreamGuid.IsValid() ? Envelope.Payload.StreamGuid.ToString() : TEXT("<invalid>");
	UE_LOG(LogGorgeousAutoReplicationMixin, VeryVerbose,
		TEXT("Applied AutoReplication property payload for entry %s on %s (%d properties, stream %s)."),
		*Envelope.EntryKey.ToString(), *TargetVariable->GetName(), Envelope.Payload.Properties.Num(), *StreamId);
}

void FGorgeousAutoReplicationMixin::EnsureBound() const
{
	ensureAlwaysMsgf(bIsBound, TEXT("FGorgeousAutoReplicationMixin used before Bind() was called."));
}

UObject* FGorgeousAutoReplicationMixin::ResolveOwnerObject() const
{
	return Owner.Get();
}

bool FGorgeousAutoReplicationMixin::IsAuthorityContext() const
{
	UObject* OwnerObject = Owner.Get();
	if (!OwnerObject)
	{
		return false;
	}

	if (const AActor* OwnerActor = Cast<AActor>(OwnerObject))
	{
		return OwnerActor->HasAuthority();
	}

	if (const UWorld* OwnerWorld = OwnerObject->GetWorld())
	{
		return OwnerWorld->GetNetMode() != NM_Client;
	}

	return false;
}

FGorgeousReplicatedVariableEntry* FGorgeousAutoReplicationMixin::FindReplicatedEntryByKey(const FName Key) const
{
	if (!ReplicatedVariables)
	{
		return nullptr;
	}

	for (FGorgeousReplicatedVariableEntry& Entry : *ReplicatedVariables)
	{
		if (Entry.Key == Key)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool FGorgeousAutoReplicationMixin::ResolveRPCDestination(const FName Key, const EGorgeousAutoReplicationTargetKind RequestedKind, EGorgeousAutoReplicationTargetKind& OutResolvedKind, UGorgeousObjectVariable*& OutVariable, UObject*& OutOwner) const
{
	OutResolvedKind = RequestedKind;
	OutVariable = nullptr;
	OutOwner = nullptr;

	UGorgeousObjectVariable* CandidateVariable = ResolveVariableForKey(Key);

	UObject* OwnerObject = ResolveOwnerObject();

	switch (RequestedKind)
	{
	case EGorgeousAutoReplicationTargetKind::EObjectVariable:
	{
		if (!CandidateVariable)
		{
			return false;
		}

		OutResolvedKind = EGorgeousAutoReplicationTargetKind::EObjectVariable;
		OutVariable = CandidateVariable;
		return true;
	}
	case EGorgeousAutoReplicationTargetKind::EOwner:
	{
		if (!OwnerObject)
		{
			return false;
		}

		OutResolvedKind = EGorgeousAutoReplicationTargetKind::EOwner;
		OutOwner = OwnerObject;
		return true;
	}
	default:
	{
		if (CandidateVariable)
		{
			OutResolvedKind = EGorgeousAutoReplicationTargetKind::EObjectVariable;
			OutVariable = CandidateVariable;
			return true;
		}

		if (OwnerObject)
		{
			OutResolvedKind = EGorgeousAutoReplicationTargetKind::EOwner;
			OutOwner = OwnerObject;
			return true;
		}

		return false;
	}
	}
}

UGorgeousObjectVariable* FGorgeousAutoReplicationMixin::ResolveVariableForKey(const FName Key) const
{
	if (!AdditionalData)
	{
		return nullptr;
	}

	const FGorgeousAutoReplicationEntry* Entry = AdditionalData->Find(Key);
	if (!Entry)
	{
		return nullptr;
	}

	if (UGorgeousObjectVariable* Cached = Entry->Handle.GetCachedValue())
	{
		return Cached;
	}

	return Entry->DefaultValue;
}

bool FGorgeousAutoReplicationMixin::CanControllerReceiveRPC(const FGorgeousQueuedRPC& QueuedRPC, AController* Controller) const
{
	if (!Controller)
	{
		return true;
	}

	UGorgeousObjectVariable* TargetVariable = ResolveVariableForKey(QueuedRPC.Key);
	if (!TargetVariable || !TargetVariable->ShouldUseRootNetworkStack())
	{
		return true;
	}

	AutoSubscribeControllerForVariable(Controller, TargetVariable);

	if (UWorld* VariableWorld = TargetVariable->GetWorld())
	{
		if (UGorgeousRootNetworkStackSubsystem* RootNetworkStack = UGorgeousRootNetworkStackSubsystem::Get(VariableWorld))
		{
			return RootNetworkStack->CanControllerAccess(TargetVariable, Controller);
		}
	}

	return TargetVariable->EvaluateAccessPolicyForController(Controller);
}

bool FGorgeousAutoReplicationMixin::CanControllerReceivePropertyPayload(const FName EntryKey, AController* Controller) const
{
	if (!Controller)
	{
		return true;
	}

	UGorgeousObjectVariable* TargetVariable = ResolveVariableForKey(EntryKey);
	if (!TargetVariable || !TargetVariable->ShouldUseRootNetworkStack())
	{
		return true;
	}

	AutoSubscribeControllerForVariable(Controller, TargetVariable);

	if (UWorld* VariableWorld = TargetVariable->GetWorld())
	{
		if (UGorgeousRootNetworkStackSubsystem* RootNetworkStack = UGorgeousRootNetworkStackSubsystem::Get(VariableWorld))
		{
			return RootNetworkStack->CanControllerAccess(TargetVariable, Controller);
		}
	}

	return TargetVariable->EvaluateAccessPolicyForController(Controller);
}

void FGorgeousAutoReplicationMixin::AutoSubscribeControllerForVariable(AController* Controller, UGorgeousObjectVariable* TargetVariable) const
{
	if (!Controller || !TargetVariable)
	{
		return;
	}

	if (!TargetVariable->ShouldUseRootNetworkStack())
	{
		return;
	}

	if (UWorld* VariableWorld = TargetVariable->GetWorld())
	{
		if (UGorgeousRootNetworkStackSubsystem* RootNetworkStack = UGorgeousRootNetworkStackSubsystem::Get(VariableWorld))
		{
			RootNetworkStack->TryAutoSubscribeController(TargetVariable, Controller);
		}
	}
}
