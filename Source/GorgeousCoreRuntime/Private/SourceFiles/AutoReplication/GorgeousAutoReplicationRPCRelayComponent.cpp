#include "UObject/NoExportTypes.h"
#include "Misc/OutputDeviceNull.h"
#include "UObject/StrongObjectPtr.h"
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
#include "GeneralSystems/VisualDataGathering/GorgeousVDGConsent.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"

#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformTime.h"
#include "UObject/UnrealType.h"
#include "AutoReplication/Tracking/GorgeousRPCDebugTracker.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplicationRelay, Log, All);

// Server-side map: RequestGuid → originating client PC (populated in ServerRelayRPC*_Implementation).
// Entries remain valid as long as the PC weak ptr is live; stale entries are lazily removed.
TMap<FGuid, UGorgeousAutoReplicationRPCRelayComponent::FClientRPCOrigin> UGorgeousAutoReplicationRPCRelayComponent::ClientInitiatedRPCOrigins;

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

    constexpr int32 MaxResultTreeNodes=1024;
    constexpr int32 MaxResultTreeDepth=64;
    constexpr int32 MaxResultTreeProperties=8192;
    constexpr int64 MaxResultTreeBytes=1024*1024;

    // ImportText may invoke native struct parsers, resolve object paths, and load assets.
    // This PackageMap-free result format accepts only value types with known-safe parsers.
    static bool IsSafeResultProperty(const FProperty* Property,int32 Depth=0)
    {
        if (!Property || Depth>32) return false;
        if (Property->IsA<FNumericProperty>() || Property->IsA<FBoolProperty>() || Property->IsA<FEnumProperty>() ||
            Property->IsA<FStrProperty>() || Property->IsA<FNameProperty>() || Property->IsA<FTextProperty>()) return true;
        if (const auto* Array=CastField<FArrayProperty>(Property)) return IsSafeResultProperty(Array->Inner,Depth+1);
        if (const auto* Set=CastField<FSetProperty>(Property)) return IsSafeResultProperty(Set->ElementProp,Depth+1);
        if (const auto* Map=CastField<FMapProperty>(Property)) return IsSafeResultProperty(Map->KeyProp,Depth+1) && IsSafeResultProperty(Map->ValueProp,Depth+1);
        if (const auto* Struct=CastField<FStructProperty>(Property))
        {
            UScriptStruct* Type=Struct->Struct;
            if (!Type) return false;
            const bool KnownValueParser=Type==TBaseStructure<FVector>::Get() || Type==TBaseStructure<FRotator>::Get() ||
                Type==TBaseStructure<FTransform>::Get() || Type==TBaseStructure<FVector2D>::Get() ||
                Type==TBaseStructure<FLinearColor>::Get() || Type==TBaseStructure<FColor>::Get() || Type==TBaseStructure<FGuid>::Get() ||
                Type==FGameplayTag::StaticStruct() || Type==FGameplayTagContainer::StaticStruct();
            if (KnownValueParser) return true;
            if (const auto* Ops=Type->GetCppStructOps()) if (Ops->HasImportTextItem()) return false;
            for (TFieldIterator<FProperty> It(Type);It;++It) if (!IsSafeResultProperty(*It,Depth+1)) return false;
            return true;
        }
        // Includes object, weak/soft/class/interface, delegates, field paths and
        // containers/structs containing them. No nested reference bypass is allowed.
        return false;
    }
    static bool ContainsTextProperty(const FProperty* Property,int32 Depth=0)
    {
        if (!Property || Depth>32) return false;
        if (Property->IsA<FTextProperty>()) return true;
        if (const auto* Array=CastField<FArrayProperty>(Property)) return ContainsTextProperty(Array->Inner,Depth+1);
        if (const auto* Set=CastField<FSetProperty>(Property)) return ContainsTextProperty(Set->ElementProp,Depth+1);
        if (const auto* Map=CastField<FMapProperty>(Property)) return ContainsTextProperty(Map->KeyProp,Depth+1) || ContainsTextProperty(Map->ValueProp,Depth+1);
        if (const auto* Struct=CastField<FStructProperty>(Property))
            for (TFieldIterator<FProperty> It(Struct->Struct);It;++It) if (ContainsTextProperty(*It,Depth+1)) return true;
        return false;
    }
    static bool ShouldSkipOVProperty(const FProperty* Property)
    {
        return !Property || GOVSkippedPropertyNames.Contains(Property->GetFName()) ||
            Property->HasAnyPropertyFlags(CPF_Transient | CPF_NonPIEDuplicateTransient | CPF_EditorOnly | CPF_Config) || !IsSafeResultProperty(Property);
    }
    static bool IsSafeResultText(const FProperty* Property,const FString& Text)
    {
        // FText's LOCTABLE parser loads string-table assets independently of object
        // property flags. Ordinary localized/invariant text remains supported.
        return !ContainsTextProperty(Property) || !Text.Contains(TEXT("LOCTABLE"),ESearchCase::IgnoreCase);
    }
    static FName ResultRegistryKey(const FGorgeousSerializedOVNode& Node)
    { return Node.RegistryKey.IsNone() ? FName(*Node.Identifier.ToString(EGuidFormats::Digits)) : Node.RegistryKey; }
    static bool ValidateResultTree(const TArray<FGorgeousSerializedOVNode>& Nodes)
    {
        if (Nodes.IsEmpty() || Nodes.Num()>MaxResultTreeNodes) return false;
        TMap<FGuid,int32> Indices; int32 RootCount=0; int32 PropertyCount=0; int64 Bytes=0;
        TMap<FGuid,TSet<FName>> SiblingKeys;
        for (int32 Index=0;Index<Nodes.Num();++Index)
        {
            const auto& Node=Nodes[Index];
            if (!Node.Identifier.IsValid() || Indices.Contains(Node.Identifier) || Node.ClassPath.Len()>2048 || Node.RegistryKey.ToString().Len()>1024 || Node.Properties.Num()>1024) return false;
            Indices.Add(Node.Identifier,Index);
            if (!Node.ParentIdentifier.IsValid()) ++RootCount;
            Bytes+=128+int64(Node.ClassPath.Len()+Node.RegistryKey.ToString().Len())*sizeof(TCHAR);
            UClass* Class=FindObject<UClass>(nullptr,*Node.ClassPath);
            if (!Class || !Class->IsChildOf(UGorgeousObjectVariable::StaticClass()) || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) return false;
            TSet<const FProperty*> SeenProperties;
            for (const auto& Capture:Node.Properties)
            {
                Bytes+=(int64(Capture.PropertyName.Len())+Capture.ExportedValue.Len())*sizeof(TCHAR)+32;
                if (++PropertyCount>MaxResultTreeProperties || Bytes>MaxResultTreeBytes || Capture.PropertyName.Len()>1024) return false;
                const FProperty* Property=FindFProperty<FProperty>(Class,*Capture.PropertyName);
                if (ShouldSkipOVProperty(Property) || SeenProperties.Contains(Property) || !IsSafeResultText(Property,Capture.ExportedValue)) return false;
                SeenProperties.Add(Property);
            }
            if (Bytes>MaxResultTreeBytes) return false;
        }
        if (RootCount!=1) return false;
        for (const auto& Node:Nodes)
        {
            const FGorgeousSerializedOVNode* Current=&Node; TSet<FGuid> Path; int32 Depth=0;
            while (Current)
            {
                if (++Depth>MaxResultTreeDepth || Path.Contains(Current->Identifier)) return false;
                Path.Add(Current->Identifier);
                if (!Current->ParentIdentifier.IsValid()) break;
                const int32* Parent=Indices.Find(Current->ParentIdentifier); if (!Parent) return false;
                Current=&Nodes[*Parent];
            }
            if (Node.ParentIdentifier.IsValid())
            {
                auto& Keys=SiblingKeys.FindOrAdd(Node.ParentIdentifier); const FName Key=ResultRegistryKey(Node);
                if (Keys.Contains(Key)) return false; Keys.Add(Key);
            }
        }
        return true;
    }
    static bool CaptureOVTreeNodes(const UGorgeousObjectVariable* Root,TArray<FGorgeousSerializedOVNode>& OutNodes)
    {
        OutNodes.Reset(); TSet<const UGorgeousObjectVariable*> Visited; TSet<FGuid> Identifiers; int64 Bytes=0;
        TFunction<bool(const UGorgeousObjectVariable*,FGuid,FName,int32)> Append;
        Append=[&](const UGorgeousObjectVariable* Node,FGuid Parent,FName Key,int32 Depth)
        {
            if (!Node || Depth>MaxResultTreeDepth || Visited.Contains(Node) || OutNodes.Num()>=MaxResultTreeNodes) return false;
            Visited.Add(Node);
            FGorgeousSerializedOVNode Captured;
            Captured.Identifier=Node->UniqueIdentifier.IsValid() ? Node->UniqueIdentifier : FGuid::NewGuid();
            if (Identifiers.Contains(Captured.Identifier)) return false; Identifiers.Add(Captured.Identifier);
            Captured.ParentIdentifier=Parent; Captured.ClassPath=Node->GetClass()->GetPathName(); Captured.RegistryKey=Key;
            Bytes+=128+int64(Captured.ClassPath.Len()+Key.ToString().Len())*sizeof(TCHAR);
            for (TFieldIterator<FProperty> It(Node->GetClass(),EFieldIteratorFlags::IncludeSuper);It;++It)
            {
                FProperty* Property=*It; if (ShouldSkipOVProperty(Property)) continue;
                FGorgeousOVPropertyCapture Capture; Capture.PropertyName=Property->GetName();
                Property->ExportTextItem_Direct(Capture.ExportedValue,Property->ContainerPtrToValuePtr<void>(Node),nullptr,const_cast<UGorgeousObjectVariable*>(Node),PPF_None);
                if (!IsSafeResultText(Property,Capture.ExportedValue)) continue;
                Bytes+=(int64(Capture.PropertyName.Len())+Capture.ExportedValue.Len())*sizeof(TCHAR)+32;
                if (Bytes>MaxResultTreeBytes || Captured.Properties.Num()>=1024) return false;
                Captured.Properties.Add(MoveTemp(Capture));
            }
            const FGuid Identifier=Captured.Identifier; OutNodes.Add(MoveTemp(Captured));
            for (const auto& Pair:Node->VariableRegistry) if (Pair.Value && !Append(Pair.Value.Get(),Identifier,Pair.Key,Depth+1)) return false;
            return true;
        };
        if (!Append(Root,FGuid(),NAME_None,1) || !ValidateResultTree(OutNodes)) { OutNodes.Reset(); return false; }
        return true;
    }
    static UGorgeousObjectVariable* ReconstructOVTree(const TArray<FGorgeousSerializedOVNode>& Nodes)
    {
        // Preflight the ENTIRE message before constructing any object or invoking a parser.
        if (!ValidateResultTree(Nodes)) return nullptr;
        TMap<FGuid,UGorgeousObjectVariable*> Created;
        TArray<TStrongObjectPtr<UGorgeousObjectVariable>> ConstructionGuards;
        FOutputDeviceNull Errors;
        for (const auto& Node:Nodes)
        {
            UClass* Class=FindObject<UClass>(nullptr,*Node.ClassPath);
            auto* NewOV=NewObject<UGorgeousObjectVariable>(GetTransientPackage(),Class);
            if (!NewOV) return nullptr;
            ConstructionGuards.Emplace(NewOV); NewOV->UniqueIdentifier=Node.Identifier;
            for (const auto& Capture:Node.Properties)
            {
                FProperty* Property=FindFProperty<FProperty>(Class,*Capture.PropertyName);
                const TCHAR* End=Property->ImportText_Direct(*Capture.ExportedValue,Property->ContainerPtrToValuePtr<void>(NewOV),NewOV,PPF_None,&Errors);
                if (!End) return nullptr;
                while (*End && FChar::IsWhitespace(*End)) ++End;
                if (*End) return nullptr;
            }
            Created.Add(Node.Identifier,NewOV);
        }
        UGorgeousObjectVariable* Root=nullptr;
        for (const auto& Node:Nodes)
        {
            auto* Child=Created.FindChecked(Node.Identifier);
            if (Node.ParentIdentifier.IsValid())
            {
                auto* Parent=Created.FindChecked(Node.ParentIdentifier);
                Parent->VariableRegistry.Add(ResultRegistryKey(Node),Child); Child->SetParentRefOnly(Parent);
            }
            else Root=Child;
        }
        return Root;
    }
} // namespace

