#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/ScopeExit.h"
#include "Editor.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameMapsSettings.h"
#include "GameFramework/GameModeBase.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "AutoReplication/GorgeousAutoReplicationGraph.h"
#include "IpNetDriver.h"
#include "Engine/PackageMapClient.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClosureLiveReplicationGraphTest,
    "GorgeousClosure.CoreEditor.LiveReplicationGraphRegistration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FClosureLiveReplicationGraphTest::RunTest(const FString&)
{
    UWorld* World=UWorld::CreateWorld(EWorldType::GamePreview,false,FName(TEXT("ClosureGraphWorld")));
    if (!TestNotNull(TEXT("Owned graph world"),World)) return false;
    TStrongObjectPtr<UIpNetDriver> Driver(NewObject<UIpNetDriver>(World));
    Driver->SetWorld(World);
    Driver->GetNetGuidCache()=MakeShared<FNetGUIDCache>(Driver.Get());
    TStrongObjectPtr<UGorgeousAutoReplicationGraph> Graph(NewObject<UGorgeousAutoReplicationGraph>(Driver.Get()));
    Graph->InitForNetDriver(Driver.Get());
    ON_SCOPE_EXIT {
        Graph->TearDown();
        UGorgeousRootObjectVariable::PurgeWorldOwnedRegistryEntries(World,true);
        Driver->SetWorld(nullptr);
        World->DestroyWorld(false);
    };
    auto* Owner=World->SpawnActor<AGorgeousGameState>();
    if (!TestNotNull(TEXT("Actual Gorgeous replication owner"),Owner)) return false;
    const FName KeyA(TEXT("ClosureGraphA")),KeyB(TEXT("ClosureGraphB"));
    TestTrue(TEXT("First actual mixin stream registers"),Owner->RegisterAutoReplicationEntry(KeyA,UInteger_SOV::StaticClass(),true,false,FGorgeousAutoReplicationStreamConfig()));
    TestTrue(TEXT("Second actual mixin stream registers"),Owner->RegisterAutoReplicationEntry(KeyB,UInteger_SOV::StaticClass(),true,false,FGorgeousAutoReplicationStreamConfig()));
    UGorgeousObjectVariable* A=nullptr;UGorgeousObjectVariable* B=nullptr;
    Owner->GetAutoReplicationMixin().TryGetValue(KeyA,A);Owner->GetAutoReplicationMixin().TryGetValue(KeyB,B);
    if (!TestNotNull(TEXT("First actual variable"),A) || !TestNotNull(TEXT("Second actual variable"),B)) return false;
    TestEqual(TEXT("First stream binds actual owner"),A->GetAutoReplicationOwner(),static_cast<UObject*>(Owner));
    TestEqual(TEXT("Second stream binds actual owner"),B->GetAutoReplicationOwner(),static_cast<UObject*>(Owner));
    auto* NodeProperty=FindFProperty<FObjectPropertyBase>(Graph->GetClass(),TEXT("AutoReplicationNode"));
    auto* Node=NodeProperty ? Cast<UReplicationGraphNode_ActorList>(NodeProperty->GetObjectPropertyValue_InContainer(Graph.Get())) : nullptr;
    if (!TestNotNull(TEXT("Actual always-relevant graph node"),Node)) return false;
    auto Count=[&]() {
        TArray<FActorRepListType> Actors;Node->GetAllActorsInNode_Debugging(Actors);
        return Actors.FilterByPredicate([&](const FActorRepListType& Actor) { return static_cast<AActor*>(Actor)==Owner; }).Num();
    };
    Graph->RegisterObjectVariable(A);Graph->RegisterObjectVariable(A);
    TestEqual(TEXT("Repeated same-variable register creates one owner list entry"),Count(),1);
    Graph->RegisterObjectVariable(B);
    TestEqual(TEXT("Two streams share one actual owner list entry"),Count(),1);
    Graph->UnregisterObjectVariable(A);
    TestEqual(TEXT("First stream removal preserves second stream's owner"),Count(),1);
    Graph->UnregisterObjectVariable(A);
    TestEqual(TEXT("Repeated removal cannot consume second stream registration"),Count(),1);
    Graph->UnregisterObjectVariable(B);
    TestEqual(TEXT("Last variable removal leaves no always-relevant owner"),Count(),0);
    Graph->RegisterObjectVariable(A);
    TestEqual(TEXT("Clean re-registration works"),Count(),1);
    Graph->UnregisterObjectVariable(A);
    TestEqual(TEXT("Clean final removal leaves no ghost owner"),Count(),0);
    return true;
}

