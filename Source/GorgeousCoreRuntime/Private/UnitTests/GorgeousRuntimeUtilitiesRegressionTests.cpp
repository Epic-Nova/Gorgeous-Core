// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Helpers/GorgeousFileHelper.h"
#include "Helpers/GorgeousLoggingHelper.h"
#include "Libraries/GorgeousLoggingBlueprintFunctionLibrary.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "Templates/GorgeousSingleton.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "IGorgeousThingsModuleInterface.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "Containers/Ticker.h"
#include "ModuleCore/GorgeousInGameLogging.h"
#include "Helpers/GorgeousLicenseHelper.h"
#include "Misc/ScopeExit.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousFileHelperRegressionTest, "GorgeousCore.RuntimeUtilities.FileMutations",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousFileHelperRegressionTest::RunTest(const FString&)
{
    const FString Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), FGuid::NewGuid().ToString());
    IFileManager::Get().MakeDirectory(*Directory, true);
    FString RelativeDirectory = FPaths::ConvertRelativePathToFull(Directory);
    const FString Base = GetGorgeousPluginBaseDir() + TEXT("/");
    FPaths::MakePathRelativeTo(RelativeDirectory, *Base);
    const FString Source = RelativeDirectory / TEXT("source.txt");
    const FString Copy = RelativeDirectory / TEXT("copy.txt");
    const FString Moved = RelativeDirectory / TEXT("moved.txt");
    TestTrue(TEXT("Write source"), WriteGorgeousFile(Source, TEXT("source"), true));
    TestTrue(TEXT("Write existing destination"), WriteGorgeousFile(Copy, TEXT("destination"), true));
    CopyGorgeousFile(Source, Copy, true);
    FString Content;
    TestTrue(TEXT("Read destination"), ReadGorgeousFile(Copy, Content, false));
    TestEqual(TEXT("Copy uses source content"), Content, FString(TEXT("source")));
    ReadGorgeousFile(Source, Content, false);
    TestEqual(TEXT("Copy preserves source"), Content, FString(TEXT("source")));
    TestTrue(TEXT("Read closes file: move immediately after reading"), MoveOrRenameGorgeousFile(Copy, Moved, false));
    TestFalse(TEXT("Move removes source path"), IsGorgeousFilePresent(Copy));
    TestTrue(TEXT("Append returns success"), InsertInGorgeousFile(Moved, TEXT("-tail")));
    TestTrue(TEXT("Replace returns success"), ReplaceInGorgeousFile(Moved, TEXT("source"), TEXT("updated")));
    TestTrue(TEXT("Delete text returns success"), DeleteInGorgeousFile(Moved, TEXT("-tail")));
    ReadGorgeousFile(Moved, Content, false);
    TestEqual(TEXT("Mutation chain content"), Content, FString(TEXT("updated")));
    TestFalse(TEXT("Missing-file mutation fails"), InsertInGorgeousFile(Copy, TEXT("missing")));
    DeleteGorgeousFile(Source); DeleteGorgeousFile(Moved);
    IFileManager::Get().DeleteDirectory(*Directory, false, false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousLoggingSuppressionRegressionTest, "GorgeousCore.RuntimeUtilities.CrossModuleSuppression",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousLoggingSuppressionRegressionTest::RunTest(const FString&)
{
    const FName Key(TEXT("Gorgeous.Audit.Suppression"));
    UGorgeousLoggingBlueprintFunctionLibrary::SetLoggingKeySuppressed(Key, true);
    TestTrue(TEXT("Runtime DLL sees suppression set by Utilities DLL"), GorgeousLogging::IsLoggingKeySuppressed(Key));
    GorgeousLogging::SetLoggingKeySuppressed(Key, false);
    TestFalse(TEXT("False lifts suppression in the Utilities DLL too"), UGorgeousLoggingBlueprintFunctionLibrary::IsLoggingKeySuppressed(Key));
    int32 Evaluations = 0;
    GT_E_LOG_ENSURE(++Evaluations == 1, TEXT("Gorgeous.Audit.Condition"), TEXT("Condition must be evaluated once"));
    TestEqual(TEXT("Assertion macro evaluates condition once"), Evaluations, 1);
    bool bOnly58 = false;
    GORGEOUS_58_ONLY(bOnly58 = true;)
    TestEqual(TEXT("ONLY matches exact engine version"), bOnly58, GORGEOUS_ENGINE_VERSION == 508);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousSingletonLifecycleRegressionTest, "GorgeousCore.RuntimeUtilities.SingletonLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousSingletonLifecycleRegressionTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    TestNull(TEXT("Abstract base cannot be a singleton instance"), UGorgeousSingleton::GetSingleton<UObject>(World, EGorgeousSingletonScope::World));
    USceneComponent* First = UGorgeousSingleton::GetSingleton<USceneComponent>(World, EGorgeousSingletonScope::World);
    TestNotNull(TEXT("Scoped singleton created"), First);
    TestEqual(TEXT("Same live world reuses singleton"), UGorgeousSingleton::GetSingleton<USceneComponent>(World, EGorgeousSingletonScope::World), First);
    FWorldDelegates::OnWorldCleanup.Broadcast(World, true, true);
    USceneComponent* Second = UGorgeousSingleton::GetSingleton<USceneComponent>(World, EGorgeousSingletonScope::World);
    TestTrue(TEXT("World cleanup releases previous singleton"), Second && Second != First);
    TestNull(TEXT("World scope requires a world"), UGorgeousSingleton::GetSingleton<USceneComponent>(nullptr, EGorgeousSingletonScope::World));
    UGameInstance* Instance = NewObject<UGameInstance>();
    World->SetGameInstance(Instance);
    USceneComponent* InstanceFirst = UGorgeousSingleton::GetSingleton<USceneComponent>(World, EGorgeousSingletonScope::GameInstance);
    FWorldDelegates::OnWorldCleanup.Broadcast(World, false, true);
    TestEqual(TEXT("Map cleanup preserves game-instance singleton"), UGorgeousSingleton::GetSingleton<USceneComponent>(World, EGorgeousSingletonScope::GameInstance), InstanceFirst);
    FWorldDelegates::OnWorldCleanup.Broadcast(World, true, true);
    USceneComponent* InstanceSecond = UGorgeousSingleton::GetSingleton<USceneComponent>(World, EGorgeousSingletonScope::GameInstance);
    TestTrue(TEXT("Session cleanup releases game-instance singleton"), InstanceSecond && InstanceSecond != InstanceFirst);
    UGorgeousSingleton::DestroySingleton<USceneComponent>(World, EGorgeousSingletonScope::GameInstance);
    UGorgeousSingleton::DestroySingleton<USceneComponent>(World, EGorgeousSingletonScope::World);
    World->DestroyWorld(false);
    return true;
}

namespace
{
    class FAuditModule final : public IGorgeousThingsModuleInterface
    {
    public:
        FName Name;
        TArray<FName> Dependencies;
        bool bCore = false;
        EGorgeousModuleFunctionality Functionality = EGorgeousModuleFunctionality::RuntimeUtilities;
        int32 Starts = 0;
        int32 Stops = 0;
        explicit FAuditModule(FName InName) : Name(InName) {}
        virtual void GorgeousStartupModule() override { ++Starts; }
        virtual void GorgeousShutdownModule() override { ++Stops; }
        virtual FName GetBelongingPluginName() const override { return Name; }
        virtual EGorgeousModuleFunctionality GetModuleFunctionality() const override { return Functionality; }
        virtual TArray<FName> GetDependentPlugins() const override { return Dependencies; }
        virtual int32 GetMinimumRequiredCoreVersion() const override { return 100; }
        virtual bool ProvidesCoreFunctionality() const override { return bCore; }
    };
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousModuleLifecycleRegressionTest, "GorgeousCore.RuntimeUtilities.ModuleStartupOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousModuleLifecycleRegressionTest::RunTest(const FString&)
{
    FAuditModule Started(TEXT("GorgeousAuditStarted"));
    Started.CompleteGorgeousStartup(); Started.CompleteGorgeousStartup();
    TestEqual(TEXT("Repeated completion starts once"), Started.Starts, 1);
    IModuleInterface* Module = &Started;
    Module->ShutdownModule();
    TestEqual(TEXT("Started module shuts down once"), Started.Stops, 1);
    FAuditModule NeverStarted(TEXT("GorgeousAuditNeverStarted"));
    Module = &NeverStarted;
    Module->ShutdownModule();
    TestEqual(TEXT("Unstarted module never receives shutdown"), NeverStarted.Stops, 0);
    TStrongObjectPtr<UGorgeousPluginHelper> Helper(NewObject<UGorgeousPluginHelper>());
    FAuditModule Deferred(TEXT("GorgeousAuditDeferred")); Deferred.bCore = true;
    Helper->DefferModuleRegistration(&Deferred);
    FTSTicker::GetCoreTicker().Tick(1.1f);
    TestTrue(TEXT("Deferred ticker registers module"), Helper->IsModuleRegistered(&Deferred));
    TestEqual(TEXT("Successful deferred registration delivers startup"), Deferred.Starts, 1);
    Helper->UnregisterModule(&Deferred);
    FAuditModule CoreRuntime(TEXT("GorgeousAuditCoreRuntime")); CoreRuntime.bCore = true; CoreRuntime.Functionality = EGorgeousModuleFunctionality::Runtime;
    TestTrue(TEXT("Core runtime fixture registered"), Helper->RegisterModule(&CoreRuntime));
    FAuditModule SamePlugin(TEXT("GorgeousEvents")); SamePlugin.Dependencies = { TEXT("GorgeousEvents") };
    TestTrue(TEXT("Own-plugin module dependency does not defer or report a circular plugin edge"), Helper->RegisterModule(&SamePlugin));
    TestFalse(TEXT("Plugin dependency graph omits self edge"), Helper->GetGorgeousPluginDependencies(TEXT("GorgeousEvents")).Contains(TEXT("GorgeousEvents")));
    Helper->UnregisterModule(&SamePlugin); Helper->UnregisterModule(&CoreRuntime);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousInGameLoggingRegressionTest, "GorgeousCore.RuntimeUtilities.InGameLogReceiver",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousInGameLoggingRegressionTest::RunTest(const FString&)
{
    UGorgeousInGameLogWidget* Widget = NewObject<UGorgeousInGameLogWidget>();
    for (int32 Index = 0; Index < 101; ++Index)
    {
        IGorgeousLogWidgetReceiver::Execute_ReceiveGorgeousLogMessage(Widget, FString::FromInt(Index), NAME_None, Logging_Information);
    }
    TestEqual(TEXT("Receiver retains a bounded message history"), Widget->Messages.Num(), 100);
    TestEqual(TEXT("Oldest message evicted"), Widget->Messages[0], FString(TEXT("1")));
    TestEqual(TEXT("Latest message delivered"), Widget->Messages.Last(), FString(TEXT("100")));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousLicenseFormatRegressionTest, "GorgeousCore.RuntimeUtilities.LicenseFormatRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousLicenseFormatRegressionTest::RunTest(const FString&)
{
    const auto Plugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
    if (!Plugin.IsValid()) { AddError(TEXT("Core plugin descriptor is required")); return false; }
    const FString File = Plugin->GetBaseDir() / TEXT("License.lic");
    TArray<uint8> Backup;
    const bool bExisted = IFileManager::Get().FileExists(*File);
    if (bExisted && !FFileHelper::LoadFileToArray(Backup, *File)) { AddError(TEXT("Cannot safely back up existing license file")); return false; }
    ON_SCOPE_EXIT { if (bExisted) { FFileHelper::SaveArrayToFile(Backup, *File); } else { IFileManager::Get().Delete(*File); } };
    const FString TestKey(TEXT("GorgeousAuditTemporaryKeyOnly"));
    TestTrue(TEXT("Create licenses in installed plugin folder"), FGorgeousCoreLicenseHelper::CreateEncryptedLicenseFile(TArray<FString>{TEXT("alpha"),TEXT("beta")}, TestKey));
    TestTrue(TEXT("Append preserves entries written by create API"), FGorgeousCoreLicenseHelper::AddLicenseEntry(TEXT("gamma"), TestKey));
    TArray<TPair<FDateTime,FString>> Entries;
    TestTrue(TEXT("Read entry API"), FGorgeousCoreLicenseHelper::ReadAllLicenseEntries(TestKey, Entries));
    TestEqual(TEXT("Create and append formats interoperate"), Entries.Num(), 3);
    if (Entries.Num() == 3)
    {
        TestEqual(TEXT("Original first entry preserved"), Entries[0].Value, FString(TEXT("alpha")));
        TestEqual(TEXT("Original second entry preserved"), Entries[1].Value, FString(TEXT("beta")));
        TestEqual(TEXT("Appended entry preserved"), Entries[2].Value, FString(TEXT("gamma")));
    }
    TArray<FString> Values; FDateTime Timestamp;
    TestTrue(TEXT("Original read API accepts appended format"), FGorgeousCoreLicenseHelper::ReadAndDecryptLicenseFile(TestKey, Values, Timestamp));
    TestTrue(TEXT("Original API returns original strings"), Values == TArray<FString>{TEXT("alpha"),TEXT("beta"),TEXT("gamma")});
    FFileHelper::SaveStringToFile(TEXT("not-valid-base64"), *File);
    TestFalse(TEXT("Malformed file is rejected"), FGorgeousCoreLicenseHelper::ReadAllLicenseEntries(TestKey, Entries));
    TestFalse(TEXT("Append cannot silently discard unreadable file"), FGorgeousCoreLicenseHelper::AddLicenseEntry(TEXT("replacement"), TestKey));
    return true;
}

#endif