UGorgeousAutoReplicationRPCRelayComponent::UGorgeousAutoReplicationRPCRelayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGorgeousAutoReplicationRPCRelayComponent::RegisterClientInitiatedRPCOrigin(const FGuid& Guid,APlayerController* Controller,float TimeoutSeconds)
{
    if (!Guid.IsValid() || !Controller) return;
    const double Now=FPlatformTime::Seconds();
    for (auto It=ClientInitiatedRPCOrigins.CreateIterator();It;++It) if (!It.Value().Controller.IsValid() || It.Value().ExpiresAt<Now) It.RemoveCurrent();
    if (ClientInitiatedRPCOrigins.Num()>=1024 && !ClientInitiatedRPCOrigins.Contains(Guid)) return;
    auto& Origin=ClientInitiatedRPCOrigins.FindOrAdd(Guid);
    Origin.Controller=Controller; Origin.ExpiresAt=Now+FMath::Clamp(TimeoutSeconds>0 ? TimeoutSeconds : 30.0f,1.0f,300.0f);
}
APlayerController* UGorgeousAutoReplicationRPCRelayComponent::ResolveClientInitiatedRPCOrigin(const FGuid& Guid)
{
    const auto* Origin=ClientInitiatedRPCOrigins.Find(Guid);
    return Origin && Origin->ExpiresAt>=FPlatformTime::Seconds() ? Origin->Controller.Get() : nullptr;
}
void UGorgeousAutoReplicationRPCRelayComponent::RegisterClientInitiatedExpectedResponder(const FGuid& Guid,const FGorgeousAutoReplicationRPCResponderHandle& Responder)
{
    if (auto* Origin=ClientInitiatedRPCOrigins.Find(Guid)) if (Responder.IsValid()) Origin->Expected.Add(Responder.GetStableKey());
}
bool UGorgeousAutoReplicationRPCRelayComponent::IsClientInitiatedExpectedResponder(const FGuid& Guid,const FGorgeousAutoReplicationRPCResponderHandle& Responder)
{
    const auto* Origin=ClientInitiatedRPCOrigins.Find(Guid);
    return Origin && Origin->Controller.IsValid() && Origin->ExpiresAt>=FPlatformTime::Seconds() && Origin->Expected.Contains(Responder.GetStableKey());
}
void UGorgeousAutoReplicationRPCRelayComponent::TryRelayResultToClientInitiator(const FGorgeousAutoReplicationRPCResult& Result,EGorgeousRPCReadyState ReadyState)
{
    auto* Origin=ClientInitiatedRPCOrigins.Find(Result.QueuedRPC.RequestGuid);
    if (!Origin || !Origin->Controller.IsValid() || Origin->ExpiresAt<FPlatformTime::Seconds()) { ClientInitiatedRPCOrigins.Remove(Result.QueuedRPC.RequestGuid); return; }
    auto* Relay=Origin->Controller->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>();
    if (!Relay) return;
    FGorgeousAutoReplicationSerializedRPCResult Serialized;
    Relay->SerializeResult(Result,Serialized);
    Serialized.ExpectedResponderKeys=Origin->Expected.Array();
    Serialized.RelayedReadyState=ReadyState;
    Relay->ClientRelayAutoReplicationResult(Serialized);
    if (ReadyState==EGorgeousRPCReadyState::Ready) Origin->Completed.Add(Result.Responder.GetStableKey());
    bool bAllComplete=!Origin->Expected.IsEmpty();
    for (const FString& Key:Origin->Expected) if (!Origin->Completed.Contains(Key)) { bAllComplete=false; break; }
    if (bAllComplete) ClientInitiatedRPCOrigins.Remove(Result.QueuedRPC.RequestGuid);
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

    // An RPC on this PC channel can precede the destination actor's open bunch.
    // Legacy UE executes unmapped RPC object references as null by default. Do
    // not acknowledge initial state until that actor channel is acknowledged;
    // the existing initial-recipient polling retries without losing the snapshot.
    if (Envelope.OwnerActor && Envelope.OwnerActor!=GetOwner())
    {
        UNetConnection* Connection=GetOwner()->GetNetConnection();
        UNetDriver* Driver=GetWorld() ? GetWorld()->GetNetDriver() : nullptr;
        if (Connection && Driver && !Driver->IsUsingIrisReplication())
        {
            const UActorChannel* Channel=Connection->FindActorChannelRef(Envelope.OwnerActor.Get());
            if (!Channel || !Channel->OpenAcked) return false;
        }
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

    APlayerController* Sender = Cast<APlayerController>(GetOwner());
    if (!Sender) return;
    const auto ActualResponder = FGorgeousAutoReplicationRPCResponderHandle::FromController(Sender);
    if (!UGorgeousAutoReplicationRPCRequestAsyncAction::IsExpectedRemoteResponder(SerializedResult.RequestGuid, ActualResponder, GetWorld())) return;
	FGorgeousAutoReplicationRPCResult Result;
    Result.ExecutionWorld=GetWorld();
	Result.QueuedRPC.RequestGuid     = SerializedResult.RequestGuid;
	Result.TargetKind               = SerializedResult.TargetKind;
	Result.Responder                = ActualResponder;
	Result.TargetVariableIdentifier = FGuid();

	// If the sender included a full OV tree, reconstruct it here so the receiving side
	// has the actual return OV (including all child entries in VariableRegistry) without
	// needing the OV to already exist in this machine's registry.
	if (SerializedResult.OVTree.Num() > 0)
	{
		if (UGorgeousObjectVariable* ReconstructedRoot = ReconstructOVTree(SerializedResult.OVTree))
		{
			Result.TargetVariable = ReconstructedRoot;
            Result.TargetVariableIdentifier=ReconstructedRoot->UniqueIdentifier;
			UE_LOG(LogGorgeousAutoReplicationRelay, Verbose,
				TEXT("ServerRelayAutoReplicationResult: Reconstructed OV tree (%d nodes) for request %s."),
				SerializedResult.OVTree.Num(),
				*SerializedResult.RequestGuid.ToString());
		}
        else return;
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

    FGorgeousAutoReplicationMixin* DestinationMixin = Envelope.OwnerActor && Envelope.OwnerActor->GetWorld() == GetWorld() ? FGorgeousAutoReplicationMixin::ResolveForActor(Envelope.OwnerActor) : nullptr;
	if (!DestinationMixin)
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayPropertyPayload: No target mixin set for entry %s."), *Envelope.EntryKey.ToString());
		return;
	}

	// Validate that the sending client is permitted to push property changes for this entry.
	APlayerController* SenderController = Cast<APlayerController>(GetOwner());
	if (!DestinationMixin->CanControllerWritePropertyPayload(Envelope, SenderController))
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning,
			TEXT("ServerRelayPropertyPayload: Blocked property payload for entry %s from controller %s due to access policy."),
			*Envelope.EntryKey.ToString(), SenderController ? *SenderController->GetName() : TEXT("<null>"));

		// Send back the server's authoritative value as a correction so the client's local
		// instance converges back to the server state instead of silently diverging.
		if (UGorgeousObjectVariable* TargetVar = DestinationMixin->ResolveVariableForKey(Envelope.EntryKey))
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
                CorrectionEnvelope.OwnerActor = Envelope.OwnerActor;
                CorrectionEnvelope.VariableClass = Envelope.VariableClass;
                CorrectionEnvelope.StreamConfig = TargetVar->GetAutoReplicationConfig();
				CorrectionEnvelope.Payload = CorrectionPayload;
				ClientRelayPropertyPayload(CorrectionEnvelope);
				UE_LOG(LogGorgeousAutoReplicationRelay, Log,
					TEXT("ServerRelayPropertyPayload: Sent authoritative correction for entry %s back to controller %s."),
					*Envelope.EntryKey.ToString(), SenderController ? *SenderController->GetName() : TEXT("<null>"));
			}
		}
		return;
	}

	DestinationMixin->HandleTransportedPropertyPayload(Envelope);
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
    APlayerController* Sender=Cast<APlayerController>(GetOwner());
    AActor* TargetActor=QueuedRPC.TargetOwnerActor;
    FGorgeousAutoReplicationMixin* DestinationMixin=TargetActor && TargetActor->GetWorld()==GetWorld() ? FGorgeousAutoReplicationMixin::ResolveForActor(TargetActor) : TargetMixin;
    if (!DestinationMixin || !DestinationMixin->CanControllerInvokeRPC(QueuedRPC,Sender)) return;
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayRPCReliable: Server received RPC %s (Type=%d)."), *QueuedRPC.Key.ToString(), static_cast<int32>(QueuedRPC.Type));

	// Register the originating PC so results can be relayed back to the initiating client.
	if (QueuedRPC.RequestGuid.IsValid())
	{
		if (APlayerController* OriginPC = Cast<APlayerController>(GetOwner()))
		{
			RegisterClientInitiatedRPCOrigin(QueuedRPC.RequestGuid, OriginPC, QueuedRPC.Payload.TimeoutSeconds);
		}
	}

    if ((FGorgeousVDGConsent::IsCollectionRequest(QueuedRPC.Key) || FGorgeousVDGConsent::IsCollectionRequest(QueuedRPC.Payload.HandlerName)) && ResolveClientInitiatedRPCOrigin(QueuedRPC.RequestGuid)!=Sender) return;
	if (!DestinationMixin)
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCReliable: No target mixin set for RPC %s."), *QueuedRPC.Key.ToString());
		return;
	}

	// Check the RPC type and route accordingly
	switch (QueuedRPC.Type)
	{
	case EGorgeousAutoReplicationRPCType::EReliableServer:
		// Server-bound RPC, deliver locally
		DestinationMixin->HandleTransportedRPC(QueuedRPC);
		break;
	case EGorgeousAutoReplicationRPCType::EReliableClient:
		// Client RPC needs to be forwarded through transporter
		DestinationMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, nullptr);
		break;
	case EGorgeousAutoReplicationRPCType::EReliableMulticast:
	{
		// Route through the target mixin's own transporter with a single
		// ForwardRPCToMulticast call.  The previous per-GPS PlayerArray loop fired
		// the server-side handler once per PlayerState (3× with a listen-server +
		// 2 clients), producing duplicate "Server" tracker entries and incorrect
		// responder labels.  Using DestinationMixin->RequestRPC once mirrors the path
		// taken by server-originated multicasts and is both correct and efficient.
		FGuid MulticastGuid = QueuedRPC.RequestGuid;
		DestinationMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, &MulticastGuid);
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
    APlayerController* Sender=Cast<APlayerController>(GetOwner());
    AActor* TargetActor=QueuedRPC.TargetOwnerActor;
    FGorgeousAutoReplicationMixin* DestinationMixin=TargetActor && TargetActor->GetWorld()==GetWorld() ? FGorgeousAutoReplicationMixin::ResolveForActor(TargetActor) : TargetMixin;
    if (!DestinationMixin || !DestinationMixin->CanControllerInvokeRPC(QueuedRPC,Sender)) return;
	UE_LOG(LogGorgeousAutoReplicationRelay, Log, TEXT("ServerRelayRPCUnreliable: Server received RPC %s (Type=%d)."), *QueuedRPC.Key.ToString(), static_cast<int32>(QueuedRPC.Type));

	// Register the originating PC so results can be relayed back to the initiating client.
	if (QueuedRPC.RequestGuid.IsValid())
	{
		if (APlayerController* OriginPC = Cast<APlayerController>(GetOwner()))
		{
			RegisterClientInitiatedRPCOrigin(QueuedRPC.RequestGuid, OriginPC, QueuedRPC.Payload.TimeoutSeconds);
		}
	}

    if ((FGorgeousVDGConsent::IsCollectionRequest(QueuedRPC.Key) || FGorgeousVDGConsent::IsCollectionRequest(QueuedRPC.Payload.HandlerName)) && ResolveClientInitiatedRPCOrigin(QueuedRPC.RequestGuid)!=Sender) return;
	if (!DestinationMixin)
	{
		UE_LOG(LogGorgeousAutoReplicationRelay, Warning, TEXT("ServerRelayRPCUnreliable: No target mixin set for RPC %s."), *QueuedRPC.Key.ToString());
		return;
	}

	// Check the RPC type and route accordingly
	switch (QueuedRPC.Type)
	{
	case EGorgeousAutoReplicationRPCType::EUnreliableServer:
		// Server-bound RPC, deliver locally
		DestinationMixin->HandleTransportedRPC(QueuedRPC);
		break;
	case EGorgeousAutoReplicationRPCType::EUnreliableClient:
		// Client RPC needs to be forwarded through transporter
		DestinationMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, nullptr);
		break;
	case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
	{
		// Same fix as the reliable path, route through target mixin's transporter
		// rather than the per-GPS PlayerArray loop.
		FGuid MulticastGuid = QueuedRPC.RequestGuid;
		DestinationMixin->RequestRPC(QueuedRPC.Key, QueuedRPC.Type, QueuedRPC.Payload, QueuedRPC.TargetKind, &MulticastGuid);
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

    // Multiple PIE worlds share a process. A malformed or unresolved level
    // reference must never apply a received client snapshot to another world.
    if (Envelope.OwnerActor && Envelope.OwnerActor->GetWorld()!=GetWorld())
    {
        UE_LOG(LogGorgeousAutoReplicationRelay,Warning,TEXT("Rejected property payload %s: destination actor belongs to another world."),*Envelope.EntryKey.ToString());
        return;
    }
    FGorgeousAutoReplicationMixin* DestinationMixin = Envelope.OwnerActor ? FGorgeousAutoReplicationMixin::ResolveForActor(Envelope.OwnerActor) : nullptr;
	if (DestinationMixin)
	{
		DestinationMixin->HandleTransportedPropertyPayload(Envelope);
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

    UGorgeousAutoReplicationRPCRequestAsyncAction::RegisterExpectedResponderKeys(SerializedResult.RequestGuid,SerializedResult.ExpectedResponderKeys,GetWorld());
	// Reconstruct the result on the client side (mirrors ServerRelayAutoReplicationResult_Implementation).
	FGorgeousAutoReplicationRPCResult Result;
    Result.ExecutionWorld=GetWorld();
	Result.QueuedRPC.RequestGuid     = SerializedResult.RequestGuid;
	Result.TargetKind                = SerializedResult.TargetKind;
	Result.Responder                 = SerializedResult.Responder;
	Result.TargetVariableIdentifier  = SerializedResult.TargetVariableIdentifier;

	if (SerializedResult.OVTree.Num() > 0)
	{
		if (UGorgeousObjectVariable* ReconstructedRoot = ReconstructOVTree(SerializedResult.OVTree))
		{
			Result.TargetVariable = ReconstructedRoot;
            Result.TargetVariableIdentifier=ReconstructedRoot->UniqueIdentifier;
		}
        else return;
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
        if (!CaptureOVTreeNodes(RootOV,OutSerialized.OVTree)) OutSerialized.RequestGuid=FGuid();
	}
}

#if WITH_DEV_AUTOMATION_TESTS
UGorgeousObjectVariable* UGorgeousAutoReplicationRPCRelayComponent::ReconstructResultTreeForAutomation(const TArray<FGorgeousSerializedOVNode>& Nodes) { return ReconstructOVTree(Nodes); }
bool UGorgeousAutoReplicationRPCRelayComponent::CaptureResultTreeForAutomation(const UGorgeousObjectVariable* Root,TArray<FGorgeousSerializedOVNode>& Nodes) { return CaptureOVTreeNodes(Root,Nodes); }
#endif

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

void UGorgeousAutoReplicationRPCRelayComponent::ServerRelaySignalBridge_Implementation(uint8 Operation, FGameplayTag Tag, AActor* TargetActor, const FInstancedStruct& Payload)
{
    AGorgeousPlayerController* Sender = Cast<AGorgeousPlayerController>(GetOwner());
    if (!Sender || !Sender->HasAuthority() || !Tag.IsValid() || Operation > 3) return;
    if (TargetActor && TargetActor->GetWorld() != Sender->GetWorld()) return;
    USignalBridgeStorage_OV* Storage = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(Sender);
    if (!Storage) return;
    switch (Operation)
    {
    case 0: Storage->Server_RegisterListener_Implementation(Tag, Sender); break;
    case 1: Storage->Server_RegisterActorListener_Implementation(Tag, TargetActor, Sender); break;
    case 2: Storage->Server_UnregisterListener_Implementation(Tag, Sender); break;
    case 3:
    {
        FInstancedStruct Sanitized=Payload;
        if (const UScriptStruct* Struct=Sanitized.GetScriptStruct())
        {
            if (const FObjectProperty* Interactor=CastField<FObjectProperty>(Struct->FindPropertyByName(TEXT("Interactor"))))
                if (Interactor->PropertyClass && Interactor->PropertyClass->IsChildOf(AActor::StaticClass())) Interactor->SetObjectPropertyValue_InContainer(Sanitized.GetMutableMemory(),Sender->GetPawn() ? static_cast<AActor*>(Sender->GetPawn()) : Sender);
        }
        Storage->Server_RequestDispatch_Implementation(Tag,Sanitized,Sender); break;
    }
    }
}

void UGorgeousAutoReplicationRPCRelayComponent::ClientRelayRPCReliable_Implementation(const FGorgeousQueuedRPC& Request)
{
    if (Request.bHasTargetOwnerActor && !Request.TargetOwnerActor) return;
    auto* Destination = Request.TargetOwnerActor ? FGorgeousAutoReplicationMixin::ResolveForActor(Request.TargetOwnerActor) : TargetMixin;
    if (Destination) Destination->HandleTransportedRPC(Request);
}
void UGorgeousAutoReplicationRPCRelayComponent::ClientRelayRPCUnreliable_Implementation(const FGorgeousQueuedRPC& Request)
{
    ClientRelayRPCReliable_Implementation(Request);
}
