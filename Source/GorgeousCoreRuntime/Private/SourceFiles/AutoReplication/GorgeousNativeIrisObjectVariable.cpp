// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/GorgeousNativeIrisSupport.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectBase.h"

#if GORGEOUS_NATIVE_IRIS_SUPPORTED
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"
#endif

namespace
{
    bool IsBuiltInMetadata(const FName Name)
    {
        return Name==TEXT("DisplayName") || Name==TEXT("BroadcastRPCResultDescriptors")
            || Name==TEXT("OwnerScopedRPCResultDescriptors");
    }

    // References/custom serializers need connection-specific PackageMaps and retain that transport.
    bool IsPortableProperty(const FProperty* Property, int32 Depth=0)
    {
        if (!Property || Depth>16 || CastField<FObjectPropertyBase>(Property) || CastField<FInterfaceProperty>(Property)
            || CastField<FDelegateProperty>(Property) || CastField<FMulticastDelegateProperty>(Property)) return false;
        if (const auto* Array=CastField<FArrayProperty>(Property)) return IsPortableProperty(Array->Inner,Depth+1);
        if (const auto* Set=CastField<FSetProperty>(Property)) return IsPortableProperty(Set->ElementProp,Depth+1);
        if (const auto* Map=CastField<FMapProperty>(Property)) return IsPortableProperty(Map->KeyProp,Depth+1)&&IsPortableProperty(Map->ValueProp,Depth+1);
        if (const auto* Struct=CastField<FStructProperty>(Property))
        {
            if (const auto* Ops=Struct->Struct->GetCppStructOps(); Ops&&Ops->HasNetSerializer()) return false;
            for (TFieldIterator<FProperty> Field(Struct->Struct);Field;++Field)
                if (!IsPortableProperty(*Field,Depth+1)) return false;
            return true;
        }
        // New property subclasses must opt in after their complete contained type is checked.
        // In particular, optional/field-path properties must not silently bypass reference checks.
        return CastField<FNumericProperty>(Property) || CastField<FBoolProperty>(Property)
            || CastField<FNameProperty>(Property) || CastField<FStrProperty>(Property)
            || CastField<FTextProperty>(Property) || CastField<FEnumProperty>(Property);
    }

    bool SameState(const FGorgeousAutoReplicationPropertyPayload& A,const FGorgeousAutoReplicationPropertyPayload& B)
    {
        if (A.Properties.Num()!=B.Properties.Num()) return false;
        for (int32 Index=0;Index<A.Properties.Num();++Index)
        {
            const auto& X=A.Properties[Index];const auto& Y=B.Properties[Index];
            if (X.PropertyName!=Y.PropertyName || X.Mode!=Y.Mode || X.Payload!=Y.Payload) return false;
        }
        return true;
    }
}

bool UGorgeousObjectVariable::IsNativeIrisStateCompatible() const
{
    if (!bSupportsNetworking || !bReplicates || !HasAutoReplicationBinding()
        || ReplicationMode!=EGorgeousObjectVariableReplicationMode::EFullAutoReplication
        || AutoReplicationConfig.Backend!=EGorgeousAutoReplicationBackend::Iris
        || AutoReplicationConfig.bRespectAccessPolicy || IsRootNetworkStackEnabled() || IsSharedNetworkStackActive()
        || GetEffectiveAccessPolicy()!=EGorgeousObjectVariableAccessPolicy::Everyone
        || !RegisteredRPCBindings.IsEmpty() || !BroadcastRPCResultDescriptors.IsEmpty()
        || !OwnerScopedRPCResultDescriptors.IsEmpty()
        || RegisteredReplicatedProperties.IsEmpty() || RegisteredReplicatedProperties.Num()>1024) return false;
    const UFunction* Conditions=GetClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGorgeousObjectVariable,ResolvePropertyReplicationConditions));
    if (Conditions && Conditions->GetOuterUClass()!=StaticClass()) return false;
    int32 StateProperties=0;
    for (const auto& Property:RegisteredReplicatedProperties)
    {
        // DisplayName already uses its native replicated property. Result descriptors retain
        // their per-connection transport; an OV with active RPCs is ineligible above.
        if (IsBuiltInMetadata(Property.PropertyName)) continue;
        ++StateProperties;
        if (Property.Mode!=EGorgeousReplicationMode::EProperty || Property.ReplicationCondition!=COND_None
            || !Property.bSendInitialState || !Property.AdvancedConfig.bCustomConditionActive
            || !Property.AdvancedConfig.bDynamicConditionActive || !Property.AdvancedConfig.NetConditionGroup.IsNone()
            || Property.bInitializeNullReferences || Property.HasRepNotify() || !IsPortableProperty(Property.CachedProperty)) return false;
    }
    return StateProperties>0;
}

