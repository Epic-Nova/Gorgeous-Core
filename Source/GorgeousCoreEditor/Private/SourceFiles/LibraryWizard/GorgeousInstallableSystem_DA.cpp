// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "LibraryWizard/GorgeousInstallableSystem_DA.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool UGorgeousInstallableSystem_DA::CheckIsInstalled() const
{
    // If it's pre-shipped, it's intrinsically installed by the Core module
    if (bIsPreShipped)
    {
        return true;
    }

    TSharedPtr<IPlugin> TargetPlugin = IPluginManager::Get().FindPlugin(TargetPluginName);
    if (!TargetPlugin.IsValid())
    {
        return false;
    }

    const FString PluginBaseDir = TargetPlugin->GetBaseDir();
    bool bHasAnyValidPath = false;
    FString ManifestPathToRead;

    // Check Source Paths
    for (const FString& Path : SourcePaths)
    {
        const FString FullPath = FPaths::Combine(PluginBaseDir, Path);
        if (IFileManager::Get().DirectoryExists(*FullPath) || FPaths::FileExists(FullPath))
        {
            bHasAnyValidPath = true;
            ManifestPathToRead = FPaths::Combine(FullPath, TEXT("SystemManifest.json"));
            break;
        }
    }

    // Check Content Paths
    if (!bHasAnyValidPath)
    {
        for (const FString& Path : ContentPaths)
        {
            const FString FullPath = FPaths::Combine(PluginBaseDir, Path);
            if (IFileManager::Get().DirectoryExists(*FullPath) || FPaths::FileExists(FullPath))
            {
                bHasAnyValidPath = true;
                ManifestPathToRead = FPaths::Combine(FullPath, TEXT("SystemManifest.json"));
                break;
            }
        }
    }

    if (bHasAnyValidPath)
    {
        // Default to Unknown if we don't find a manifest but the folder exists
        const_cast<UGorgeousInstallableSystem_DA*>(this)->InstalledVersion = TEXT("Unknown");

        if (!ManifestPathToRead.IsEmpty() && FPaths::FileExists(ManifestPathToRead))
        {
            FString FileContents;
            if (FFileHelper::LoadFileToString(FileContents, *ManifestPathToRead))
            {
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);
                if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
                {
                    if (JsonObject->HasField(TEXT("Version")))
                    {
                        const_cast<UGorgeousInstallableSystem_DA*>(this)->InstalledVersion = JsonObject->GetStringField(TEXT("Version"));
                    }
                }
            }
        }
    }

    return bHasAnyValidPath;
}
