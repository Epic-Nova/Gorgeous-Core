// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/GorgeousPersistentData.h"
#include "../../../../Shared/GorgeousPersistentDataFile.h"
#include "../Helpers/GorgeousPersistentDataJson.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ScopeExit.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousInstalledPluginDiscoveryTest,
	"Gorgeous.Core.PluginHelper.InstalledDiscovery", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousInstalledPluginDiscoveryTest::RunTest(const FString& Parameters)
{
	const UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton();
	if (!TestNotNull(TEXT("Plugin helper"), Helper)) return false;
	const TSet<FName> Installed = Helper->GetKnownGorgeousPlugins(false);
	const TSet<FName> Enabled = Helper->GetKnownGorgeousPlugins();
	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
	{
		const FName Name(*Plugin->GetName());
		if (!Helper->IsGorgeousPlugin(Name)) continue;
		TestTrue(*FString::Printf(TEXT("Installed discovery includes %s regardless of module registration"), *Plugin->GetName()), Installed.Contains(Name));
		TestEqual(*FString::Printf(TEXT("Enabled discovery matches plugin-manager state for %s"), *Plugin->GetName()), Enabled.Contains(Name), Plugin->IsEnabled());
	}
	TestTrue(TEXT("Core has a canonical persistent entry"), Helper->GetInstalledPluginSystemEntries().Contains(TEXT("Plugin:Core")));
	TestEqual(TEXT("Every installed plugin receives one persistent entry"), Helper->GetInstalledPluginSystemEntries().Num(), Installed.Num());
	FString MissingDigest(TEXT("stale"));
	TestFalse(TEXT("An unknown plugin cannot be verified as an empty binary distribution"), FGorgeousPersistentData::TryComputePluginBinaryChecksum(NAME_None, MissingDigest));
	TestTrue(TEXT("Failed digest clears caller output"), MissingDigest.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousPluginPersistentFieldsTest,
	"Gorgeous.Core.PluginHelper.PersistentFields", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousPluginPersistentFieldsTest::RunTest(const FString& Parameters)
{
	const TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("PrivateOwnerField"), TEXT("preserved"));
	const TSharedRef<FJsonObject> Validation = MakeShared<FJsonObject>();
	Validation->SetNumberField(TEXT("ValidationCount"), 17);
	Validation->SetStringField(TEXT("PrivateValidationField"), TEXT("preserved"));
	Data->SetObjectField(TEXT("SystemValidation"), Validation);
	const TArray<FString> Before{ TEXT("CommonUIFoundation"), TEXT("MyProjectSystem"), TEXT("Plugin:Events") };
	GorgeousPersistentDataJson::WriteInstalledSystems(*Data, Before);
	Validation->RemoveField(TEXT("InstalledPluginsDiscoveryVersion")); // Existing pre-discovery data.
	TestTrue(TEXT("Legacy plugin entries are backfilled once"), GorgeousPersistentDataJson::BackfillInstalledPlugins(*Data, { TEXT("Plugin:Inventory"), TEXT("Plugin:Core") }));
	TestFalse(TEXT("Later installs are left for normal change detection"), GorgeousPersistentDataJson::BackfillInstalledPlugins(*Data, { TEXT("Plugin:LaterInstall") }));
	TestTrue(TEXT("Backfill includes previously omitted plugins"), GorgeousPersistentDataJson::ReadInstalledSystems(*Data).Contains(TEXT("Plugin:Core")));
	TestEqual(TEXT("Top-level fields owned by other subsystems survive"), Data->GetStringField(TEXT("PrivateOwnerField")), FString(TEXT("preserved")));
	TestEqual(TEXT("Nested validation count survives"), Validation->GetNumberField(TEXT("ValidationCount")), 17.0);
	TestEqual(TEXT("Nested private fields survive"), Validation->GetStringField(TEXT("PrivateValidationField")), FString(TEXT("preserved")));
	for (const FString& Entry : Before) TestTrue(TEXT("Backfilling plugins preserves ordinary and previous entries"), GorgeousPersistentDataJson::ReadInstalledSystems(*Data).Contains(Entry));
	const TSharedRef<FJsonObject> Checksums = GorgeousPersistentDataJson::ObjectField(*Data, TEXT("VerifiedBinaryChecksums"));
	Checksums->SetStringField(TEXT("GorgeousCore"), TEXT("legacy-core-digest"));
	GorgeousPersistentDataJson::ObjectField(*Data, TEXT("VerifiedBinaryChecksums"))->SetStringField(TEXT("GorgeousEvents"), TEXT("events-digest"));
	TestEqual(TEXT("Adding another plugin retains the legacy Core value"), Checksums->GetStringField(TEXT("GorgeousCore")), FString(TEXT("legacy-core-digest")));
	const FJsonObject Empty;
	TestTrue(TEXT("Missing SystemValidation does not assert"), GorgeousPersistentDataJson::ReadInstalledSystems(Empty).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousPluginBinaryChecksumTest,
	"Gorgeous.Core.PluginHelper.BinaryChecksum", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousPluginBinaryChecksumTest::RunTest(const FString& Parameters)
{
	const FString Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation/GorgeousPluginChecksum"), FGuid::NewGuid().ToString(EGuidFormats::Digits));
	IFileManager::Get().MakeDirectory(*Directory, true);
	FString Digest;
	TestFalse(TEXT("An empty binary folder is not a valid digest"), GorgeousPersistentDataJson::ComputeBinaryDirectoryChecksum(Directory, Digest));
#if PLATFORM_WINDOWS
	const FString BinaryName(TEXT("Module.dll"));
#elif PLATFORM_MAC
	const FString BinaryName(TEXT("Module.dylib"));
#else
	const FString BinaryName(TEXT("Module.so"));
#endif
	const FString Binary = FPaths::Combine(Directory, BinaryName);
	FFileHelper::SaveStringToFile(TEXT("first built binary"), *Binary);
	TestTrue(TEXT("A built binary produces a digest"), GorgeousPersistentDataJson::ComputeBinaryDirectoryChecksum(Directory, Digest));
	const FString Original = Digest;
	FFileHelper::SaveStringToFile(TEXT("nonbinary debug metadata"), *FPaths::Combine(Directory, TEXT("Module.pdb")));
	TestTrue(TEXT("Digest remains valid with auxiliary files"), GorgeousPersistentDataJson::ComputeBinaryDirectoryChecksum(Directory, Digest));
	TestEqual(TEXT("Auxiliary files do not change the legacy algorithm"), Digest, Original);
	FFileHelper::SaveStringToFile(TEXT("rebuilt binary"), *Binary);
	TestTrue(TEXT("A rebuilt binary produces a digest"), GorgeousPersistentDataJson::ComputeBinaryDirectoryChecksum(Directory, Digest));
	TestNotEqual(TEXT("Binary changes invalidate verification"), Digest, Original);
	IFileManager::Get().DeleteDirectory(*Directory, false, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousPersistentDataServiceTest,
    "Gorgeous.Core.PluginHelper.PersistentService", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousPersistentDataServiceTest::RunTest(const FString&)
{
    TestNotNull(TEXT("Offline cache struct retains its RuntimeUtilities script identity"),
        FindObject<UScriptStruct>(nullptr, TEXT("/Script/GorgeousCoreRuntimeUtilities.GorgeousOfflineSystemCacheEntry")));
    TestNotNull(TEXT("Plugin update cache struct retains its RuntimeUtilities script identity"),
        FindObject<UScriptStruct>(nullptr, TEXT("/Script/GorgeousCoreRuntimeUtilities.GorgeousPluginUpdateCacheEntry")));

    const FString Directory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(),
        TEXT("Automation/GorgeousPersistentData"), FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    IFileManager::Get().MakeDirectory(*Directory, true);
    ON_SCOPE_EXIT
    {
        const FString AutomationRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation")));
        if (Directory.StartsWith(AutomationRoot + TEXT("/"))) IFileManager::Get().DeleteDirectory(*Directory, false, true);
    };
    const FString Path = FPaths::Combine(Directory, TEXT("metadata.json"));
    const FString Initial = TEXT("{\"PrivateOwnerField\":\"preserved\",\"SystemValidation\":{\"ValidationCount\":17,\"ValidationInterval\":13,\"PrivateValidationField\":\"before\",\"InstalledSystems\":[\"ProjectSystem\",\"Plugin:Events\"]},\"VerifiedBinaryChecksums\":{\"UnrelatedPlugin\":\"private-digest\"}}");
    if (!TestTrue(TEXT("Isolated metadata fixture created"), FFileHelper::SaveStringToFile(Initial, *Path))) return false;

    FGorgeousPersistentData Store(Path);
    TSet<FString> Values;
    TestFalse(TEXT("Missing nudge snapshot remains distinguishable"), Store.TryGetKnownNudgePlugins(Values));
    Store.SetKnownNudgePlugins({});
    TestTrue(TEXT("Empty nudge snapshot remains present"), Store.TryGetKnownNudgePlugins(Values));
    TestTrue(TEXT("Present empty nudge baseline contains no plugins"), Values.IsEmpty());
    TestEqual(TEXT("Existing validation interval is loaded"), Store.GetSystemValidationInterval(), 13);
    const auto Installed = Store.GetRecordedInstalledSystems({TEXT("Plugin:Core"), TEXT("Plugin:Inventory")});
    TestTrue(TEXT("Legacy installed plugins are backfilled"), Installed.Contains(TEXT("Plugin:Inventory")));
    TestTrue(TEXT("Ordinary installed systems remain intact"), Installed.Contains(TEXT("ProjectSystem")));
    TestFalse(TEXT("Later plugins are not silently re-baselined"), Store.GetRecordedInstalledSystems({TEXT("Plugin:LaterInstall")}).Contains(TEXT("Plugin:LaterInstall")));

    // A writer outside this service may add private metadata after the snapshot loaded.
    TSharedPtr<FJsonObject> External;
    if (!TestTrue(TEXT("Metadata fixture is readable"), GorgeousPersistentDataFile::ReadObject(Path, External) == GorgeousPersistentDataFile::EReadResult::Loaded)) return false;
    External->SetStringField(TEXT("ExternalPrivateField"), TEXT("latest"));
    External->GetObjectField(TEXT("SystemValidation"))->SetStringField(TEXT("PrivateValidationField"), TEXT("after"));
    FString ExternalContents;
    FJsonSerializer::Serialize(External.ToSharedRef(), TJsonWriterFactory<>::Create(&ExternalContents));
    FFileHelper::SaveStringToFile(ExternalContents, *Path);

    Store.SetAcknowledgedNudges({TEXT("release-one")});
    Store.DismissSystemActionPrompt(TEXT("prompt-one"));
    Store.SetNudgeFeedCache(TEXT("{\"schemaVersion\":7,\"entries\":[]}"));
    Store.SetSystemValidationCount(18);
    Store.SetForceDevMode(true);
    TestTrue(TEXT("Circular notification records first display"), Store.MarkCircularDependencyNotified(TEXT("A|B")));
    TestFalse(TEXT("Circular notification is not repeated"), Store.MarkCircularDependencyNotified(TEXT("A|B")));
#if WITH_EDITOR
    TestFalse(TEXT("Banner starts unseen"), Store.HasShownResourceProtectionBanner());
    Store.MarkResourceProtectionBannerShown();
#endif
    TSharedPtr<FJsonObject> Saved;
    if (!TestTrue(TEXT("Owned writes leave valid JSON"), GorgeousPersistentDataFile::ReadObject(Path, Saved) == GorgeousPersistentDataFile::EReadResult::Loaded)) return false;
    TestEqual(TEXT("Unknown root field survives owned writes"), Saved->GetStringField(TEXT("PrivateOwnerField")), FString(TEXT("preserved")));
    TestEqual(TEXT("New external root field survives owned writes"), Saved->GetStringField(TEXT("ExternalPrivateField")), FString(TEXT("latest")));
    TestEqual(TEXT("Latest unknown nested field survives owned writes"), Saved->GetObjectField(TEXT("SystemValidation"))->GetStringField(TEXT("PrivateValidationField")), FString(TEXT("after")));
    TestEqual(TEXT("Known validation leaf updates"), Saved->GetObjectField(TEXT("SystemValidation"))->GetIntegerField(TEXT("ValidationCount")), 18);

    FGorgeousPersistentData Reloaded(Path);
    TestTrue(TEXT("Acknowledgements survive another service load"), Reloaded.GetAcknowledgedNudges().Contains(TEXT("release-one")));
    TestTrue(TEXT("Dismissed prompt survives another service load"), Reloaded.IsSystemActionPromptDismissed(TEXT("prompt-one")));
    TestTrue(TEXT("Developer override survives another service load"), Reloaded.GetForceDevMode());
    TestFalse(TEXT("Circular record survives another service load"), Reloaded.MarkCircularDependencyNotified(TEXT("A|B")));
#if WITH_EDITOR
    TestTrue(TEXT("Banner record survives another service load"), Reloaded.HasShownResourceProtectionBanner());
#endif
    FString FeedJson;
    TestTrue(TEXT("Cached feed protocol is reconstructed"), Reloaded.TryGetNudgeFeedCache(FeedJson));
    TSharedPtr<FJsonObject> Feed;
    FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(FeedJson), Feed);
    TestTrue(TEXT("Feed protocol does not expose persistent root fields"), Feed.IsValid() && !Feed->HasField(TEXT("PrivateOwnerField")));
    if (Feed.IsValid()) TestEqual(TEXT("Feed schema is retained"), Feed->GetIntegerField(TEXT("schemaVersion")), 7);
    Reloaded.InvalidatePersistedChecksum({FName(TEXT("GorgeousCore"))});
    GorgeousPersistentDataFile::ReadObject(Path, Saved);
    TestEqual(TEXT("Checksum invalidation retains unknown plugin data"),
        Saved->GetObjectField(TEXT("VerifiedBinaryChecksums"))->GetStringField(TEXT("UnrelatedPlugin")), FString(TEXT("private-digest")));
    TestEqual(TEXT("Checksum invalidation retains legacy Core empty-digest contract"),
        Saved->GetObjectField(TEXT("VerifiedBinaryChecksums"))->GetStringField(TEXT("GorgeousCore")), FString());

    const FString PartialPath = FPaths::Combine(Directory, TEXT("partial.json"));
    FFileHelper::SaveStringToFile(TEXT("{\"SystemValidation\":{\"PrivateField\":\"keep\"}}"), *PartialPath);
    FGorgeousPersistentData PartialStore(PartialPath);
    TestEqual(TEXT("Partial legacy validation defaults missing count to zero"), PartialStore.GetSystemValidationCount(), 0);
    PartialStore.IncrementSystemValidationCount();
    TestEqual(TEXT("Partial legacy validation increments from zero"), PartialStore.GetSystemValidationCount(), 1);
    GorgeousPersistentDataFile::ReadObject(PartialPath, Saved);
    TestEqual(TEXT("Filling a missing validation count preserves private fields"),
        Saved->GetObjectField(TEXT("SystemValidation"))->GetStringField(TEXT("PrivateField")), FString(TEXT("keep")));

    // Both an initial parse failure and corruption after loading must fail closed.
    AddExpectedError(TEXT("Could not parse Gorgeous persistent data"), EAutomationExpectedErrorFlags::Contains, 2);
    const FString Broken(TEXT("{ invalid-json"));
    const FString BrokenPath = FPaths::Combine(Directory, TEXT("broken.json"));
    FFileHelper::SaveStringToFile(Broken, *BrokenPath);
    FGorgeousPersistentData BrokenStore(BrokenPath);
    BrokenStore.SetForceDevMode(true);
    FString Contents;
    FFileHelper::LoadFileToString(Contents, *BrokenPath);
    TestEqual(TEXT("Malformed pre-existing metadata is not replaced"), Contents, Broken);

    const FString ChangedPath = FPaths::Combine(Directory, TEXT("changed.json"));
    FFileHelper::SaveStringToFile(Initial, *ChangedPath);
    FGorgeousPersistentData ChangedStore(ChangedPath);
    ChangedStore.LoadPersistentData();
    FFileHelper::SaveStringToFile(Broken, *ChangedPath);
    ChangedStore.SetSystemValidationCount(99);
    FFileHelper::LoadFileToString(Contents, *ChangedPath);
    TestEqual(TEXT("A file corrupted after loading is not replaced"), Contents, Broken);
    return true;
}
#endif