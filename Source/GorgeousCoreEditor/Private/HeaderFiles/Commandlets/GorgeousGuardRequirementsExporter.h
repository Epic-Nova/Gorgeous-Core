// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once
#include "AssetRegistry/AssetRegistryModule.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/SecureHash.h"
#include "UObject/Package.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

/** Explicit metadata export; never changes or saves a guard asset. */
namespace GorgeousGuardRequirements
{
inline int32 Export(const FString& Params)
{
    FString PluginName = TEXT("GorgeousCore");
    FParse::Value(*Params, TEXT("GuardPlugin="), PluginName);
    const auto Plugin = IPluginManager::Get().FindPlugin(PluginName);
    if (!Plugin) { UE_LOG(LogTemp, Error, TEXT("GUARD_MANIFEST: plugin not found: %s"), *PluginName); return 1; }
    FString Root = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir()) + TEXT("/");
    FPaths::NormalizeDirectoryName(Root);
    Root += TEXT("/");
    IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    Registry.SearchAllAssets(true);
    FARFilter Filter;
    Filter.ClassPaths.Add(UGorgeousExtensionResourceGuard::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;
    Filter.PackagePaths.Add(FName(*(TEXT("/") + PluginName)));
    Filter.bRecursivePaths = true;
    TArray<FAssetData> Assets;
    Registry.GetAssets(Filter, Assets);
    if (Assets.IsEmpty()) { UE_LOG(LogTemp, Error, TEXT("GUARD_MANIFEST: no guard assets found under /%s; refusing an empty export"), *PluginName); return 1; }
    Assets.Sort([](const FAssetData& A, const FAssetData& B) { return A.GetSoftObjectPath().ToString() < B.GetSoftObjectPath().ToString(); });
    TArray<TSharedPtr<FJsonValue>> Guards;
    TSet<FName> Identifiers;
    TSet<FString> Folders;
    auto Requirements = [](const TMap<FName, bool>& Map)
    {
        TArray<FName> Names; Map.GetKeys(Names);
        Names.Sort([](FName A, FName B) { return A.ToString() < B.ToString(); });
        TArray<TSharedPtr<FJsonValue>> Values;
        for (FName Name : Names)
        {
            auto Entry = MakeShared<FJsonObject>();
            Entry->SetStringField(TEXT("Name"), Name.ToString());
            Entry->SetBoolField(TEXT("Optional"), Map.FindChecked(Name));
            Values.Add(MakeShared<FJsonValueObject>(Entry));
        }
        return Values;
    };
    for (const auto& Asset : Assets)
    {
        const auto* Guard = Cast<UGorgeousExtensionResourceGuard>(Asset.GetAsset());
        if (!Guard || Guard->SystemIdentifier.IsNone() || Identifiers.Contains(Guard->SystemIdentifier) || Guard->GetOutermost()->IsDirty())
        {
            UE_LOG(LogTemp, Error, TEXT("GUARD_MANIFEST: invalid, duplicate, or unsaved guard: %s"), *Asset.GetSoftObjectPath().ToString()); return 1;
        }
        Identifiers.Add(Guard->SystemIdentifier);
        FString Filename = FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(Asset.PackageName.ToString(), FPackageName::GetAssetPackageExtension()));
        FPaths::NormalizeFilename(Filename);
        if (!Filename.StartsWith(Root) || !FPaths::FileExists(Filename)) { UE_LOG(LogTemp, Error, TEXT("GUARD_MANIFEST: source asset outside plugin or missing: %s"), *Filename); return 1; }
        FString Relative = Filename.Mid(Root.Len());
        Folders.Add(FPaths::GetPath(Relative));
        auto Entry = MakeShared<FJsonObject>();
        Entry->SetStringField(TEXT("AssetPath"), Guard->GetPathName());
        Entry->SetStringField(TEXT("GuardClass"), Guard->GetClass()->GetPathName());
        Entry->SetStringField(TEXT("SourceAsset"), Relative);
        Entry->SetStringField(TEXT("SystemIdentifier"), Guard->SystemIdentifier.ToString());
        Entry->SetStringField(TEXT("OwningPluginName"), Guard->OwningPluginName.ToString());
        Entry->SetBoolField(TEXT("IsContentPackGuard"), Guard->bIsContentPackGuard);
        Entry->SetStringField(TEXT("ContentSubPath"), Guard->ContentSubPath);
        Entry->SetArrayField(TEXT("RequiredPlugins"), Requirements(Guard->RequiredPlugins));
        Entry->SetArrayField(TEXT("RequiredBlueprintPacks"), Requirements(Guard->RequiredBlueprintPacks));
        Guards.Add(MakeShared<FJsonValueObject>(Entry));
    }
    TArray<FString> FolderNames = Folders.Array(); FolderNames.Sort();
    TSet<FString> SourceNames;
    TArray<TSharedPtr<FJsonValue>> JsonFolders;
    for (const auto& Folder : FolderNames)
    {
        JsonFolders.Add(MakeShared<FJsonValueString>(Folder));
        TArray<FString> Files;
        IFileManager::Get().FindFilesRecursive(Files, *(Root + Folder), TEXT("*.uasset"), true, false);
        for (auto File : Files) { FPaths::NormalizeFilename(File); SourceNames.Add(File.Mid(Root.Len())); }
    }
    TArray<FString> Names = SourceNames.Array(); Names.Sort();
    TArray<TSharedPtr<FJsonValue>> Sources;
    for (const auto& Name : Names)
    {
        TArray<uint8> Bytes;
        if (!FFileHelper::LoadFileToArray(Bytes, *(Root + Name)))
        { UE_LOG(LogTemp, Error, TEXT("GUARD_MANIFEST: cannot hash %s"), *Name); return 1; }
        auto Entry = MakeShared<FJsonObject>();
        Entry->SetStringField(TEXT("Path"), Name);
        // This fingerprint detects stale build inputs; it is not a package signature.
        Entry->SetStringField(TEXT("Sha1"), FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num()).ToString().ToLower());
        Sources.Add(MakeShared<FJsonValueObject>(Entry));
    }
    auto Document = MakeShared<FJsonObject>();
    Document->SetNumberField(TEXT("SchemaVersion"), 1);
    Document->SetStringField(TEXT("HashAlgorithm"), TEXT("SHA1"));
    Document->SetStringField(TEXT("PluginName"), PluginName);
    Document->SetArrayField(TEXT("GuardFolders"), JsonFolders);
    Document->SetArrayField(TEXT("Sources"), Sources);
    Document->SetArrayField(TEXT("Guards"), Guards);
    FString Output;
    FJsonSerializer::Serialize(Document, TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Output));
    FString Destination = Root + TEXT("Config/GorgeousGuardRequirements.json");
    FParse::Value(*Params, TEXT("GuardManifestOutput="), Destination);
    FString Existing;
    if (!FFileHelper::LoadFileToString(Existing, *Destination) || Existing != Output)
    {
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Destination), true);
        if (!FFileHelper::SaveStringToFile(Output, *Destination, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
        { UE_LOG(LogTemp, Error, TEXT("GUARD_MANIFEST: cannot write %s"), *Destination); return 1; }
    }
    UE_LOG(LogTemp, Display, TEXT("GUARD_MANIFEST_RESULT Plugin=%s Guards=%d Sources=%d Path=%s"), *PluginName, Guards.Num(), Sources.Num(), *Destination);
    return 0;
}
}
