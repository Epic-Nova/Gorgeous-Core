// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
<==========================================================================*/
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Helpers/GorgeousPluginHelper.h"

TArray<UGeneralSystemConfiguration_PDA*> UGeneralSystemConfiguration_PDA::GetAllSystemConfigurations()
{
    TArray<UGeneralSystemConfiguration_PDA*> Results;

    if (!FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
    {
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    }

    FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = ARM.Get();

    // 1) Query by the PDA class directly
    TArray<FAssetData> PDAAssets;
    AssetRegistry.GetAssetsByClass(UGeneralSystemConfiguration_PDA::StaticClass()->GetClassPathName(), PDAAssets, true);

    for (const FAssetData& Data : PDAAssets)
    {
        if (!Data.IsValid())
            continue;

        UObject* Obj = Data.GetAsset(); // may load synchronously
        UGeneralSystemConfiguration_PDA* PDA = Cast<UGeneralSystemConfiguration_PDA>(Obj);

        if (!PDA)
        {
            const FString ObjectPath = Data.GetObjectPathString();
            PDA = LoadObject<UGeneralSystemConfiguration_PDA>(nullptr, *ObjectPath);
        }

        if (PDA)
        {
            Results.AddUnique(PDA);
        }
    }

    // 2) Fallback: scan known /Systems directories
    UGorgeousPluginHelper* PluginHelper = GetMutableDefault<UGorgeousPluginHelper>();
    TSet<FName> Known = PluginHelper ? PluginHelper->GetKnownGorgeousPlugins() : TSet<FName>();

    TArray<FName> PathsToSearch;
    if (Known.Num() == 0)
    {
        PathsToSearch.Add(FName(TEXT("/GorgeousCore/Systems")));
    }
    else
    {
        for (const FName& Plugin : Known)
        {
            PathsToSearch.Add(FName(*FString::Printf(TEXT("/%s/Systems"), *Plugin.ToString())));
        }
    }

    for (const FName& Path : PathsToSearch)
    {
        TArray<FAssetData> Found;
        AssetRegistry.GetAssetsByPath(Path, Found, true);

        for (const FAssetData& Data : Found)
        {
            if (!Data.IsValid())
                continue;

            bool bAlready = false;
            for (UGeneralSystemConfiguration_PDA* Existing : Results)
            {
                if (Existing && Existing->GetPathName() == Data.GetObjectPathString())
                {
                    bAlready = true;
                    break;
                }
            }
            if (bAlready)
                continue;

            UObject* Obj = Data.GetAsset();
            UGeneralSystemConfiguration_PDA* PDA = Cast<UGeneralSystemConfiguration_PDA>(Obj);
            if (!PDA)
            {
                PDA = LoadObject<UGeneralSystemConfiguration_PDA>(nullptr, *Data.GetObjectPathString());
            }

            if (PDA)
            {
                Results.AddUnique(PDA);
            }
        }
    }

    return Results;
}
