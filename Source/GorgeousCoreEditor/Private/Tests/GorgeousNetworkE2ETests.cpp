#include "Tests/GorgeousNetworkAuditActors.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "SharedTests/GorgeousLimitationAuditTestTypes.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NativeGameplayTags.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStorage_OV.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationSettings.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationListenerStructures.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Engine/GameInstance.h"
#include "GeneralSystems/GorgeousPermissionManagerSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "GeneralSystems/VisualDataGathering/GorgeousVDGConsent.h"
#include "GeneralSystems/VisualDataGathering/GorgeousVDGStoreRouting.h"
#include "AutoReplication/Globals/GorgeousAutoReplicationRPCPayloadGlobals.h"
#include "UObject/StructOnScope.h"

template<typename T> static void SetAuditInteger(T* Owner,FName Key,int32 Value)
{
    if (!Owner) return;
    Owner->RegisterAutoReplicationEntry(Key,UInteger_SOV::StaticClass(),true,false,FGorgeousAutoReplicationStreamConfig());
    UGorgeousObjectVariable* Variable=nullptr;
    if (Owner->GetAutoReplicationMixin().TryGetValue(Key,Variable) && Variable)
        IGorgeousSingleObjectVariablesSetter_I::Execute_SetIntegerSingleObjectVariable(Variable,NAME_None,Value);
}
AGorgeousNetworkAuditActor::AGorgeousNetworkAuditActor()
{
    bReplicates=true; bAlwaysRelevant=true;
    AutoReplication=CreateDefaultSubobject<UGorgeousAutoReplicationOwnerComponent>(TEXT("AutoReplication"));
    Stats=CreateDefaultSubobject<UGorgeousStatComponent_AC>(TEXT("Stats"));
}
void AGorgeousNetworkAuditActor::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        FSignalBridgeEventDelegate Listener;
        Listener.BindDynamic(this,&AGorgeousNetworkAuditActor::OnAuditStatSignal);
        USignalBridgeBlueprintFunctionLibrary::Listen(this,GetDefault<UGorgeousStatFoundationSettings>()->ChangeSignalRoot,nullptr,Listener);
        Stats->SetStat(GetGorgeousNetworkAuditStatTag(),913);
    }
    if (HasAuthority())
        if (auto* Variable=AutoReplication->RegisterObjectVariable(TEXT("AuditActor"),UInteger_SOV::StaticClass(),FGorgeousAutoReplicationStreamConfig()))
        {
            int32 InitialValue=901;
            IGorgeousSingleObjectVariablesSetter_I::Execute_SetIntegerSingleObjectVariable(Variable,NAME_None,InitialValue);
        }
}
void AGorgeousNetworkAuditActor::OnAuditStatSignal(FGameplayTag SignalTag,const FInstancedStruct& Payload)
{
    if (const auto* StatPayload=Payload.GetPtr<FGorgeousStatModificationPayload_S>())
        bObservedStatParentSignal |= SignalTag==GetDefault<UGorgeousStatFoundationSettings>()->ChangeSignalRoot &&
            StatPayload->Target==this && StatPayload->Stat==GetGorgeousNetworkAuditStatTag() && StatPayload->Value==913;
}
AGorgeousNetworkAuditGameMode::AGorgeousNetworkAuditGameMode()
{
    PlayerControllerClass=AGorgeousNetworkAuditController::StaticClass();
    PlayerStateClass=AGorgeousPlayerState::StaticClass();
    GameStateClass=AGorgeousGameState::StaticClass();
    DefaultPawnClass=nullptr;
    bDelayedStart=false; // Exercise actual world BeginPlay and component initialization.
}
void AGorgeousNetworkAuditGameMode::StartPlay()
{
    Super::StartPlay();
    GetWorld()->SpawnActor<AGorgeousNetworkAuditActor>();
    SetAuditInteger(GetWorld()->GetGameState<AGorgeousGameState>(),TEXT("AuditShared"),701);
    if (auto* GS=GetWorld()->GetGameState<AGorgeousGameState>()) GS->RegisterAutoReplicationEntry(TEXT("AuditRPC"),UGorgeousNetworkAuditRPCVariable::StaticClass(),true,false,FGorgeousAutoReplicationStreamConfig());
    SetAuditInteger(Cast<AGorgeousWorldSettings>(GetWorld()->GetWorldSettings()),TEXT("AuditWorld"),801);
    if (auto* GS=GetWorld()->GetGameState<AGorgeousGameState>())
    {
        GS->RegisterAutoReplicationEntry(TEXT("AuditSubscribedChannel"),UInteger_SOV::StaticClass(),true,false,FGorgeousAutoReplicationStreamConfig());
        UGorgeousObjectVariable* Variable=nullptr;
        if (GS->GetAutoReplicationMixin().TryGetValue(TEXT("AuditSubscribedChannel"),Variable) && Variable)
        {
            Variable->SetSharedNetworkStackEnabled(true);
            Variable->RootNetworkConfig.ReplicationChannel=TEXT("Audit.RestrictedChannel");
            int32 Initial=777;
            IGorgeousSingleObjectVariablesSetter_I::Execute_SetIntegerSingleObjectVariable(Variable,NAME_None,Initial);
        }
    }
}
void AGorgeousNetworkAuditGameMode::PostLogin(APlayerController* Player)
{
    Super::PostLogin(Player);
    if (auto* PC=Cast<AGorgeousNetworkAuditController>(Player))
    {
        const int32 Id=PC->PlayerState ? PC->PlayerState->GetPlayerId() : -1;
        SetAuditInteger(PC,TEXT("AuditPrivate"),1000+Id);
        SetAuditInteger(Cast<AGorgeousPlayerState>(PC->PlayerState),TEXT("AuditPlayerState"),2000+Id);
    }
}

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "FileHelpers.h"
#include "UObject/StrongObjectPtr.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_CustomEvent.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousVDGConsentGraphRegression,"GorgeousCore.LimitationAudit.VDGConsentGraph",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousVDGConsentGraphRegression::RunTest(const FString&)
{
    auto* Blueprint=LoadObject<UBlueprint>(nullptr,TEXT("/GorgeousCore/Systems/VisualDataGeathering/Classes/ActorComponents/BP_VisualDataGeatheringInteractor_AC"));
    if (!TestNotNull(TEXT("Actual interactor Blueprint loads"),Blueprint)) return false;
    TArray<UEdGraph*> Graphs; Blueprint->GetAllGraphs(Graphs);
    UK2Node_CustomEvent* Entry=nullptr; UK2Node_AsyncAction* Gate=nullptr; int32 Gates=0;
    for (UEdGraph* Graph:Graphs) for (UEdGraphNode* Node:Graph->Nodes)
    {
        if (auto* Event=Cast<UK2Node_CustomEvent>(Node)) if (Event->CustomFunctionName==TEXT("GeatherBySchemaClient")) Entry=Event;
        if (auto* Action=Cast<UK2Node_AsyncAction>(Node))
            if (Action->GetFactoryFunction()==UAsyncAction_RequestVDGConsent::StaticClass()->FindFunctionByName(TEXT("RequestVDGConsent"))) { Gate=Action; ++Gates; }
    }
    if (!TestNotNull(TEXT("Existing gathering entry remains"),Entry) || !TestNotNull(TEXT("Explicit consent gate exists in shipped graph"),Gate)) return false;
    TestEqual(TEXT("Exactly one consent gate prevents repeated dialog insertion"),Gates,1);
    auto* Then=Entry->FindPin(UEdGraphSchema_K2::PN_Then);
    auto* Execute=Gate->FindPin(UEdGraphSchema_K2::PN_Execute);
    auto* Accepted=Gate->FindPin(TEXT("OnAccepted"));
    auto* Declined=Gate->FindPin(TEXT("OnDeclined"));
    TestTrue(TEXT("Gathering starts at consent, with no direct construction bypass"),Then && Execute && Then->LinkedTo.Num()==1 && Then->LinkedTo[0]==Execute);
    TestTrue(TEXT("Explicit acceptance resumes the original collection path"),Accepted && Accepted->LinkedTo.Num()==1);
    TestTrue(TEXT("Declining does not execute collection"),Declined && Declined->LinkedTo.IsEmpty());
    return true;
}

