// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
<==========================================================================*/
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Helpers/GorgeousPluginHelper.h"

#include "Engine/AssetManager.h"

TArray<UGeneralSystemConfiguration_PDA*> UGeneralSystemConfiguration_PDA::GetAllSystemConfigurations()
{
    TArray<UGeneralSystemConfiguration_PDA*> Results;

    UAssetManager& AssetManager = UAssetManager::Get();
    TArray<FAssetData> AssetDataList;

    // HIGH PERFORMANCE LOOKUP:
    // Instead of scanning the entire Asset Registry or disk, we perform a 
    // direct O(1) lookup in the Asset Manager's primary type index.
    AssetManager.GetPrimaryAssetDataList(TEXT("GorgeousBlueprintSystems"), AssetDataList);

    for (const FAssetData& Data : AssetDataList)
    {
        if (UGeneralSystemConfiguration_PDA* PDA = Cast<UGeneralSystemConfiguration_PDA>(Data.GetAsset()))
        {
            Results.AddUnique(PDA);
        }
        else if (UGeneralSystemConfiguration_PDA* LoadedPDA = LoadObject<UGeneralSystemConfiguration_PDA>(nullptr, *Data.GetSoftObjectPath().ToString()))
        {
            Results.AddUnique(LoadedPDA);
        }
    }

    return Results;
}
