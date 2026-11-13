#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "AutoReplication/Globals/GorgeousAutoReplicationRPCPayloadGlobals.h"
#include "SharedTests/GorgeousLimitationAuditTestTypes.h"
#include "SharedTests/GorgeousSignalBridgeInsightHelper.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/GarbageCollection.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousNativeContainerRegression, "GorgeousCore.LimitationAudit.NativeContainers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousNativeContainerRegression::RunTest(const FString&)
{
    UInteger_QOV* Queue=NewObject<UInteger_QOV>();
    Queue->EnqueueQueue(11); Queue->EnqueueQueue(22);
    int32 Number=0;
    TestTrue(TEXT("Queue dequeue succeeds without generated interface assert"), Queue->DequeueQueue(Number));
    TestEqual(TEXT("FIFO order"),Number,11);
    UInteger_SKOV* Stack=NewObject<UInteger_SKOV>();
    Stack->PushStack(11); Stack->PushStack(22);
    TestTrue(TEXT("Stack pop succeeds"),Stack->PopStack(Number));
    TestEqual(TEXT("LIFO order"),Number,22);
    UInteger_DQOV* Deque=NewObject<UInteger_DQOV>();
    Deque->PushFrontDeque(11); Deque->PushBackDeque(22);
    TestTrue(TEXT("Deque pop succeeds"),Deque->PopBackDeque(Number));
    TestEqual(TEXT("Deque order"),Number,22);
    UInteger_MMOV* Multi=NewObject<UInteger_MMOV>();
    UInteger_SOV* First=NewObject<UInteger_SOV>();
    Multi->AddMultiMap(1, First); Multi->AddMultiMap(1, Queue);
    TestEqual(TEXT("Multimap keeps duplicate key entries"), Multi->FindMultiMapValues(1).Values.Num(),2);
    TestNotNull(TEXT("Queue storage participates in serialization and GC"),FindFProperty<FArrayProperty>(Queue->GetClass(),TEXT("Value")));
    TestNotNull(TEXT("Stack reflected storage"),FindFProperty<FArrayProperty>(Stack->GetClass(),TEXT("Value")));
    TestNotNull(TEXT("Deque reflected storage"),FindFProperty<FArrayProperty>(Deque->GetClass(),TEXT("Value")));
    TestNotNull(TEXT("Multimap reflected storage"),FindFProperty<FMapProperty>(Multi->GetClass(),TEXT("Value")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousContainerGCRegression,"GorgeousCore.LimitationAudit.ContainerGC",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousContainerGCRegression::RunTest(const FString&)
{
    TStrongObjectPtr<UObject_QOTOV> Queue(NewObject<UObject_QOTOV>());
    TWeakObjectPtr<UObject> Child(NewObject<UGorgeousLimitationAuditReceiver>());
    Queue->EnqueueQueue(Child.Get());
    CollectGarbage(RF_NoFlags);
    TestTrue(TEXT("Queue object element survives GC solely through reflected storage"),Child.IsValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDynamicPropertyRegression,"GorgeousCore.LimitationAudit.DynamicPropertyTypeSafety",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDynamicPropertyRegression::RunTest(const FString&)
{
    UInteger_SOV* Variable=NewObject<UInteger_SOV>();
    int32 Value=17;
    Variable->SetIntegerSingleObjectVariable_Implementation(TEXT("Value"),Value);
    TestEqual(TEXT("Matching dynamic property accepts write"),Variable->GetIntegerSingleObjectVariable_Implementation(TEXT("Value")),17);
    const FString Before=Variable->GetDisplayName();
    Variable->SetIntegerSingleObjectVariable_Implementation(TEXT("DisplayName"),Value);
    TestEqual(TEXT("Mismatched property name never corrupts string storage"),Variable->GetDisplayName(),Before);
    TestEqual(TEXT("Mismatched read returns intact fallback value"),Variable->GetIntegerSingleObjectVariable_Implementation(TEXT("DisplayName")),17);
    TestFalse(TEXT("Client property writes require explicit opt in"),Variable->bAllowClientPropertyWrites);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousRPCWireRegression,"GorgeousCore.LimitationAudit.RPCWireAndAllowlist",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousRPCWireRegression::RunTest(const FString&)
{
    UGorgeousLimitationAuditReceiver* Receiver=NewObject<UGorgeousLimitationAuditReceiver>();
    FGorgeousQueuedRPC Request;
    Request.Payload=UGorgeousAutoReplicationRPCPayloadGlobals::MakeAutoReplicationRPCPayload(TEXT("Receive"));
    UGorgeousAutoReplicationRPCPayloadGlobals::AddAutoReplicationRPCNameArgument(Request.Payload,TEXT("Token"),TEXT("PortableName"));
    TArray<FString> Values={TEXT("one"),TEXT("two")};
    FProperty* ArrayProperty=FindFProperty<FProperty>(Receiver->FindFunction(TEXT("Receive")),TEXT("Values"));
    TestTrue(TEXT("Container arguments serialize through an archive"),UGorgeousAutoReplicationRPCPayloadGlobals::AddArgumentFromProperty(Request.Payload,TEXT("Values"),ArrayProperty,&Values));
    TestFalse(TEXT("Unregistered owner handler rejected"),UGorgeousObjectVariable::InvokeNativeAutoReplicationRPCHandlerOnObject(Receiver,Request));
    TestEqual(TEXT("Rejected handler did not execute"),Receiver->Calls,0);
    TestTrue(TEXT("Explicit handler can be registered"),UGorgeousObjectVariable::RegisterAutoReplicationRPCHandlerOnObject(Receiver,TEXT("Receive")));
    TestTrue(TEXT("Registered handler executes"),UGorgeousObjectVariable::InvokeNativeAutoReplicationRPCHandlerOnObject(Receiver,Request));
    TestEqual(TEXT("FName roundtrips as text"),Receiver->LastName,FName(TEXT("PortableName")));
    TestEqual(TEXT("Container values survive serialization"),Receiver->LastValues.Num(),2);
    if (Receiver->LastValues.Num()==2) TestEqual(TEXT("Container second item"),Receiver->LastValues[1],FString(TEXT("two")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousScopedSignalRegression,"GorgeousCore.LimitationAudit.ScopedSignalUnlisten",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousScopedSignalRegression::RunTest(const FString&)
{
    USignalBridgeStorage_OV* Bridge=NewObject<USignalBridgeStorage_OV>();
    UGorgeousSignalBridgeInsightHelper* A=NewObject<UGorgeousSignalBridgeInsightHelper>();
    UGorgeousSignalBridgeInsightHelper* B=NewObject<UGorgeousSignalBridgeInsightHelper>();
    FSignalBridgeEventDelegate DA,DB;
    DA.BindDynamic(A,&UGorgeousSignalBridgeInsightHelper::HandleTestSignal);
    DB.BindDynamic(B,&UGorgeousSignalBridgeInsightHelper::HandleTestSignal);
    Bridge->Listen(TAG_Gorgeous_Test_SignalBridge_Perf,nullptr,DA);
    Bridge->Listen(TAG_Gorgeous_Test_SignalBridge_Perf,nullptr,DB);
    Bridge->Unlisten(TAG_Gorgeous_Test_SignalBridge_Perf,nullptr,DA);
    Bridge->Dispatch(TAG_Gorgeous_Test_SignalBridge_Perf,FInstancedStruct());
    TestEqual(TEXT("Removed delegate stays silent"),A->SignalsReceived,0);
    TestEqual(TEXT("Concurrent listener remains registered"),B->SignalsReceived,1);
    return true;
}

#include "ObjectVariables/GorgeousExtendedObjectVariables.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDiskSnapshotRegression,"GorgeousCore.LimitationAudit.DiskSnapshot",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDiskSnapshotRegression::RunTest(const FString&)
{
    UInteger_SOV* Variable=NewObject<UInteger_SOV>();
    Variable->UniqueIdentifier=FGuid::NewGuid();
    int32 Original=271;
    Variable->SetIntegerSingleObjectVariable_Implementation(NAME_None,Original);
    const FString Slot=TEXT("Automation_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    const FString Filename=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Gorgeous/ObjectVariables"),Slot+TEXT(".gov"));
    ON_SCOPE_EXIT { IFileManager::Get().Delete(*Filename); IFileManager::Get().Delete(*(Filename+TEXT(".tmp"))); };
    TestTrue(TEXT("Snapshot saves to project Saved directory"),UGorgeousObjectVariable::SaveObjectVariableSnapshot(Variable,Slot));
    TestTrue(TEXT("Save success corresponds to a nonempty final file"),IFileManager::Get().FileSize(*Filename)>0);
    UInteger_SOV* Restored=Cast<UInteger_SOV>(UGorgeousObjectVariable::LoadObjectVariableSnapshot(Slot));
    TestNotNull(TEXT("Concrete class is restored"),Restored);
    if (Restored)
    {
        TestEqual(TEXT("Disk snapshot restores value"),Restored->GetIntegerSingleObjectVariable_Implementation(NAME_None),Original);
        TestTrue(TEXT("Disk imports assign a fresh valid identity"),Restored->UniqueIdentifier.IsValid() && Restored->UniqueIdentifier!=Variable->UniqueIdentifier);
    }
    Original=937; Variable->SetIntegerSingleObjectVariable_Implementation(NAME_None,Original);
    TestTrue(TEXT("Existing snapshot can be atomically replaced and reports success"),UGorgeousObjectVariable::SaveObjectVariableSnapshot(Variable,Slot));
    auto* Replaced=Cast<UInteger_SOV>(UGorgeousObjectVariable::LoadObjectVariableSnapshot(Slot));
    TestNotNull(TEXT("Overwritten snapshot loads"),Replaced);
    if (Replaced) TestEqual(TEXT("Overwritten file contains new value"),Replaced->GetIntegerSingleObjectVariable_Implementation(NAME_None),Original);
    TestFalse(TEXT("Slot path cannot escape saved directory"),UGorgeousObjectVariable::SaveObjectVariableSnapshot(Variable,TEXT("../Escape")));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousExtendedValueRegression,"GorgeousCore.LimitationAudit.ExtendedValueTypes",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousExtendedValueRegression::RunTest(const FString&)
{
    auto* Vector=NewObject<UGorgeousVectorObjectVariable>();
    Vector->SetValue(FVector(1,2,3));
    TestTrue(TEXT("Vector value preserved"),Vector->GetValue().Equals(FVector(1,2,3)));
    auto* Transform=NewObject<UGorgeousTransformObjectVariable>();
    Transform->SetValue(FTransform(FVector(4,5,6)));
    TestTrue(TEXT("Transform value preserved"),Transform->GetValue().GetTranslation().Equals(FVector(4,5,6)));
    auto* Struct=NewObject<UGorgeousStructObjectVariable>();
    FGorgeousTestSignalPayload TestStruct; TestStruct.PrimeResult=271;
    Struct->SetValue(FInstancedStruct::Make(TestStruct));
    TestEqual(TEXT("Arbitrary reflected struct preserved"),Struct->GetValue().Get<FGorgeousTestSignalPayload>().PrimeResult,271);
    auto* Enum=NewObject<UGorgeousEnumObjectVariable>();
    Enum->EnumType=StaticEnum<EGorgeousObjectVariableReplicationMode>();
    TestFalse(TEXT("Invalid enum values rejected"),Enum->SetValue(99999));
    return true;
}

#include "AutoReplication/GorgeousReplicationBudget.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousBudgetRegression,"GorgeousCore.LimitationAudit.ReplicationBudget",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousBudgetRegression::RunTest(const FString&)
{
    FGorgeousReplicationTokenBucket Budget;
    Budget.Refill(0,4);
    TestTrue(TEXT("Initial stream budget allows dispatch"),Budget.CanDispatch(4));
    Budget.Consume(8192);
    TestFalse(TEXT("Oversized payload incurs debt instead of allowing unbounded throughput"),Budget.CanDispatch(4));
    Budget.Refill(0.5,4);
    TestFalse(TEXT("Budget remains throttled until debt has recovered"),Budget.CanDispatch(4));
    Budget.Refill(0.6,4);
    TestTrue(TEXT("Deferred updates can dispatch after refill"),Budget.CanDispatch(4));
    TestTrue(TEXT("Zero budget explicitly means unlimited"),Budget.CanDispatch(0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousValueChangedRegression,"GorgeousCore.LimitationAudit.ValueChanged",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousValueChangedRegression::RunTest(const FString&)
{
    auto* Variable=NewObject<UGorgeousConditionAuditVariable>(); auto* Listener=NewObject<UGorgeousLimitationAuditReceiver>();
    Variable->ObserveValueChanges(Listener);
    int32 Value=71;
    Variable->SetAuditNativeValue(Value);
    Variable->SetAuditNativeValue(Value);
    TestEqual(TEXT("Native setter emits one event for an actual change"),Listener->Changes,1);
    Value=72; Variable->SetAuditNamedValue(Value);
    TestEqual(TEXT("Dynamic setter uses same change event"),Listener->Changes,2);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousLevelPersistenceRegression,"GorgeousCore.LimitationAudit.LevelPersistence",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousLevelPersistenceRegression::RunTest(const FString&)
{
    if (!FParse::Param(FCommandLine::Get(),TEXT("GorgeousAllowAuditPIE"))) { AddError(TEXT("Requires isolated audit project flag because this exercises real level-transition registry cleanup.")); return false; }
    auto* Root=UGorgeousRootObjectVariable::GetRootObjectVariable();
    if (!TestNotNull(TEXT("Default root available"),Root)) return false;
    FGuid A,B;
    auto* Temporary=Root->NewObjectVariable(UInteger_SOV::StaticClass(),A,Root,false,TEXT("AuditTemporary"));
    auto* Persistent=Root->NewObjectVariable(UInteger_SOV::StaticClass(),B,Root,true,TEXT("AuditPersistent"));
    TestNotNull(TEXT("Temporary created"),Temporary); TestNotNull(TEXT("Persistent created"),Persistent);
    UGorgeousRootObjectVariable::CleanupRegistry(false);
    TestNull(TEXT("Nonpersistent data removed by level-transition cleanup"),UGorgeousRootObjectVariable::FindVariableByIdentifier(A));
    TestEqual(TEXT("Persistent data survives level-transition cleanup"),UGorgeousRootObjectVariable::FindVariableByIdentifier(B),Persistent);
    UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Persistent);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousConditionRegression,"GorgeousCore.LimitationAudit.ReplicationConditions",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousConditionRegression::RunTest(const FString&)
{
    auto Build=[&](ELifetimeCondition Condition,bool Initial,bool Owner,bool Simulated,bool Replay,bool Custom,bool Dynamic,bool Group) {
        auto* Variable=NewObject<UGorgeousConditionAuditVariable>();
        Variable->Configuration.ReplicationCondition=Condition;
        Variable->Configuration.bCustomConditionActive=Custom;
        Variable->Configuration.bDynamicConditionActive=Dynamic;
        Variable->ActivateReplication(FGorgeousAutoReplicationContext());
        FGorgeousAutoReplicationConditionContext Context;
        Context.bIsInitialState=Initial; Context.bIsOwnerConnection=Owner;
        Context.bIsAutonomousProxy=Owner; Context.bIsSimulatedProxy=Simulated;
        Context.bIsReplayConnection=Replay; Context.bNetGroupMatch=Group;
        FGorgeousAutoReplicationPropertyPayload Payload;
        const bool bBuilt=Variable->BuildAutoReplicationPropertyPayload(Context,Payload);
        TestTrue(TEXT("Condition fixture still serializes unconditional base metadata"),bBuilt && Payload.Properties.ContainsByPredicate([](const auto& Property) { return Property.PropertyName==TEXT("DisplayName"); }));
        const auto* Value=Payload.Properties.FindByPredicate([](const auto& Property) { return Property.PropertyName==TEXT("Value"); });
        if (Value) TestEqual(TEXT("Transmitted Value retains its actual lifetime condition"),int32(Value->ReplicationCondition),int32(Condition));
        return Value!=nullptr;
    };
    TestTrue(TEXT("InitialOnly sends initial snapshot"),Build(COND_InitialOnly,true,false,false,false,false,false,false));
    TestFalse(TEXT("InitialOnly suppresses later updates"),Build(COND_InitialOnly,false,false,false,false,false,false,false));
    TestTrue(TEXT("SimulatedOnly sees simulated connection"),Build(COND_SimulatedOnly,false,false,true,false,false,false,false));
    TestTrue(TEXT("AutonomousOnly sees owning autonomous connection"),Build(COND_AutonomousOnly,false,true,false,false,false,false,false));
    TestFalse(TEXT("Custom false suppresses property"),Build(COND_Custom,false,false,false,false,false,false,false));
    TestTrue(TEXT("Custom configured predicate enables property"),Build(COND_Custom,false,false,false,false,true,false,false));
    TestTrue(TEXT("Dynamic configured predicate enables property"),Build(COND_Dynamic,false,false,false,false,false,true,false));
    TestTrue(TEXT("NetGroup context enables property"),Build(COND_NetGroup,false,false,false,false,false,false,true));
    TestTrue(TEXT("ReplayOnly sees replay connection"),Build(COND_ReplayOnly,false,false,false,true,false,false,false));
    TestFalse(TEXT("SkipReplay filters replay connection"),Build(COND_SkipReplay,false,false,false,true,false,false,false));
    return true;
}
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousTrunkRoundTripRegression,"GorgeousCore.LimitationAudit.TrunkRoundTrip",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousTrunkRoundTripRegression::RunTest(const FString&)
{
    auto* Source=NewObject<UInteger_SOV>();
    Source->UniqueIdentifier=FGuid::NewGuid(); int32 InitialValue=271; Source->SetIntegerSingleObjectVariable_Implementation(NAME_None,InitialValue);
    FGorgeousObjectVariableTrunk Original;
    FGorgeousObjectVariableSerializedPayload Payload;
    if (!TestTrue(TEXT("Concrete OV writes a valid reflected payload"),Source->SerializeToPayload(Payload))) return false;
    Original.SetPayload(Payload);
    TArray<uint8> Bytes; FMemoryWriter Writer(Bytes,true); FObjectAndNameAsStringProxyArchive Save(Writer,false);
    FGorgeousObjectVariableTrunk::StaticStruct()->SerializeItem(Save,&Original,nullptr);
    FGorgeousObjectVariableTrunk Restored; FMemoryReader Reader(Bytes,true); FObjectAndNameAsStringProxyArchive Load(Reader,false);
    FGorgeousObjectVariableTrunk::StaticStruct()->SerializeItem(Load,&Restored,nullptr);
    TestFalse(TEXT("Reflected trunk import has no archive error"),Load.IsError());
    auto* Value=Restored.GetOrCreateRuntimeInstance(Source->UniqueIdentifier,GetTransientPackage());
    if (!TestNotNull(TEXT("Serialized entry hydrates a runtime OV"),Value)) return false;
    TestEqual(TEXT("Concrete class survives trunk export/import"),Value->GetClass(),UInteger_SOV::StaticClass());
    TestEqual(TEXT("Stored Value survives trunk export/import"),IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Value,NAME_None),271);
    TestEqual(TEXT("Repeated hydration shares the cache"),Restored.GetOrCreateRuntimeInstance(Source->UniqueIdentifier,GetTransientPackage()),Value);
    Restored.InvalidateAll();
    TestTrue(TEXT("Invalidating cache hydrates a fresh instance"),Restored.GetOrCreateRuntimeInstance(Source->UniqueIdentifier,GetTransientPackage())!=Value);
    auto* Owner=NewObject<UGorgeousTrunkAuditOwner>();
    Owner->Mixin.Bind(Owner,&Owner->AdditionalData,&Owner->ReplicatedData);
    auto& Slot=Owner->AdditionalData.FindOrAdd(TEXT("AuditStored"));
    Slot.DefaultValue=Source;
    Owner->Mixin.InitializeAdditionalData(false);
    TestTrue(TEXT("Runtime binding persists slot identifier"),Slot.SerializedVariableIdentifier.IsValid());
    TestNotNull(TEXT("Runtime binding writes trunk payload"),Owner->DefaultObjectVariableTrunk.FindPayload(Slot.SerializedVariableIdentifier));
    Slot.DefaultValue=nullptr; Slot.Handle.Reset(); Owner->DefaultObjectVariableTrunk.InvalidateAll();
    Owner->Mixin.InitializeAdditionalData(false);
    TestNotNull(TEXT("Runtime binding rehydrates missing slot from trunk"),Slot.DefaultValue.Get());
    if (Slot.DefaultValue)
        TestEqual(TEXT("Runtime trunk rehydration retains concrete value"),IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Slot.DefaultValue,NAME_None),271);
    return true;
}
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStorage_OV.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationSettings.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "NativeGameplayTags.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GorgeousAuditStatAccess,"Gorgeous.Test.Stat.AuditAccess");
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousStatAccessRegression,"GorgeousCore.LimitationAudit.StatAccessAndInitialZero",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousStatAccessRegression::RunTest(const FString&)
{
    UWorld* World=nullptr;
    if (GEngine) for (const FWorldContext& Context:GEngine->GetWorldContexts())
        if (Context.World() && Context.World()->GetNetMode()!=NM_Client) { World=Context.World(); break; }
    if (!TestNotNull(TEXT("Stat test requires an initialized authority/editor world"),World)) return false;
    auto* Actor=World->SpawnActor<AActor>(); auto* Allowed=World->SpawnActor<AGorgeousPlayerController>(); auto* Denied=World->SpawnActor<AGorgeousPlayerController>();
    if (!Actor || !Allowed || !Denied) { AddError(TEXT("Stat fixture actor creation failed.")); if(Actor)Actor->Destroy(); if(Allowed)Allowed->Destroy(); if(Denied)Denied->Destroy(); return false; }
    ON_SCOPE_EXIT { Actor->Destroy(); Allowed->Destroy(); Denied->Destroy(); };
    auto* Settings=GetMutableDefault<UGorgeousStatFoundationSettings>(); const auto Saved=Settings->StatRegistry;
    ON_SCOPE_EXIT { Settings->StatRegistry=Saved; };
    FGorgeousStat_S Metadata; Metadata.AccessRules.AccessPolicy=EGorgeousObjectVariableAccessPolicy::Custom;
    Metadata.AccessRules.AllowedControllers.Add(Allowed); Settings->StatRegistry.Add(TAG_GorgeousAuditStatAccess.GetTag(),Metadata);
    auto* Storage=NewObject<UGorgeousStatFoundationStorage_OV>(World); auto* Listener=NewObject<UGorgeousLimitationAuditReceiver>();
    Storage->OnActorStatChanged.AddDynamic(Listener,&UGorgeousLimitationAuditReceiver::OnStatChanged);
    TestFalse(TEXT("Unlisted controller gets an explicit rejected result"),Storage->SetStatValue(Actor,TAG_GorgeousAuditStatAccess.GetTag(),0,Denied));
    TestTrue(TEXT("AllowedControllers instance whitelist is honored"),Storage->SetStatValue(Actor,TAG_GorgeousAuditStatAccess.GetTag(),0,Allowed));
    TestEqual(TEXT("First explicit zero produces change event"),Listener->Changes,1);
    TestTrue(TEXT("Idempotent accepted write returns true"),Storage->SetStatValue(Actor,TAG_GorgeousAuditStatAccess.GetTag(),0,Allowed));
    TestEqual(TEXT("Repeated zero does not duplicate event"),Listener->Changes,1);
    TestTrue(TEXT("Authority caller without arbitrary requester can update private stat"),Storage->SetStatValue(Actor,TAG_GorgeousAuditStatAccess.GetTag(),7,nullptr));
    Settings->StatRegistry.Remove(TAG_GorgeousAuditStatAccess.GetTag());
    TestTrue(TEXT("Absent metadata uses explicit default Everyone access"),Storage->SetStatValue(Actor,TAG_GorgeousAuditStatAccess.GetTag(),8,Denied));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousSelfReferenceFilterRegression,"GorgeousCore.LimitationAudit.SelfReferenceFilter",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousSelfReferenceFilterRegression::RunTest(const FString&)
{
    auto* Root=UGorgeousRootObjectVariable::GetRootObjectVariable(); if (!Root) { AddError(TEXT("Registry unavailable.")); return false; }
    FGuid Id; auto* Array=Root->NewObjectVariable(UObject_AOTOV::StaticClass(),Id,Root,false,TEXT("AuditOrdinaryObjectArray"));
    auto* Object=NewObject<UGorgeousLimitationAuditReceiver>(); TArray<UObject*> Objects{Object};
    IGorgeousArrayObjectVariablesSetter_I::Execute_SetObjectObjectArrayObjectVariable(Array,NAME_None,Objects);
    TestFalse(TEXT("Ordinary object arrays do not masquerade as QoL references"),FGorgeousQualityOfLifeStatics::ResolveSelfReferences(nullptr,UGorgeousLimitationAuditReceiver::StaticClass()).Contains(Object));
    TMap<FName,FGorgeousObjectVariableEntry> Entries;
    TestNotNull(TEXT("Explicit self reference registers"),FGorgeousQualityOfLifeStatics::EnsureSelfReference(Object,Entries,false));
    TestTrue(TEXT("Marked self reference is discoverable"),FGorgeousQualityOfLifeStatics::ResolveSelfReferences(nullptr,UGorgeousLimitationAuditReceiver::StaticClass()).Contains(Object));
    FGorgeousQualityOfLifeStatics::ClearSelfReference(Object,Entries); UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Array);
    return true;
}
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousResultTreeIngressRegression,"GorgeousCore.LimitationAudit.ResultTreeIngress",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousResultTreeIngressRegression::RunTest(const FString&)
{
    auto MakeNode=[](UClass* Class,FGuid Parent=FGuid()) {
        FGorgeousSerializedOVNode Node; Node.Identifier=FGuid::NewGuid(); Node.ParentIdentifier=Parent; Node.ClassPath=Class->GetPathName(); return Node;
    };
    auto Decode=[](const TArray<FGorgeousSerializedOVNode>& Nodes) { return UGorgeousAutoReplicationRPCRelayComponent::ReconstructResultTreeForAutomation(Nodes); };
    auto Root=MakeNode(UInteger_SOV::StaticClass());
    FGorgeousOVPropertyCapture Value; Value.PropertyName=TEXT("Value"); Value.ExportedValue=TEXT("271"); Root.Properties.Add(Value);
    auto Child=MakeNode(UInteger_SOV::StaticClass(),Root.Identifier); Child.RegistryKey=TEXT("Child");
    TStrongObjectPtr<UGorgeousObjectVariable> Result(Decode({Root,Child}));
    if (!TestNotNull(TEXT("Valid primitive tree reconstructs through actual RPC ingress decoder"),Result.Get())) return false;
    TestEqual(TEXT("Primitive return payload retained"),IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Result.Get(),NAME_None),271);
    TestTrue(TEXT("Child registry linked"),Result->VariableRegistry.Contains(TEXT("Child")));
    if (auto* Found=Result->VariableRegistry.Find(TEXT("Child"))) TestEqual(TEXT("Reconstructed parent pointer matches registry hierarchy"),(*Found)->GetParent(),Result.Get());
    auto SelfCycle=Root; SelfCycle.ParentIdentifier=SelfCycle.Identifier;
    TestNull(TEXT("Self-cycle rejected"),Decode({SelfCycle}));
    auto CycleA=MakeNode(UInteger_SOV::StaticClass()); auto CycleB=MakeNode(UInteger_SOV::StaticClass());
    CycleA.ParentIdentifier=CycleB.Identifier; CycleB.ParentIdentifier=CycleA.Identifier;
    TestNull(TEXT("Disconnected cyclic component rejected even with a valid root"),Decode({Root,CycleA,CycleB}));
    auto Missing=Child; Missing.ParentIdentifier=FGuid::NewGuid();
    TestNull(TEXT("Dangling parent rejected"),Decode({Root,Missing}));
    auto Duplicate=Child; Duplicate.Identifier=Root.Identifier;
    TestNull(TEXT("Duplicate node identifiers rejected"),Decode({Root,Duplicate}));
    auto OtherRoot=MakeNode(UInteger_SOV::StaticClass());
    TestNull(TEXT("Multiple roots rejected"),Decode({Root,OtherRoot}));
    auto OtherChild=MakeNode(UInteger_SOV::StaticClass(),Root.Identifier); OtherChild.RegistryKey=TEXT("Child");
    TestNull(TEXT("Duplicate sibling keys rejected"),Decode({Root,Child,OtherChild}));
    TArray<FGorgeousSerializedOVNode> Deep; Deep.Add(Root);
    for (int32 Index=0;Index<64;++Index) Deep.Add(MakeNode(UInteger_SOV::StaticClass(),Deep.Last().Identifier));
    TestNull(TEXT("Tree depth beyond64 rejected before allocation"),Decode(Deep));
    auto OversizedA=MakeNode(UString_SOV::StaticClass()); auto OversizedB=MakeNode(UString_SOV::StaticClass(),OversizedA.Identifier);
    FGorgeousOVPropertyCapture Huge; Huge.PropertyName=TEXT("Value"); Huge.ExportedValue=FString::ChrN(300000,TEXT('a'));
    OversizedA.Properties.Add(Huge); OversizedB.Properties.Add(Huge);
    TestNull(TEXT("Aggregate text budget applies across all nodes, not only each property"),Decode({OversizedA,OversizedB}));
    auto NestedObject=MakeNode(UObject_AOTOV::StaticClass());
    FGorgeousOVPropertyCapture Reference; Reference.PropertyName=TEXT("Value"); Reference.ExportedValue=TEXT("(/Game/ShouldNeverLoad.Asset)"); NestedObject.Properties.Add(Reference);
    TestNull(TEXT("Object references nested in containers rejected before ImportText"),Decode({NestedObject}));
    auto UnsafeStruct=MakeNode(UGorgeousRPCUnsafeImportAuditVariable::StaticClass()); UnsafeStruct.Properties.Add(Reference);
    FGorgeousRPCUnsafeImportAudit::ImportCalls=0;
    TestNull(TEXT("Nested native custom parser is not allowed on untrusted ingress"),Decode({UnsafeStruct}));
    TestEqual(TEXT("Native import side effect witness never executes"),FGorgeousRPCUnsafeImportAudit::ImportCalls,0);
    auto TextNode=MakeNode(UText_SOV::StaticClass()); Reference.ExportedValue=TEXT("LOCTABLE(\"/Game/ShouldNeverLoad.Asset\",\"Key\")"); TextNode.Properties.Add(Reference);
    TestNull(TEXT("String-table FText asset loading form rejected"),Decode({TextNode}));
    auto BadInteger=Root; BadInteger.Properties[0].ExportedValue=TEXT("271 trailing-data");
    TestNull(TEXT("Parser trailing input rejected rather than silently accepted"),Decode({BadInteger}));
    auto* Native=NewObject<UInteger_SOV>(); int32 Number=937; Native->SetIntegerSingleObjectVariable_Implementation(NAME_None,Number);
    Native->VariableRegistry.Add(TEXT("Cycle"),Native);
    TArray<FGorgeousSerializedOVNode> Captured;
    TestFalse(TEXT("Sender capture rejects cyclic local tree instead of recursing forever"),UGorgeousAutoReplicationRPCRelayComponent::CaptureResultTreeForAutomation(Native,Captured));
    TestTrue(TEXT("Rejected capture leaves no partial payload"),Captured.IsEmpty());
    Native->VariableRegistry.Reset();
    TestTrue(TEXT("Unregistered native return OV receives valid capture identity"),UGorgeousAutoReplicationRPCRelayComponent::CaptureResultTreeForAutomation(Native,Captured));
    TestNotNull(TEXT("Captured native return OV roundtrips"),Decode(Captured));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousRootGCSafetyRegression,"GorgeousCore.LimitationAudit.RootRegistryGCSafety",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousRootGCSafetyRegression::RunTest(const FString&)
{
    const FName Name(*FString::Printf(TEXT("AuditGCRoot_%s"),*FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    auto* Root=NewObject<UGorgeousRootObjectVariable>();
    Root->AddToRoot();
    UGorgeousRootObjectVariable::NamedRootInstances.Add(Name,Root);
    TWeakObjectPtr<UGorgeousRootObjectVariable> WeakRoot(Root);
    TestEqual(TEXT("Registered rooted instance resolves"),UGorgeousRootObjectVariable::TryGetExistingRoot(Name),Root);
    Root->RemoveFromRoot();
    Root=nullptr;
    CollectGarbage(RF_NoFlags);
    TestFalse(TEXT("Unrooted registry entries do not keep retired PIE roots alive"),WeakRoot.IsValid());
    TestNull(TEXT("Collected root never dereferenced through static registry"),UGorgeousRootObjectVariable::TryGetExistingRoot(Name));
    TestFalse(TEXT("BeginDestroy unregisters exact root registration"),UGorgeousRootObjectVariable::NamedRootInstances.Contains(Name));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousReplicationSlotOrderingRegression,"GorgeousCore.LimitationAudit.ReplicationSlotOrdering",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousReplicationSlotOrderingRegression::RunTest(const FString&)
{
    TStrongObjectPtr<UGorgeousTrunkAuditOwner> Owner(NewObject<UGorgeousTrunkAuditOwner>());
    Owner->Mixin.Bind(Owner.Get(),&Owner->AdditionalData,&Owner->ReplicatedData);
    Owner->Mixin.InitializeAdditionalData(true);
    TStrongObjectPtr<UGorgeousTrunkAuditOwner> Sender(NewObject<UGorgeousTrunkAuditOwner>());
    Sender->Mixin.Bind(Sender.Get(),&Sender->AdditionalData,&Sender->ReplicatedData);
    auto* Source=NewObject<UInteger_SOV>(Sender.Get());
    auto& SourceSlot=Sender->AdditionalData.FindOrAdd(TEXT("AuditOrder"));
    SourceSlot.DefaultValue=Source; SourceSlot.bReplicate=true;
    Sender->Mixin.InitializeAdditionalData(true); // Actual public owner binding path.
    int32 Initial=741;
    Source->SetIntegerSingleObjectVariable_Implementation(NAME_None,Initial);
    FGorgeousAutoReplicationConditionContext Context; Context.bIsInitialState=true;
    FGorgeousAutoReplicationPropertyEnvelope Envelope;
    Envelope.EntryKey=TEXT("AuditOrder"); Envelope.VariableClass=UInteger_SOV::StaticClass();
    if (!TestTrue(TEXT("Actual sender builds the populated initial payload"),Source->BuildAutoReplicationPropertyPayload(Context,Envelope.Payload))) return false;
    Owner->Mixin.HandleTransportedPropertyPayload(Envelope);
    UGorgeousObjectVariable* Hydrated=nullptr;
    if (!TestTrue(TEXT("RPC-first delivery materializes the concrete slot through actual receiver"),Owner->Mixin.TryGetValue(Envelope.EntryKey,Hydrated) && Hydrated)) return false;
    TestEqual(TEXT("Received payload has concrete value before metadata arrives"),IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Hydrated,NAME_None),741);
    if (!TestTrue(TEXT("Receiver registered native metadata slot"),Owner->ReplicatedData.Num()==1)) return false;
    auto& Metadata=Owner->ReplicatedData[0];
    const FGuid ServerIdentifier=FGuid::NewGuid();
    Metadata.Value=nullptr; // Server's non-native OV reference has not mapped yet.
    Metadata.ValueClass=UInteger_SOV::StaticClass(); Metadata.VariableIdentifier=ServerIdentifier;
    Owner->Mixin.RefreshCachedValues(); // The actual native OnRep entrypoint.
    UGorgeousObjectVariable* Resolved=nullptr;
    Owner->Mixin.TryGetValue(Envelope.EntryKey,Resolved);
    TestEqual(TEXT("Late native metadata preserves the already received slot instance"),Resolved,Hydrated);
    if (Resolved)
    {
        TestEqual(TEXT("Late metadata cannot reset the received value to its class default"),IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Resolved,NAME_None),741);
        TestEqual(TEXT("Retained instance adopts authoritative metadata identity"),Resolved->UniqueIdentifier,ServerIdentifier);
    }
    Metadata.Value=nullptr;
    Owner->Mixin.RefreshCachedValues();
    Owner->Mixin.TryGetValue(Envelope.EntryKey,Resolved);
    TestEqual(TEXT("Repeated unresolved metadata remains idempotent"),Resolved,Hydrated);
    Metadata.Value=nullptr; Metadata.ValueClass=UString_SOV::StaticClass(); Metadata.VariableIdentifier=FGuid::NewGuid();
    Owner->Mixin.RefreshCachedValues();
    Owner->Mixin.TryGetValue(Envelope.EntryKey,Resolved);
    TestTrue(TEXT("A genuine class replacement does not reuse incompatible hydrated storage"),Resolved && Resolved->IsA<UString_SOV>());

    UWorld* EditorWorld=nullptr;
    if (GEngine) for (const FWorldContext& WorldContext:GEngine->GetWorldContexts())
        if (WorldContext.WorldType==EWorldType::Editor && WorldContext.World()) { EditorWorld=WorldContext.World(); break; }
    if (!TestNotNull(TEXT("Early-receive regression requires an editor world without BeginPlay"),EditorWorld)) return false;
    TArray<AGorgeousPlayerController*> Controllers;
    ON_SCOPE_EXIT
    {
        for (AGorgeousPlayerController* Controller:Controllers)
            if (IsValid(Controller))
            {
                FGorgeousQualityOfLifeStatics::RemoveOwnerFromSelfReference(Controller,Controller->AdditionalGorgeousData);
                Controller->SetRole(ROLE_Authority);
                Controller->Destroy(true,false);
            }
    };
    auto CheckEarlyReceive=[&](bool bConfigured,bool bExplicitlyDisabled,const TCHAR* Label)
    {
        auto* Controller=EditorWorld->SpawnActor<AGorgeousPlayerController>();
        if (!TestNotNull(Label,Controller)) return;
        Controllers.Add(Controller);
        Controller->SetRole(ROLE_AutonomousProxy);
        Controller->bActivateNetworkingCapabilities=bConfigured;
        TestFalse(TEXT("Early-receive fixture has not begun play"),Controller->HasActorBegunPlay());
        TestFalse(TEXT("Early-receive fixture is a remote actor"),Controller->HasAuthority());
        FGorgeousAutoReplicationMixin& Receiver=Controller->GetAutoReplicationMixin();
        if (bExplicitlyDisabled) Receiver.InitializeAdditionalData(false);
        TestFalse(TEXT("Incoming payload precedes mixin network initialization"),Receiver.IsNetworkingEnabled());
        FGorgeousAutoReplicationPropertyEnvelope EarlyEnvelope=Envelope;
        EarlyEnvelope.EntryKey=TEXT("AuditEarly");
        EarlyEnvelope.OwnerActor=Controller;
        Receiver.HandleTransportedPropertyPayload(EarlyEnvelope);
        UGorgeousObjectVariable* EarlyVariable=nullptr;
        const bool bReceived=Receiver.TryGetValue(EarlyEnvelope.EntryKey,EarlyVariable) && EarlyVariable;
        if (bConfigured && !bExplicitlyDisabled)
        {
            TestTrue(TEXT("Configured early remote receive initializes its slot instead of dropping the initial payload"),bReceived);
            if (EarlyVariable)
                TestEqual(TEXT("Initial value survives reception before actor BeginPlay"),IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(EarlyVariable,NAME_None),741);
        }
        else
        {
            TestFalse(TEXT("Early receive never overrides disabled configuration or explicit initialization opt-out"),bReceived);
            TestFalse(TEXT("Disabled receiver remains disabled"),Receiver.IsNetworkingEnabled());
        }
    };
    CheckEarlyReceive(true,false,TEXT("Create configured early receiver"));
    CheckEarlyReceive(false,false,TEXT("Create config-disabled early receiver"));
    CheckEarlyReceive(true,true,TEXT("Create explicitly disabled early receiver"));
    return true;
}

#endif