struct FGorgeousNetworkAuditState
{
    FAutomationTestBase* Test=nullptr;
    bool bDedicated=false;
    bool bChannelSubscriptionTest=false;
    bool bVDGConsentTest=false;
    TWeakObjectPtr<UActorComponent> VDGManager;
    TWeakObjectPtr<AGorgeousPlayerController> VDGClient;
    TWeakObjectPtr<AGorgeousPlayerController> VDGServerController;
    FString VDGStableId;
    int32 ChannelRecipientId=INDEX_NONE;
    TWeakObjectPtr<APlayerController> ChannelRecipient;
    int32 Phase=0;
    double PhaseStarted=0;
    TStrongObjectPtr<ULevelEditorPlaySettings> Settings;
    TSubclassOf<AWorldSettings> PreviousWorldSettingsClass;
    TWeakObjectPtr<UWorld> Server;
    TStrongObjectPtr<UGorgeousAutoReplicationRPCRequestAsyncAction> DeferredAction;
    TStrongObjectPtr<UGorgeousAutoReplicationRPCRequestAsyncAction> MulticastAction;
    TStrongObjectPtr<UGorgeousNetworkAuditCallbacks> DeferredCallbacks;
    TStrongObjectPtr<UGorgeousNetworkAuditCallbacks> MulticastCallbacks;
    int32 InitialClientWorlds=2;
};
static UGorgeousObjectVariable* FindAuditVariable(FGorgeousAutoReplicationMixin& Mixin,FName Key)
{
    UGorgeousObjectVariable* Variable=nullptr;
    Mixin.TryGetValue(Key,Variable);
    return Variable;
}
static int32 AuditIntegerValue(FGorgeousAutoReplicationMixin& Mixin,FName Key)
{
    UGorgeousObjectVariable* Variable=FindAuditVariable(Mixin,Key);
    return Variable ? IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Variable,NAME_None) : MIN_int32;
}
static bool ReadAuditInteger(FGorgeousAutoReplicationMixin& Mixin,FName Key,int32 Expected)
{
    UGorgeousObjectVariable* Variable=nullptr;
    return Mixin.TryGetValue(Key,Variable) && Variable && IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Variable,NAME_None)==Expected;
}
static bool MakeVDGAuditPayload(UActorComponent* Manager,const FString& PlayerId,int32 Value,FGorgeousRPCPayload& Payload)
{
    UFunction* Function=Manager ? Manager->FindFunction(TEXT("WriteVDG Property")) : nullptr;
    if (!Function) return false;
    FStructOnScope Parameters(Function);
    FStructProperty* Property=FindFProperty<FStructProperty>(Function,TEXT("Property"));
    if (!Property) return false;
    void* PropertyData=Property->ContainerPtrToValuePtr<void>(Parameters.GetStructMemory());
    FNameProperty* Identifier=nullptr;
    for (TFieldIterator<FProperty> It(Property->Struct);It;++It)
        if (It->GetName().StartsWith(TEXT("PropertyIdentifier"))) Identifier=CastField<FNameProperty>(*It);
    if (!Identifier) return false;
    Identifier->SetPropertyValue_InContainer(PropertyData,TEXT("VDGConsentAudit"));
    Payload=UGorgeousAutoReplicationRPCPayloadGlobals::MakeAutoReplicationRPCPayload(TEXT("WriteVDG Property"));
    Payload.TimeoutSeconds=5;
    if (!UGorgeousAutoReplicationRPCPayloadGlobals::AddArgumentFromProperty(Payload,TEXT("Property"),Property,PropertyData)) return false;
    UGorgeousAutoReplicationRPCPayloadGlobals::AddAutoReplicationRPCStringArgument(Payload,TEXT("PlayerStableID"),PlayerId);
    FProperty* ValueProperty=FindFProperty<FObjectProperty>(Function,TEXT("Value"));
    auto* Number=NewObject<UInteger_SOV>(); Number->SetIntegerSingleObjectVariable_Implementation(NAME_None,Value);
    UGorgeousObjectVariable* NumberObject=Number;
    return ValueProperty && UGorgeousAutoReplicationRPCPayloadGlobals::AddArgumentFromProperty(Payload,TEXT("Value"),ValueProperty,&NumberObject);
}
static bool InitializeVDGAuditSystem(UActorComponent* Manager)
{
    // Bundled systems enroll through this existing initialization API; adding
    // an ActorComponent alone does not add it to GeneralRuntime.ActiveSystems.
    UFunction* Initialize=Manager ? Manager->FindFunction(TEXT("FinishInitialisation")) : nullptr;
    if (!Initialize || Initialize->NumParms) return false;
    Manager->ProcessEvent(Initialize,nullptr);
    UClass* Library=StaticLoadClass(UObject::StaticClass(),nullptr,TEXT("/GorgeousCore/Systems/General/BP_General_BFL.BP_General_BFL_C"));
    UFunction* GetRuntime=Library ? Library->FindFunctionByName(TEXT("GetGeneralRuntime")) : nullptr;
    if (!GetRuntime) return false;
    FStructOnScope RuntimeParameters(GetRuntime);
    for (TFieldIterator<FProperty> It(GetRuntime);It;++It)
        if (It->HasAnyPropertyFlags(CPF_Parm) && !It->HasAnyPropertyFlags(CPF_OutParm|CPF_ReturnParm))
            if (auto* Object=CastField<FObjectProperty>(*It)) Object->SetObjectPropertyValue_InContainer(RuntimeParameters.GetStructMemory(),Manager);
    Library->GetDefaultObject()->ProcessEvent(GetRuntime,RuntimeParameters.GetStructMemory());
    UObject* Runtime=nullptr;
    for (TFieldIterator<FProperty> It(GetRuntime);It;++It)
        if (It->HasAnyPropertyFlags(CPF_ReturnParm|CPF_OutParm))
            if (auto* Object=CastField<FObjectProperty>(*It)) Runtime=Object->GetObjectPropertyValue_InContainer(RuntimeParameters.GetStructMemory());
    UFunction* GetActive=Runtime ? Runtime->FindFunction(TEXT("GetActiveSystems")) : nullptr;
    if (!GetActive) return false;
    FStructOnScope ActiveParameters(GetActive);
    Runtime->ProcessEvent(GetActive,ActiveParameters.GetStructMemory());
    for (TFieldIterator<FProperty> It(GetActive);It;++It)
        if (It->HasAnyPropertyFlags(CPF_ReturnParm|CPF_OutParm))
            if (auto* Array=CastField<FArrayProperty>(*It))
                if (auto* Object=CastField<FObjectProperty>(Array->Inner))
                {
                    FScriptArrayHelper Items(Array,Array->ContainerPtrToValuePtr<void>(ActiveParameters.GetStructMemory()));
                    for (int32 Index=0;Index<Items.Num();++Index)
                        if (Object->GetObjectPropertyValue(Items.GetRawPtr(Index))==Manager) return true;
                }
    return false;
}
static UGorgeousObjectVariable* ReadVDGAuditAnswer(UActorComponent* Manager,const FString& PlayerId)
{
    UFunction* Function=Manager ? Manager->FindFunction(TEXT("GetContainerValue")) : nullptr;
    if (!Function) return nullptr;
    FStructOnScope Parameters(Function);
    for (TFieldIterator<FProperty> It(Function);It;++It)
    {
        if (It->GetFName()==TEXT("PlayerStableID"))
            if (auto* String=CastField<FStrProperty>(*It)) String->SetPropertyValue_InContainer(Parameters.GetStructMemory(),PlayerId);
        if (It->GetFName()==TEXT("PropertyIdentifier"))
            if (auto* Name=CastField<FNameProperty>(*It)) Name->SetPropertyValue_InContainer(Parameters.GetStructMemory(),TEXT("VDGConsentAudit"));
    }
    // This public Blueprint function takes its source map explicitly; leaving
    // TargetContainer default-initialized only tests an empty temporary map.
    auto* TargetContainer=FindFProperty<FMapProperty>(Function,TEXT("TargetContainer"));
    if (!TargetContainer) return nullptr;
    for (FName StoreName:{FName(TEXT("MemberProperties")),FName(TEXT("GlobalProperties"))})
    {
        auto* Store=FindFProperty<FMapProperty>(Manager->GetClass(),StoreName);
        if (!Store || !TargetContainer->SameType(Store)) continue;
        TargetContainer->CopyCompleteValue(TargetContainer->ContainerPtrToValuePtr<void>(Parameters.GetStructMemory()),Store->ContainerPtrToValuePtr<void>(Manager));
        Manager->ProcessEvent(Function,Parameters.GetStructMemory());
        for (TFieldIterator<FProperty> It(Function);It;++It)
            if (It->HasAnyPropertyFlags(CPF_ReturnParm|CPF_OutParm))
                if (auto* Object=CastField<FObjectProperty>(*It))
                    if (auto* Value=Cast<UGorgeousObjectVariable>(Object->GetObjectPropertyValue_InContainer(Parameters.GetStructMemory()))) return Value;
    }
    return nullptr;
}
class FGorgeousNetworkAuditLatent final : public IAutomationLatentCommand
{
    TSharedPtr<FGorgeousNetworkAuditState> State;
public:
    explicit FGorgeousNetworkAuditLatent(TSharedPtr<FGorgeousNetworkAuditState> In):State(In) {}
    virtual bool Update() override
    {
        auto& S=*State;
        TArray<UWorld*> Clients; UWorld* Server=nullptr;
        for (const FWorldContext& Context:GEngine->GetWorldContexts())
        {
            UWorld* World=Context.World();
            if (!World || Context.WorldType!=EWorldType::PIE) continue;
            if (World->GetNetMode()==NM_Client) Clients.Add(World);
            else if (World->GetNetMode()==NM_ListenServer || World->GetNetMode()==NM_DedicatedServer) Server=World;
        }
        const double Elapsed=FPlatformTime::Seconds()-S.PhaseStarted;
        auto Cleanup=[&]() { if (GEditor) GEditor->RequestEndPlayMap(); S.Phase=3; S.PhaseStarted=FPlatformTime::Seconds(); };
        auto AuditRPCReady=[](UWorld* World) {
            auto* State=World ? World->GetGameState<AGorgeousGameState>() : nullptr;
            UGorgeousObjectVariable* Variable=nullptr;
            return State && State->GetAutoReplicationMixin().TryGetValue(TEXT("AuditRPC"),Variable) &&
                IsValid(Variable) && Variable->IsA<UGorgeousNetworkAuditRPCVariable>() &&
                Variable->IsRPCBindingAllowed(TEXT("AuditDeferred"),EGorgeousAutoReplicationRPCType::EReliableServer) &&
                Variable->IsRPCBindingAllowed(TEXT("AuditImmediate"),EGorgeousAutoReplicationRPCType::EReliableMulticast);
        };
        auto Ready=[&](int32 Count) {
            if (!Server || Clients.Num()<Count) return false;
            // Integer delivery does not imply that a separate RPC entry and
            // its native handler bindings have reached each participant yet.
            const bool bNeedsRPC=!S.bVDGConsentTest && !S.bChannelSubscriptionTest;
            if (bNeedsRPC && !AuditRPCReady(Server)) return false;
            for (UWorld* World:Clients)
            {
                auto* PC=Cast<AGorgeousNetworkAuditController>(World->GetFirstPlayerController());
                auto* GS=World->GetGameState<AGorgeousGameState>();
                auto* WS=Cast<AGorgeousWorldSettings>(World->GetWorldSettings());
                auto* PS=PC ? Cast<AGorgeousPlayerState>(PC->PlayerState) : nullptr;
                if (!PC || !GS || !WS || !PS) return false;
                if (bNeedsRPC && !AuditRPCReady(World)) return false;
                bool bActorReady=false;
                for (TActorIterator<AGorgeousNetworkAuditActor> It(World);It;++It)
                    bActorReady|=ReadAuditInteger(It->AutoReplication->GetAutoReplicationMixin(),TEXT("AuditActor"),901) &&
                        It->Stats->GetStatStorage() && It->Stats->GetStat(GetGorgeousNetworkAuditStatTag())==913;
                if (!bActorReady) return false;
                const int32 Id=PS->GetPlayerId();
                if (!ReadAuditInteger(PC->GetAutoReplicationMixin(),TEXT("AuditPrivate"),1000+Id) ||
                    !ReadAuditInteger(PS->GetAutoReplicationMixin(),TEXT("AuditPlayerState"),2000+Id) ||
                    !ReadAuditInteger(GS->GetAutoReplicationMixin(),TEXT("AuditShared"),701) ||
                    !ReadAuditInteger(WS->GetAutoReplicationMixin(),TEXT("AuditWorld"),801)) return false;
            }
            return true;
        };
        if (S.Phase==0)
        {
            if (Ready(2))
            {
                S.Test->AddInfo(TEXT("Two connected clients received distinct PC values, their PlayerState values, shared GameState and WorldSettings values."));
                if (S.bVDGConsentTest)
                {
                    auto* GS=Server->GetGameState<AGorgeousGameState>();
                    UClass* Class=StaticLoadClass(UActorComponent::StaticClass(),nullptr,TEXT("/GorgeousCore/Systems/VisualDataGeathering/Classes/ActorComponents/BP_VisualDataGeatheringManager_AC.BP_VisualDataGeatheringManager_AC_C"));
                    if (!Class || !GS) { S.Test->AddError(TEXT("Actual VDG manager class/server GameState unavailable")); Cleanup(); return false; }
                    auto* Manager=NewObject<UActorComponent>(GS,Class);
                    GS->AddInstanceComponent(Manager); Manager->RegisterComponent(); S.VDGManager=Manager;
                    if (!S.Test->TestTrue(TEXT("Existing FinishInitialisation enrolls the actual VDG manager in GeneralRuntime.GetActiveSystems"),InitializeVDGAuditSystem(Manager))) { Cleanup(); return false; }
                    S.Test->TestTrue(TEXT("Installed Team and Playlist assets do not suppress the sole active VDG writer"),UGorgeousVDGStoreRouting::ShouldUseLocalVDGStore(Manager));
                    UClass* BaseSystem=StaticLoadClass(UActorComponent::StaticClass(),nullptr,TEXT("/GorgeousCore/Systems/General/BP_GeneralSystem_AC.BP_GeneralSystem_AC_C"));
                    if (!BaseSystem) { S.Test->AddError(TEXT("Existing harmless GeneralSystem fixture base is unavailable")); Cleanup(); return false; }
                    auto* TeamFixture=NewObject<UActorComponent>(GS,BaseSystem);
                    auto* PlaylistFixture=NewObject<UActorComponent>(GS,BaseSystem);
                    TeamFixture->ComponentTags.Add(TEXT("Team_System")); PlaylistFixture->ComponentTags.Add(TEXT("Playlist_System"));
                    GS->AddInstanceComponent(TeamFixture); TeamFixture->RegisterComponent();
                    const bool bTeamReady=InitializeVDGAuditSystem(TeamFixture);
                    S.Test->TestTrue(TEXT("One enrolled delegated system preserves the existing local-writer condition"),bTeamReady && UGorgeousVDGStoreRouting::ShouldUseLocalVDGStore(Manager));
                    GS->AddInstanceComponent(PlaylistFixture); PlaylistFixture->RegisterComponent();
                    const bool bPlaylistReady=InitializeVDGAuditSystem(PlaylistFixture);
                    S.Test->TestTrue(TEXT("Both enrolled delegated systems suppress the local writer"),bTeamReady && bPlaylistReady && !UGorgeousVDGStoreRouting::ShouldUseLocalVDGStore(Manager));
                    for (UActorComponent* Fixture:{TeamFixture,PlaylistFixture})
                    {
                        UFunction* Shutdown=Fixture->FindFunction(TEXT("FinishShutdown"));
                        if (Shutdown && !Shutdown->NumParms) Fixture->ProcessEvent(Shutdown,nullptr);
                        else S.Test->AddError(TEXT("Existing GeneralSystem unregister API is unavailable"));
                        Fixture->DestroyComponent();
                    }
                    S.Test->TestTrue(TEXT("Unregistered delegated systems restore the sole active VDG writer"),UGorgeousVDGStoreRouting::ShouldUseLocalVDGStore(Manager));
                    auto* PC=Cast<AGorgeousPlayerController>(Clients[0]->GetFirstPlayerController()); S.VDGClient=PC;
                    const int32 Id=PC->PlayerState->GetPlayerId();
                    for (auto It=Server->GetPlayerControllerIterator();It;++It)
                        if (It->Get() && It->Get()->PlayerState && It->Get()->PlayerState->GetPlayerId()==Id) S.VDGServerController=Cast<AGorgeousPlayerController>(It->Get());
                    if (!S.VDGServerController.IsValid()) { S.Test->AddError(TEXT("VDG authority player identity unavailable")); Cleanup(); return false; }
                    auto* AuthorityState=CastChecked<AGorgeousPlayerState>(S.VDGServerController->PlayerState);
                    S.VDGStableId=AuthorityState->GetGorgeousStablePlayerId_Implementation();
                    if (S.VDGStableId.IsEmpty())
                    {
                        S.VDGStableId=FString::Printf(TEXT("VDG-Audit-%d"),Id);
                        AuthorityState->SetGorgeousStablePlayerId(S.VDGStableId);
                    }
                    FGorgeousRPCPayload Payload;
                    if (!MakeVDGAuditPayload(Manager,S.VDGStableId,337,Payload)) { S.Test->AddError(TEXT("Cannot construct actual VDG receiver arguments")); Cleanup(); return false; }
                    S.DeferredAction.Reset(UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(PC,TEXT("RequestVDGPropertyWrite"),EGorgeousAutoReplicationRPCType::EReliableServer,Payload,EGorgeousAutoReplicationTargetKind::EActorComponent,Clients[0]->GetGameState<AGorgeousGameState>()));
                    S.DeferredCallbacks.Reset(NewObject<UGorgeousNetworkAuditCallbacks>());
                    S.DeferredAction->OnFailed.AddDynamic(S.DeferredCallbacks.Get(),&UGorgeousNetworkAuditCallbacks::OnFailure);
                    S.DeferredAction->Activate();
                    S.Test->TestEqual(TEXT("Default-off VDG request fails locally without sending data"),S.DeferredCallbacks->Failed,1);
                    S.Test->TestFalse(TEXT("Server has not implicitly granted VDG consent"),S.VDGServerController->HasVisualDataGatheringConsent());
                    PC->SetVisualDataGatheringConsent(true);
                    S.Phase=10; S.PhaseStarted=FPlatformTime::Seconds(); return false;
                }
                if (S.bChannelSubscriptionTest)
                {
                    if (Elapsed<2) return false;
                    auto* Stack=UGorgeousRootNetworkStackSubsystem::Get(Server);
                    if (!Stack) { S.Test->AddError(TEXT("Authority channel subsystem missing.")); Cleanup(); return false; }
                    S.ChannelRecipientId=Clients[0]->GetFirstPlayerController()->PlayerState->GetPlayerId();
                    for (UWorld* World:Clients)
                        S.Test->TestFalse(TEXT("An unjoined named channel never admits a policy-allowed client"),ReadAuditInteger(World->GetGameState<AGorgeousGameState>()->GetAutoReplicationMixin(),TEXT("AuditSubscribedChannel"),777));
                    for (FConstPlayerControllerIterator It=Server->GetPlayerControllerIterator();It;++It)
                        if (It->Get() && It->Get()->PlayerState && It->Get()->PlayerState->GetPlayerId()==S.ChannelRecipientId) S.ChannelRecipient=It->Get();
                    if (!S.ChannelRecipient.IsValid()) { S.Test->AddError(TEXT("Cannot identify selected recipient on authority.")); Cleanup(); return false; }
                    Stack->SubscribeControllerToChannel(S.ChannelRecipient.Get(),TEXT("Audit.RestrictedChannel"));
                    SetAuditInteger(Server->GetGameState<AGorgeousGameState>(),TEXT("AuditSubscribedChannel"),778);
                    S.Phase=4; S.PhaseStarted=FPlatformTime::Seconds();
                    return false;
                }
                for (UWorld* World:Clients)
                {
                    auto* PC=Cast<AGorgeousNetworkAuditController>(World->GetFirstPlayerController());
                    auto* Relay=PC->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>();
                    if (!Relay) { S.Test->AddError(TEXT("Owning client has no relay component.")); Cleanup(); return false; }
                    FGorgeousQueuedRPC Hostile;
                    Hostile.Key=TEXT("AuditPrivate"); Hostile.Type=EGorgeousAutoReplicationRPCType::EReliableServer;
                    Hostile.TargetKind=EGorgeousAutoReplicationTargetKind::EOwner; Hostile.TargetOwnerActor=PC; Hostile.bHasTargetOwnerActor=true;
                    Hostile.Payload.HandlerName=TEXT("UnregisteredAuditFunction"); Hostile.RequestGuid=FGuid::NewGuid();
                    Relay->RelayRPCToServer(Hostile,true,&PC->GetAutoReplicationMixin());
                    UGorgeousObjectVariable* Variable=nullptr;
                    PC->GetAutoReplicationMixin().TryGetValue(TEXT("AuditPrivate"),Variable);
                    int32 Forged=999999;
                    IGorgeousSingleObjectVariablesSetter_I::Execute_SetIntegerSingleObjectVariable(Variable,NAME_None,Forged);
                    FGorgeousAutoReplicationConditionContext Context;
                    FGorgeousAutoReplicationPropertyEnvelope Envelope;
                    Envelope.EntryKey=TEXT("AuditPrivate"); Envelope.OwnerActor=PC; Envelope.VariableClass=UInteger_SOV::StaticClass();
                    Variable->BuildAutoReplicationPropertyPayload(Context,Envelope.Payload);
                    Relay->RelayPropertyPayloadToServer(Envelope,&PC->GetAutoReplicationMixin());
                }
                for (TActorIterator<AGorgeousNetworkAuditActor> It(Server);It;++It)
                    S.Test->TestTrue(TEXT("Unregistered combined stat tag dispatches parent signal with real target/stat/value"),It->bObservedStatParentSignal);
                auto& Coordinator=FGorgeousAutoReplicationCoordinator::Get(Server);
                const int32 BeforeTransition=Coordinator.GetActiveStreamCount();
                S.Test->TestTrue(TEXT("Authority actually registered active replication streams"),BeforeTransition>0);
                UGorgeousCoreRuntimeGlobals::SetAutoReplicationUseIrisOverride(Server,false);
                S.Test->TestEqual(TEXT("Backend transition preserves stream registrations"),Coordinator.GetActiveStreamCount(),BeforeTransition);
                UGorgeousCoreRuntimeGlobals::ClearAutoReplicationUseIrisOverride(Server);
                S.Test->TestEqual(TEXT("Clearing override preserves stream registrations"),Coordinator.GetActiveStreamCount(),BeforeTransition);
                auto* ServerPermissions=Server->GetGameInstance()->GetSubsystem<UGorgeousPermissionManagerSubsystem>();
                if (!ServerPermissions) { S.Test->AddError(TEXT("Server permission subsystem missing.")); Cleanup(); return false; }
                const FName AuditRole(TEXT("AuditServerRole"));
                ServerPermissions->CreateOrUpdateRole(AuditRole,{{TEXT("Audit.ServerOnly"),true}});
                for (UWorld* World:Clients)
                {
                    APlayerController* ClientPC=World->GetFirstPlayerController();
                    auto* ClientPermissions=World->GetGameInstance()->GetSubsystem<UGorgeousPermissionManagerSubsystem>();
                    if (!ClientPermissions) { S.Test->AddError(TEXT("Client permission subsystem missing.")); Cleanup(); return false; }
                    ClientPermissions->CreateOrUpdateRole(AuditRole,{{TEXT("Audit.ServerOnly"),true}});
                    ClientPermissions->GrantRole(ClientPC,AuditRole);
                    ClientPermissions->GrantPermission(ClientPC,TEXT("Audit.ServerOnly"),true);
                    bool Handled=false;
                    S.Test->TestFalse(TEXT("Client-local mutation cannot grant gameplay permission"),ClientPermissions->EvaluatePlayerPermission(ClientPC,TEXT("Audit.ServerOnly"),Handled));
                    S.Test->TestFalse(TEXT("Client-local permission tables remain unhandled"),Handled);
                    if (auto* Owned=Cast<AGorgeousPlayerController>(ClientPC))
                    {
                        Owned->bLastPermissionMutationAccepted=true;
                        Owned->Server_RequestGorgeousPermissionMutation(Owned->PlayerState,EGorgeousPermissionMutation::GrantRole,TEXT("AuditUnauthorized"),true);
                    }

                }
                for (FConstPlayerControllerIterator It=Server->GetPlayerControllerIterator();It;++It)
                {
                    APlayerController* PC=It->Get(); if (!PC || !PC->PlayerState) continue;
                    ServerPermissions->GrantRole(PC,AuditRole);
                    bool Handled=false;
                    S.Test->TestTrue(TEXT("Server role grants permission through controller identity"),ServerPermissions->EvaluatePlayerPermission(PC,TEXT("Audit.ServerOnly"),Handled));
                    S.Test->TestTrue(TEXT("Controller and PlayerState resolve the same permission identity"),ServerPermissions->EvaluatePlayerPermission(PC->PlayerState,TEXT("Audit.ServerOnly"),Handled));
                    ServerPermissions->GrantPermission(PC,TEXT("Audit.ServerOnly"),false);
                    S.Test->TestFalse(TEXT("Server explicit deny overrides assigned role"),ServerPermissions->EvaluatePlayerPermission(PC->PlayerState,TEXT("Audit.ServerOnly"),Handled));
                }
                UWorld* RequestWorld=Clients[0];
                auto* GS=RequestWorld->GetGameState<AGorgeousGameState>();
                auto StartRequest=[&](FName Handler,EGorgeousAutoReplicationRPCType Type,TStrongObjectPtr<UGorgeousAutoReplicationRPCRequestAsyncAction>& Action,TStrongObjectPtr<UGorgeousNetworkAuditCallbacks>& Callbacks) {
                    FGorgeousRPCPayload Payload; Payload.HandlerName=Handler; Payload.TimeoutSeconds=20;
                    Action.Reset(UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(GS,TEXT("AuditRPC"),Type,Payload,EGorgeousAutoReplicationTargetKind::EObjectVariable,GS));
                    Callbacks.Reset(NewObject<UGorgeousNetworkAuditCallbacks>());
                    Action->OnCompleted.AddDynamic(Callbacks.Get(),&UGorgeousNetworkAuditCallbacks::OnComplete);
                    Action->OnFailed.AddDynamic(Callbacks.Get(),&UGorgeousNetworkAuditCallbacks::OnFailure);
                    Action->Activate();
                };
                StartRequest(TEXT("AuditDeferred"),EGorgeousAutoReplicationRPCType::EReliableServer,S.DeferredAction,S.DeferredCallbacks);
                StartRequest(TEXT("AuditImmediate"),EGorgeousAutoReplicationRPCType::EReliableMulticast,S.MulticastAction,S.MulticastCallbacks);
                S.Phase=1; S.PhaseStarted=FPlatformTime::Seconds();
            }
            else if (Elapsed>90)
            {
                S.Test->AddError(TEXT("Initial network state did not converge: PC/PlayerState/GameState/WorldSettings routing or privacy failed."));
                S.Test->AddInfo(FString::Printf(TEXT("PIE server=%s begun=%d clients=%d"),*GetNameSafe(Server),Server && Server->HasBegunPlay(),Clients.Num()));
                for (UWorld* World:Clients)
                {
                    auto* PC=Cast<AGorgeousNetworkAuditController>(World->GetFirstPlayerController());
                    auto* PS=PC ? Cast<AGorgeousPlayerState>(PC->PlayerState) : nullptr;
                    auto* GS=World->GetGameState<AGorgeousGameState>();
                    auto* WS=Cast<AGorgeousWorldSettings>(World->GetWorldSettings());
                    const int32 Id=PS ? PS->GetPlayerId() : -1;
                    S.Test->AddInfo(FString::Printf(TEXT("Client %s begun=%d PC=%s PS=%s GS=%s WS=%s PCvalue=%d PSvalue=%d GSvalue=%d WSvalue=%d"),
                        *World->GetName(),World->HasBegunPlay(),*GetNameSafe(PC),*GetNameSafe(PS),*GetNameSafe(GS),*GetNameSafe(WS),
                        PC && ReadAuditInteger(PC->GetAutoReplicationMixin(),TEXT("AuditPrivate"),1000+Id),
                        PS && ReadAuditInteger(PS->GetAutoReplicationMixin(),TEXT("AuditPlayerState"),2000+Id),
                        GS && ReadAuditInteger(GS->GetAutoReplicationMixin(),TEXT("AuditShared"),701),
                        WS && ReadAuditInteger(WS->GetAutoReplicationMixin(),TEXT("AuditWorld"),801)));
                    if (PC && PS) S.Test->AddInfo(FString::Printf(TEXT("Private value actual=%d expected=%d"),AuditIntegerValue(PC->GetAutoReplicationMixin(),TEXT("AuditPrivate")),1000+PS->GetPlayerId()));
                    for (TActorIterator<AGorgeousWorldSettings> It(World);It;++It)
                        S.Test->AddInfo(FString::Printf(TEXT("WorldSettings candidate %s selected=%d WS801=%d StatStorage=%s"),*It->GetPathName(),*It==WS,ReadAuditInteger(It->GetAutoReplicationMixin(),TEXT("AuditWorld"),801),*GetNameSafe(FindAuditVariable(It->GetAutoReplicationMixin(),TEXT("StatStorage")))));
                    for (TActorIterator<AGorgeousNetworkAuditActor> It(World);It;++It)
                        S.Test->AddInfo(FString::Printf(TEXT("AuditActor %s begun=%d OV901=%d stat=%g storage=%s"),*It->GetName(),It->HasActorBegunPlay(),ReadAuditInteger(It->AutoReplication->GetAutoReplicationMixin(),TEXT("AuditActor"),901),It->Stats->GetStat(GetGorgeousNetworkAuditStatTag()),*GetNameSafe(It->Stats->GetStatStorage())));
                }
                Cleanup();
            }
        }
        else if (S.Phase==1 && Elapsed>3)
        {
            if (S.DeferredCallbacks->Completed!=1 || S.MulticastCallbacks->Completed!=1)
            {
                if (S.DeferredCallbacks->Failed || S.MulticastCallbacks->Failed || Elapsed>30) { S.Test->AddError(TEXT("Client-initiated deferred server/multicast result did not complete over actual relay.")); Cleanup(); }
                return false;
            }
            S.Test->TestTrue(TEXT("Client-initiated multicast collects server and both clients before completion"),S.MulticastAction->GetResolvedResults().Num()>=3);
            auto ReturnedInteger=[](UGorgeousAutoReplicationRPCRequestAsyncAction* Action) {
                UGorgeousObjectVariable* Variable=Action->GetResolvedTargetVariable();
                if (auto* Container=Cast<UGorgeousRPC_OV>(Variable)) Variable=Container->GetCachedTargetVariable();
                return Variable ? IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Variable,NAME_None) : -1;
            };
            S.Test->TestEqual(TEXT("Deferred server handler carries populated return OV back to client"),ReturnedInteger(S.DeferredAction.Get()),937);
            S.Test->TestEqual(TEXT("Multicast return carries handler value"),ReturnedInteger(S.MulticastAction.Get()),731);
            if (!Server) { S.Test->AddError(TEXT("Server world disappeared during hostile request test.")); Cleanup(); return false; }
            for (FConstPlayerControllerIterator It=Server->GetPlayerControllerIterator();It;++It)
            {
                if (auto* PC=Cast<AGorgeousNetworkAuditController>(It->Get()))
                {
                    S.Test->TestEqual(TEXT("Unregistered client UFUNCTION never executed on server"),PC->UnregisteredWitness,0);
                    S.Test->TestTrue(TEXT("Unapproved client property write did not alter authoritative player value"),ReadAuditInteger(PC->GetAutoReplicationMixin(),TEXT("AuditPrivate"),1000+PC->PlayerState->GetPlayerId()));
                }
            }
            for (UWorld* World:Clients)
            {
                auto* PC=Cast<AGorgeousPlayerController>(World->GetFirstPlayerController());
                auto* Permissions=World->GetGameInstance()->GetSubsystem<UGorgeousPermissionManagerSubsystem>();
                S.Test->TestTrue(TEXT("Owner client receives its authoritative assigned role"),Permissions && Permissions->HasRole(PC,TEXT("AuditServerRole")));
                bool Handled=false;
                S.Test->TestFalse(TEXT("Owner snapshot includes authoritative explicit deny"),Permissions && Permissions->EvaluatePlayerPermission(PC,TEXT("Audit.ServerOnly"),Handled));
                S.Test->TestTrue(TEXT("Received deny is handled rather than missing local state"),Handled);
                S.Test->TestFalse(TEXT("Actual owning-controller admin mutation RPC defaults denied"),PC->bLastPermissionMutationAccepted);
            }
            auto* ServerPermissions=Server->GetGameInstance()->GetSubsystem<UGorgeousPermissionManagerSubsystem>();
            for (FConstPlayerControllerIterator It=Server->GetPlayerControllerIterator();It;++It)
                S.Test->TestFalse(TEXT("Forbidden edit did not grant role on authority"),ServerPermissions->HasRole(It->Get(),TEXT("AuditUnauthorized")));
            S.Test->TestTrue(TEXT("Rejected writes were corrected on each client"),Ready(2));
            GEditor->RequestLateJoin();
            S.Phase=2; S.PhaseStarted=FPlatformTime::Seconds();
        }
        else if (S.Phase==2)
        {
            if (Ready(3))
            {
                S.Test->AddInfo(TEXT("Late client received unchanged GameState/WorldSettings snapshots and correctly isolated PC/PlayerState data."));
                Cleanup();
            }
            else if (Elapsed>90)
            {
                S.Test->AddError(TEXT("Late-join client did not receive unchanged authoritative state."));
                S.Test->AddInfo(FString::Printf(TEXT("Late join server=%s clients=%d"),*GetNameSafe(Server),Clients.Num()));
                for (UWorld* World:Clients)
                {
                    auto* PC=Cast<AGorgeousNetworkAuditController>(World->GetFirstPlayerController());
                    auto* PS=PC ? Cast<AGorgeousPlayerState>(PC->PlayerState) : nullptr;
                    auto* GS=World->GetGameState<AGorgeousGameState>(); auto* WS=Cast<AGorgeousWorldSettings>(World->GetWorldSettings());
                    const int32 Id=PS ? PS->GetPlayerId() : -1;
                    S.Test->AddInfo(FString::Printf(TEXT("Late client %s PC=%s PS=%s GS=%s WS=%s PCvalue=%d PSvalue=%d GSvalue=%d WSvalue=%d"),*World->GetPathName(),*GetNameSafe(PC),*GetNameSafe(PS),*GetNameSafe(GS),*GetNameSafe(WS),PC && ReadAuditInteger(PC->GetAutoReplicationMixin(),TEXT("AuditPrivate"),1000+Id),PS && ReadAuditInteger(PS->GetAutoReplicationMixin(),TEXT("AuditPlayerState"),2000+Id),GS && ReadAuditInteger(GS->GetAutoReplicationMixin(),TEXT("AuditShared"),701),WS && ReadAuditInteger(WS->GetAutoReplicationMixin(),TEXT("AuditWorld"),801)));
                    if (PC && PS) S.Test->AddInfo(FString::Printf(TEXT("Late private actual=%d expected=%d"),AuditIntegerValue(PC->GetAutoReplicationMixin(),TEXT("AuditPrivate")),1000+PS->GetPlayerId()));
                    for (TActorIterator<AGorgeousNetworkAuditActor> It(World);It;++It)
                        S.Test->AddInfo(FString::Printf(TEXT("Late actor %s OV901=%d stat=%g storage=%s"),*It->GetPathName(),ReadAuditInteger(It->AutoReplication->GetAutoReplicationMixin(),TEXT("AuditActor"),901),It->Stats->GetStat(GetGorgeousNetworkAuditStatTag()),*GetNameSafe(It->Stats->GetStatStorage())));
                }
                Cleanup();
            }
        }
        else if (S.Phase==4 && Elapsed>2)
        {
            for (UWorld* World:Clients)
            {
                const bool bSelected=World->GetFirstPlayerController()->PlayerState->GetPlayerId()==S.ChannelRecipientId;
                auto* GS=World->GetGameState<AGorgeousGameState>();
                if (bSelected) S.Test->TestTrue(TEXT("Explicitly subscribed client receives the named-channel update"),GS && ReadAuditInteger(GS->GetAutoReplicationMixin(),TEXT("AuditSubscribedChannel"),778));
                else S.Test->TestFalse(TEXT("Other policy-allowed clients remain excluded from the named channel"),GS && ReadAuditInteger(GS->GetAutoReplicationMixin(),TEXT("AuditSubscribedChannel"),778));
            }
            auto* Stack=UGorgeousRootNetworkStackSubsystem::Get(Server);
            if (!Stack || !S.ChannelRecipient.IsValid()) { S.Test->AddError(TEXT("Channel owner disappeared before unsubscribe.")); Cleanup(); return false; }
            Stack->UnsubscribeControllerFromChannel(S.ChannelRecipient.Get(),TEXT("Audit.RestrictedChannel"));
            S.Test->TestFalse(TEXT("Explicit unsubscribe immediately removes membership"),Stack->IsControllerSubscribedToChannel(S.ChannelRecipient.Get(),TEXT("Audit.RestrictedChannel")));
            SetAuditInteger(Server->GetGameState<AGorgeousGameState>(),TEXT("AuditSubscribedChannel"),779);
            S.Phase=5; S.PhaseStarted=FPlatformTime::Seconds();
        }
        else if (S.Phase==5 && Elapsed>2)
        {
            auto* Stack=UGorgeousRootNetworkStackSubsystem::Get(Server);
            S.Test->TestFalse(TEXT("A send never silently recreates an explicitly removed named-channel subscription"),Stack && Stack->IsControllerSubscribedToChannel(S.ChannelRecipient.Get(),TEXT("Audit.RestrictedChannel")));
            for (UWorld* World:Clients)
            {
                auto* GS=World->GetGameState<AGorgeousGameState>();
                S.Test->TestFalse(TEXT("Unsubscribed and never-subscribed clients do not receive later named-channel updates"),GS && ReadAuditInteger(GS->GetAutoReplicationMixin(),TEXT("AuditSubscribedChannel"),779));
            }
            S.Test->TestTrue(TEXT("Default-channel actor/stat/framework delivery remains intact while named channel is restricted"),Ready(2));
            Cleanup();
        }
        else if (S.Phase==10)
        {
            if (!S.VDGServerController.IsValid() || !S.VDGClient.IsValid() || !S.VDGManager.IsValid()) { S.Test->AddError(TEXT("VDG fixture disappeared")); Cleanup(); return false; }
            auto* PS=Cast<AGorgeousPlayerState>(S.VDGClient->PlayerState);
            if (!S.VDGServerController->HasVisualDataGatheringConsent() || !PS || PS->GetGorgeousStablePlayerId_Implementation()!=S.VDGStableId)
            { if (Elapsed>10) { S.Test->AddError(TEXT("Explicit consent or stable identity did not reach authority/client")); Cleanup(); } return false; }
            FGorgeousRPCPayload Payload;
            if (!MakeVDGAuditPayload(S.VDGManager.Get(),S.VDGStableId,337,Payload)) { S.Test->AddError(TEXT("Cannot construct accepted own VDG payload")); Cleanup(); return false; }
            auto* PC=S.VDGClient.Get();
            S.DeferredAction.Reset(UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(PC,TEXT("RequestVDGPropertyWrite"),EGorgeousAutoReplicationRPCType::EReliableServer,Payload,EGorgeousAutoReplicationTargetKind::EActorComponent,PC->GetWorld()->GetGameState<AGorgeousGameState>()));
            S.DeferredCallbacks.Reset(NewObject<UGorgeousNetworkAuditCallbacks>());
            S.DeferredAction->OnCompleted.AddDynamic(S.DeferredCallbacks.Get(),&UGorgeousNetworkAuditCallbacks::OnComplete);
            S.DeferredAction->OnFailed.AddDynamic(S.DeferredCallbacks.Get(),&UGorgeousNetworkAuditCallbacks::OnFailure);
            S.DeferredAction->Activate(); S.Phase=11; S.PhaseStarted=FPlatformTime::Seconds();
        }
        else if (S.Phase==11)
        {
            if (!S.DeferredCallbacks->Completed)
            { if (S.DeferredCallbacks->Failed || Elapsed>10) { S.Test->AddError(TEXT("Actual accepted own VDG Blueprint receiver did not complete over relay")); Cleanup(); } return false; }
            const auto& Resolved=S.DeferredAction->GetResolvedResult();
            S.Test->TestTrue(TEXT("Accepted request result identifies an actor-component receiver"),Resolved.TargetKind==EGorgeousAutoReplicationTargetKind::EActorComponent);
            S.Test->AddInfo(FString::Printf(TEXT("VDG accepted result kind=%d owner=%s results=%d"),static_cast<int32>(Resolved.TargetKind),*GetNameSafe(Resolved.TargetOwner),S.DeferredAction->GetResolvedResults().Num()));
            for (FName StoreName:{FName(TEXT("MemberProperties")),FName(TEXT("GlobalProperties"))})
            {
                auto* Store=FindFProperty<FMapProperty>(S.VDGManager->GetClass(),StoreName);
                if (!Store) { S.Test->AddError(TEXT("Actual Manager is missing the documented property store")); continue; }
                FScriptMapHelper Map(Store,Store->ContainerPtrToValuePtr<void>(S.VDGManager.Get()));
                S.Test->AddInfo(FString::Printf(TEXT("VDG actual store %s has %d entries"),*StoreName.ToString(),Map.Num()));
                auto* KeyProperty=CastField<FNameProperty>(Store->KeyProp);
                auto* ValueProperty=CastField<FObjectProperty>(Store->ValueProp);
                for (int32 Index=0,Observed=0;Index<Map.GetMaxIndex() && Observed<32;++Index)
                    if (Map.IsValidIndex(Index) && KeyProperty && ValueProperty)
                    {
                        ++Observed;
                        auto* Stored=Cast<UGorgeousObjectVariable>(ValueProperty->GetObjectPropertyValue(Map.GetValuePtr(Index)));
                        const int32 Number=Stored && Stored->IsA<UInteger_SOV>() ? IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Stored,NAME_None) : MIN_int32;
                        S.Test->AddInfo(FString::Printf(TEXT("VDG actual stored key=%s class=%s integer=%d"),*KeyProperty->GetPropertyValue(Map.GetKeyPtr(Index)).ToString(),Stored ? *Stored->GetClass()->GetName() : TEXT("null"),Number));
                    }
            }
            UGorgeousObjectVariable* Answer=ReadVDGAuditAnswer(S.VDGManager.Get(),S.VDGStableId);
            S.Test->TestTrue(TEXT("Actual bundled VDG handler stored the accepted own answer"),Answer && Answer->IsA<UInteger_SOV>() && IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Answer,NAME_None)==337);
            auto* PC=S.VDGClient.Get();
            FGorgeousRPCPayload Foreign;
            if (!MakeVDGAuditPayload(S.VDGManager.Get(),TEXT("VDG-Audit-Other"),999,Foreign)) { S.Test->AddError(TEXT("Cannot construct foreign-identity control")); Cleanup(); return false; }
            S.MulticastAction.Reset(UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(PC,TEXT("RequestVDGPropertyWrite"),EGorgeousAutoReplicationRPCType::EReliableServer,Foreign,EGorgeousAutoReplicationTargetKind::EActorComponent,PC->GetWorld()->GetGameState<AGorgeousGameState>()));
            S.MulticastCallbacks.Reset(NewObject<UGorgeousNetworkAuditCallbacks>());
            S.MulticastAction->OnCompleted.AddDynamic(S.MulticastCallbacks.Get(),&UGorgeousNetworkAuditCallbacks::OnComplete);
            S.MulticastAction->OnFailed.AddDynamic(S.MulticastCallbacks.Get(),&UGorgeousNetworkAuditCallbacks::OnFailure);
            S.MulticastAction->Activate(); S.Phase=12; S.PhaseStarted=FPlatformTime::Seconds();
        }
        else if (S.Phase==12)
        {
            if (!S.MulticastCallbacks->Failed && Elapsed<8) return false;
            S.Test->TestEqual(TEXT("Consent never accepts a foreign-player VDG write"),S.MulticastCallbacks->Completed,0);
            S.Test->TestTrue(TEXT("Foreign write is explicitly rejected/failed"),S.MulticastCallbacks->Failed>0);
            UGorgeousObjectVariable* Own=ReadVDGAuditAnswer(S.VDGManager.Get(),S.VDGStableId);
            S.Test->TestTrue(TEXT("Rejected foreign request preserves the existing answer"),Own && IGorgeousSingleObjectVariablesGetter_I::Execute_GetIntegerSingleObjectVariable(Own,NAME_None)==337);
            S.VDGClient->SetVisualDataGatheringConsent(false); S.Phase=13; S.PhaseStarted=FPlatformTime::Seconds();
        }
        else if (S.Phase==13)
        {
            if (S.VDGServerController->HasVisualDataGatheringConsent())
            { if (Elapsed>10) { S.Test->AddError(TEXT("Revocation did not reach authority")); Cleanup(); } return false; }
            S.Test->TestFalse(TEXT("Revoked consent immediately stops subsequent collection requests"),FGorgeousVDGConsent::CanSendCollectionRequest(S.VDGClient.Get()));
            S.Test->AddInfo(TEXT("Actual VDG Blueprint own-player write completed; default-off/foreign-player/revocation boundaries checked over connected PIE worlds."));
            Cleanup();
        }
        else if (S.Phase==3)
        {
            if (!Server && Clients.IsEmpty())
            {
                GEngine->WorldSettingsClass=S.PreviousWorldSettingsClass;
                S.DeferredAction.Reset(); S.MulticastAction.Reset(); S.DeferredCallbacks.Reset(); S.MulticastCallbacks.Reset();
                S.Settings.Reset(); return true;
            }
            if (Elapsed>30) { S.Test->AddError(TEXT("PIE cleanup timed out.")); GEditor->EndPlayMap(); GEngine->WorldSettingsClass=S.PreviousWorldSettingsClass; return true; }
        }
        return false;
    }
};
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FGorgeousNetworkE2E,"GorgeousCore.LimitationAudit.NetworkE2E",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FGorgeousNetworkE2E::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{ Names.Add(TEXT("ListenServer")); Commands.Add(TEXT("Listen")); Names.Add(TEXT("DedicatedServer")); Commands.Add(TEXT("Dedicated")); Names.Add(TEXT("ChannelSubscriptions")); Commands.Add(TEXT("DedicatedChannels")); Names.Add(TEXT("VDGConsent")); Commands.Add(TEXT("DedicatedVDGConsent")); }
bool FGorgeousNetworkE2E::RunTest(const FString& Parameters)
{
    if (!GEditor || GEditor->PlayWorld || !FParse::Param(FCommandLine::Get(),TEXT("GorgeousAllowAuditPIE")))
    { AddError(TEXT("Run in an isolated fresh project with -GorgeousAllowAuditPIE; this test creates a blank editor map and owns its PIE session.")); return false; }
    auto State=MakeShared<FGorgeousNetworkAuditState>(); State->Test=this; State->bDedicated=Parameters.StartsWith(TEXT("Dedicated")); State->bChannelSubscriptionTest=Parameters.Contains(TEXT("Channels"));
    State->bVDGConsentTest=Parameters.Contains(TEXT("VDGConsent"));
    // A level-owned WorldSettings must have a saved package identity for native
    // network reference mapping. An unsaved /Temp map produces unsupported outer
    // NetGUIDs in PIE and is not a valid cross-client WorldSettings fixture.
    FString AuditMap;
    if (!FParse::Value(FCommandLine::Get(),TEXT("GorgeousAuditNetworkMap="),AuditMap) ||
        !FPackageName::IsValidLongPackageName(AuditMap) || !FPackageName::DoesPackageExist(AuditMap))
    {
        AddError(TEXT("Provide -GorgeousAuditNetworkMap=/Game/Path/To/SavedAuditMap with native GorgeousWorldSettings; the isolated test loads it read-only and never saves assets."));
        return false;
    }
    State->PreviousWorldSettingsClass=GEngine->WorldSettingsClass;
    GEngine->WorldSettingsClass=AGorgeousWorldSettings::StaticClass();
    const FString MapFilename=FPackageName::LongPackageNameToFilename(AuditMap,FPackageName::GetMapPackageExtension());
    if (!FEditorFileUtils::LoadMap(MapFilename,false,false) || !GEditor->GetEditorWorldContext().World() ||
        !Cast<AGorgeousWorldSettings>(GEditor->GetEditorWorldContext().World()->GetWorldSettings()))
    {
        GEngine->WorldSettingsClass=State->PreviousWorldSettingsClass;
        AddError(TEXT("Saved audit map failed to load with native GorgeousWorldSettings."));
        return false;
    }
    State->Settings.Reset(DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage()));
    State->Settings->SetPlayNetMode(State->bDedicated ? PIE_Client : PIE_ListenServer);
    State->Settings->SetRunUnderOneProcess(true); State->Settings->SetPlayNumberOfClients(State->bDedicated ? 2 : 3);
    State->Settings->SetServerPort(State->bDedicated ? 19872 : 19871); State->Settings->bAutoCompileBlueprintsOnLaunch=false;
    FRequestPlaySessionParams Request; Request.EditorPlaySettings=State->Settings.Get(); Request.GameModeOverride=AGorgeousNetworkAuditGameMode::StaticClass(); Request.bAllowOnlineSubsystem=false;
    State->PhaseStarted=FPlatformTime::Seconds(); GEditor->RequestPlaySession(Request);
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FGorgeousNetworkAuditLatent>(State));
    return true;
}
#endif