namespace ClosurePlainPIE
{
struct FState
{
    FAutomationTestBase* Test=nullptr;
    TStrongObjectPtr<ULevelEditorPlaySettings> Settings;
    FSoftClassPath PreviousGameInstance;
    int32 Phase=0,Cycle=0;
    double PhaseStart=0;
    bool bRestored=false;
    void Restore() {
        if (!bRestored) { GetMutableDefault<UGameMapsSettings>()->GameInstanceClass=PreviousGameInstance;bRestored=true; }
    }
    ~FState() { Restore(); }
    bool CheckDefaultParent(const TCHAR* Where) {
        UGorgeousRootObjectVariable* Root=UGorgeousRootObjectVariable::GetRootObjectVariable();
        if (!Test->TestNotNull(FString::Printf(TEXT("%s root"),Where),Root)) return false;
        FGuid Identifier;
        UGorgeousObjectVariable* Variable=UGorgeousObjectVariable::CreateObjectVariable(UInteger_SOV::StaticClass(),Identifier,nullptr,false,TEXT("ClosureDefaultParent"));
        if (!Test->TestNotNull(FString::Printf(TEXT("%s parentless factory"),Where),Variable)) return false;
        Test->TestTrue(TEXT("Factory result has real registered identifier"),Identifier.IsValid());
        Test->TestEqual(TEXT("Factory resolves actual default root parent"),Variable->GetParent(),static_cast<UGorgeousObjectVariable*>(Root));
        Test->TestEqual(TEXT("Default root registry resolves created identity"),UGorgeousRootObjectVariable::FindVariableByIdentifier(Identifier),Variable);
        UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Variable);
        return true;
    }
    void Start() {
        FRequestPlaySessionParams Request;Request.EditorPlaySettings=Settings.Get();Request.GameModeOverride=AGameModeBase::StaticClass();Request.bAllowOnlineSubsystem=false;
        GEditor->RequestPlaySession(Request);Phase=1;PhaseStart=FPlatformTime::Seconds();
    }
};
class FPoll final : public IAutomationLatentCommand
{
public:
    explicit FPoll(TSharedRef<FState> InState):State(InState) {}
    bool Update() override {
        if (!GEditor) { State->Test->AddError(TEXT("Editor disappeared during owned PIE"));State->Restore();return true; }
        const bool bTimedOut=FPlatformTime::Seconds()-State->PhaseStart>30.0;
        if (bTimedOut && State->Phase==3) { State->Test->AddError(TEXT("Owned PIE did not end after cancellation"));State->Restore();return true; }
        if (bTimedOut && State->Phase!=3) {
            State->Test->AddError(TEXT("Timed out waiting for actual plain-GameInstance PIE lifecycle"));
            GEditor->RequestEndPlayMap();State->Phase=3;State->PhaseStart=FPlatformTime::Seconds();return false;
        }
        if (State->Phase==1) {
            UWorld* World=GEditor->PlayWorld;
            if (!World || !World->HasBegunPlay() || !World->GetGameInstance()) return false;
            State->Test->TestEqual(TEXT("PIE uses plain engine GameInstance, no Gorgeous override"),World->GetGameInstance()->GetClass(),UGameInstance::StaticClass());
            State->CheckDefaultParent(State->Cycle==0 ? TEXT("First plain PIE") : TEXT("Second plain PIE"));
            GEditor->RequestEndPlayMap();State->Phase=2;State->PhaseStart=FPlatformTime::Seconds();return false;
        }
        bool bPIEContext=false;
        for (const FWorldContext& Context:GEngine->GetWorldContexts()) if (Context.WorldType==EWorldType::PIE) bPIEContext=true;
        if (GEditor->PlayWorld || bPIEContext) return false;
        if (State->Phase==3) { State->Restore();return true; }
        // Let the real teardown ticker restore root creation. Do not manually
        // unblock it or synthesize BeginPIE/EndPIE delegate broadcasts.
        if (!UGorgeousRootObjectVariable::IsRootCreationAllowed()) return false;
        State->CheckDefaultParent(State->Cycle==0 ? TEXT("Editor after first PIE") : TEXT("Editor after second PIE"));
        if (++State->Cycle<2) { State->Start();return false; }
        State->Restore();return true;
    }
private:TSharedRef<FState> State;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClosurePlainGameInstancePIETest,
    "GorgeousClosure.CoreEditor.PlainGameInstanceRepeatedPIE",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FClosurePlainGameInstancePIETest::RunTest(const FString&)
{
    if (!GEditor || GEditor->PlayWorld || !FParse::Param(FCommandLine::Get(),TEXT("GorgeousAllowAuditPIE"))) {
        AddError(TEXT("Requires isolated project -GorgeousAllowAuditPIE with no existing PIE session."));return false;
    }
    auto State=MakeShared<ClosurePlainPIE::FState>();State->Test=this;
    State->PreviousGameInstance=GetMutableDefault<UGameMapsSettings>()->GameInstanceClass;
    GetMutableDefault<UGameMapsSettings>()->GameInstanceClass=FSoftClassPath(UGameInstance::StaticClass());
    State->Settings.Reset(DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage()));
    State->Settings->SetPlayNetMode(PIE_Standalone);State->Settings->SetRunUnderOneProcess(true);State->Settings->SetPlayNumberOfClients(1);State->Settings->bAutoCompileBlueprintsOnLaunch=false;
    if (!FAutomationEditorCommonUtils::CreateNewMap()) { AddError(TEXT("Owned blank map creation failed"));State->Restore();return false; }
    State->CheckDefaultParent(TEXT("Editor before first PIE"));State->Start();
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<ClosurePlainPIE::FPoll>(State));
    return true;
}
#endif
