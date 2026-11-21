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
#include "LibraryWizard/GorgeousUpdateManager.h"

#if WITH_DEV_AUTOMATION_TESTS
//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/StrongObjectPtr.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousNativeUpdatePayloadTest Implementation
//=============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousNativeUpdatePayloadTest,
    "GorgeousCore.Editor.Updates.NativePluginCheckContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousNativeUpdatePayloadTest::RunTest(const FString&)
{
    const FString ScratchRoot = FPaths::ConvertRelativePathToFull(
        FPaths::Combine(FPaths::AutomationTransientDir(), TEXT("GorgeousUpdatePayload")));
    const FString PluginRoot = FPaths::Combine(ScratchRoot, FGuid::NewGuid().ToString(EGuidFormats::Digits));
    const FString PublicCore = FPaths::Combine(PluginRoot, TEXT("Source/GorgeousUpdateFixtureRuntime/Public/ModuleCore"));
    const FString PrivateSource = FPaths::Combine(PluginRoot, TEXT("Source/GorgeousUpdateFixtureRuntime/Private"));
    IFileManager::Get().MakeDirectory(*PublicCore, true);
    IFileManager::Get().MakeDirectory(*PrivateSource, true);
    ON_SCOPE_EXIT
    {
        // Recursive cleanup is restricted to the resolved, unique automation fixture.
        if (FPaths::IsUnderDirectory(PluginRoot, ScratchRoot)) IFileManager::Get().DeleteDirectory(*PluginRoot, false, true);
    };
    const FString FirstHeader = TEXT("// Native license fixture A.\n");
    const FString LastHeader = TEXT("// Native license fixture Z.\n");
    // Create in reverse lexical order to exercise the existing sorted footprint.
    if (!TestTrue(TEXT("Create last ModuleCore header"), FFileHelper::SaveStringToFile(LastHeader,
        *FPaths::Combine(PublicCore, TEXT("LicenseZ.h")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))) return false;
    if (!TestTrue(TEXT("Create first ModuleCore header"), FFileHelper::SaveStringToFile(FirstHeader,
        *FPaths::Combine(PublicCore, TEXT("LicenseA.h")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))) return false;
    if (!TestTrue(TEXT("Create source outside ModuleCore"), FFileHelper::SaveStringToFile(TEXT("// Diagnostic source checksum fixture.\n"),
        *FPaths::Combine(PrivateSource, TEXT("Fixture.cpp")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))) return false;

    const FString ExpectedBaseInput = TEXT("LicenseA.h") + FirstHeader + TEXT("LicenseZ.h") + LastHeader;
    const FTCHARToUTF8 Utf8ExpectedBase(*ExpectedBaseInput);
    const FString ExpectedBaseFootprint = FMD5::HashBytes(
        reinterpret_cast<const uint8*>(Utf8ExpectedBase.Get()), Utf8ExpectedBase.Length());
    TStrongObjectPtr<UGorgeousUpdateManager> Manager(NewObject<UGorgeousUpdateManager>());
    const FString PluginName = TEXT("GorgeousUpdateContractFixture");
    const auto Entry = Manager->BuildPluginUpdateEntry(PluginName, PluginRoot, TEXT("0.9.0"));
    const FString Salt = Entry->GetStringField(TEXT("Salt"));
    FGuid ParsedSalt;
    TestTrue(TEXT("Native payload has a nonempty fresh GUID salt"), FGuid::ParseExact(Salt, EGuidFormats::Digits, ParsedSalt) && ParsedSalt.IsValid());
    const FString ExpectedProofInput = ExpectedBaseFootprint + Salt;
    const FTCHARToUTF8 Utf8ExpectedProof(*ExpectedProofInput);
    const FString ExpectedProof = FMD5::HashBytes(
        reinterpret_cast<const uint8*>(Utf8ExpectedProof.Get()), Utf8ExpectedProof.Length());
    TestEqual(TEXT("Checksum matches API md5(ModuleCore base footprint + salt)"), Entry->GetStringField(TEXT("Checksum")), ExpectedProof);
    TestEqual(TEXT("The diagnostic source SHA-256 uses its own field"), Entry->GetStringField(TEXT("SourceChecksumSha256")).Len(), 64);
    TestNotEqual(TEXT("The diagnostic digest is not sent as the license proof"), Entry->GetStringField(TEXT("SourceChecksumSha256")), Entry->GetStringField(TEXT("Checksum")));
    const auto NextEntry = Manager->BuildPluginUpdateEntry(PluginName, PluginRoot, TEXT("0.9.0"));
    TestNotEqual(TEXT("Each plugin-check entry receives a new salt"), NextEntry->GetStringField(TEXT("Salt")), Salt);
    TestNotEqual(TEXT("A fresh salt changes the transmitted license proof"), NextEntry->GetStringField(TEXT("Checksum")), Entry->GetStringField(TEXT("Checksum")));

    const auto Payload = MakeShared<FJsonObject>();
    Payload->SetArrayField(TEXT("Plugins"), {MakeShared<FJsonValueObject>(Entry)});
    FString Serialized;
    TestTrue(TEXT("Serialize the same plugin-check wire entry used by CheckForUpdates"), FJsonSerializer::Serialize(Payload, TJsonWriterFactory<>::Create(&Serialized)));
    TSharedPtr<FJsonObject> Parsed;
    if (!TestTrue(TEXT("Native request JSON round trips"), FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Serialized), Parsed) && Parsed.IsValid())) return false;
    const auto RoundTrip = Parsed->GetArrayField(TEXT("Plugins"))[0]->AsObject();
    TestEqual(TEXT("Wire plugin identity is intact"), RoundTrip->GetStringField(TEXT("PluginName")), PluginName);
    TestEqual(TEXT("Wire version is intact"), RoundTrip->GetStringField(TEXT("CurrentVersion")), FString(TEXT("0.9.0")));
    TestEqual(TEXT("Wire proof is intact"), RoundTrip->GetStringField(TEXT("Checksum")), ExpectedProof);
    TestEqual(TEXT("Wire salt is intact"), RoundTrip->GetStringField(TEXT("Salt")), Salt);

    FString ExportPath;
    if (FParse::Value(FCommandLine::Get(), TEXT("GorgeousUpdateContractExport="), ExportPath))
    {
        ExportPath = FPaths::ConvertRelativePathToFull(ExportPath);
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(ExportPath), true);
        TestTrue(TEXT("Export native wire JSON for the API cross-language contract test"),
            FFileHelper::SaveStringToFile(Serialized, *ExportPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM));
        const auto Expected = MakeShared<FJsonObject>();
        Expected->SetStringField(TEXT("expectedBaseFootprint"), ExpectedBaseFootprint);
        Expected->SetStringField(TEXT("pluginName"), PluginName);
        Expected->SetStringField(TEXT("currentVersion"), TEXT("0.9.0"));
        FString ExpectedJson;
        FJsonSerializer::Serialize(Expected, TJsonWriterFactory<>::Create(&ExpectedJson));
        const FString ExpectedPath = FPaths::ChangeExtension(ExportPath, TEXT("expected.json"));
        TestTrue(TEXT("Export expected fixture metadata separately from the real wire JSON"),
            FFileHelper::SaveStringToFile(ExpectedJson, *ExpectedPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM));
        AddInfo(FString::Printf(TEXT("Native plugin-check contract exported to %s; fixture metadata %s"), *ExportPath, *ExpectedPath));
    }
    return true;
}
#endif