bool UGorgeousObjectVariable::TryPublishNativeIrisState(int64* OutBytes)
{
#if GORGEOUS_NATIVE_IRIS_SUPPORTED
    AActor* Actor=Cast<AActor>(GetAutoReplicationOwner());
    UWorld* World=Actor?Actor->GetWorld():nullptr;
    UNetDriver* Driver=World?World->GetNetDriver():nullptr;
    if (!Actor || !Actor->HasAuthority() || GetOuter()!=Actor || !Actor->GetIsReplicated()
        || !Actor->IsUsingRegisteredSubObjectList() || !Driver || !Driver->IsUsingIrisReplication()
        || !UE::Net::FReplicationSystemUtil::GetReplicationSystem(Driver)
        || !IsNativeIrisStateCompatible() || !FGorgeousAutoReplicationCoordinator::Get(World).IsIrisStreamActive(this))
    {
        ReleaseNativeIrisState();return false;
    }
    // Never take ownership of a subobject registration created by the project/another backend.
    if (!NativeIrisOwner.IsValid() && Actor->IsReplicatedSubObjectRegistered(this)) return false;

    FGorgeousAutoReplicationConditionContext Context;
    Context.StreamGuid=NativeIrisState.StreamGuid.IsValid()?NativeIrisState.StreamGuid:FGuid::NewGuid();
    FGorgeousAutoReplicationPropertyPayload State;
    if (!BuildAutoReplicationPropertyPayload(Context,State))
    { ReleaseNativeIrisState();return false; }
    State.Properties.RemoveAll([](const auto& Property){return IsBuiltInMetadata(Property.PropertyName);});
    int32 ExpectedProperties=0;
    for (const auto& Property:RegisteredReplicatedProperties)
        if (!IsBuiltInMetadata(Property.PropertyName)) ++ExpectedProperties;
    if (State.Properties.Num()!=ExpectedProperties) { ReleaseNativeIrisState();return false; }
    int64 Bytes=0;for (const auto& Property:State.Properties) Bytes+=Property.Payload.Num();
    // Bound the byte-array fragment below the native array serializer's element ceiling.
    if (Bytes>60*1024) { ReleaseNativeIrisState();return false; }

    const bool bChanged=NativeIrisPublishCount==0 || !SameState(State,NativeIrisState);
    bEverUsedNativeIris=true;NativeIrisState=MoveTemp(State);
    if (!NativeIrisOwner.IsValid())
    {
        NativeIrisOwner=Actor;
        Actor->AddReplicatedSubObject(this,COND_None);
    }
    if (bChanged)
    {
        ++NativeIrisPublishCount;
        Actor->FlushNetDormancy();Actor->ForceNetUpdate();
    }
    if (OutBytes) *OutBytes=bChanged?Bytes:0;
    return true;
#else
    return false;
#endif
}

void UGorgeousObjectVariable::ReleaseNativeIrisState()
{
    if (UObjectInitialized()&&!GExitPurge)
        if (AActor* Actor=NativeIrisOwner.Get();Actor&&Actor->HasAuthority()) Actor->RemoveReplicatedSubObject(this);
    NativeIrisOwner.Reset();
    NativeIrisState.Reset();
}

bool UGorgeousObjectVariable::HasNativeIrisReplicationHandle() const
{
#if GORGEOUS_NATIVE_IRIS_SUPPORTED
    const AActor* Actor=Cast<AActor>(GetAutoReplicationOwner());
    if (!Actor) Actor=GetTypedOuter<AActor>();
    if (Actor)
        if (const auto* Bridge=UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(Actor))
            return Bridge->GetReplicatedRefHandle(this).IsValid();
#endif
    return false;
}

void UGorgeousObjectVariable::OnRep_NativeIrisState()
{
    if (NativeIrisState.IsEmpty()) return;
    bEverUsedNativeIris=true;++NativeIrisReceiveCount;bNativeIrisStatePending=true;
    ApplyPendingNativeIrisState();
}

void UGorgeousObjectVariable::ApplyPendingNativeIrisState()
{
    if (!bNativeIrisStatePending || RegisteredReplicatedProperties.IsEmpty() || HasAuthority()) return;
    int64 Bytes=0;for (const auto& Property:NativeIrisState.Properties) Bytes+=Property.Payload.Num();
    if (Bytes>60*1024 || NativeIrisState.Properties.Num()>1024) { bNativeIrisStatePending=false;return; }
    auto State=NativeIrisState;
    for (auto& Property:State.Properties) Property.bIsInitialState=NativeIrisAppliedStateCount==0;
    if (ApplyAutoReplicationPropertyPayload(State,nullptr,true))
    { ++NativeIrisAppliedStateCount;bNativeIrisStatePending=false; }
}

#if UE_WITH_IRIS
void UGorgeousObjectVariable::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,UE::Net::EFragmentRegistrationFlags Flags)
{
#if GORGEOUS_NATIVE_IRIS_SUPPORTED
    UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this,Context,Flags);
#else
    Super::RegisterReplicationFragments(Context,Flags);
#endif
}
#endif
