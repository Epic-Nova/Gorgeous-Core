// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
#include "GeneralSystemConfiguration_PDA.generated.h"

class UGeneralSystem_AC;

UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGeneralSystemConfiguration_PDA : public UGorgeousPrimaryDataAsset
{
    GENERATED_BODY()
    
public:
    /** UGorgeousPrimaryDataAsset Interface */
    virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("GorgeousBlueprintSystems"); }
    virtual TArray<FString> GetDefaultScanPaths() const override { return { TEXT("Systems") }; }

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        FString SystemName = GetPathName();

        // Extract after /Systems/
        if (!SystemName.Split(TEXT("/Systems/"), nullptr, &SystemName))
        {
            return FPrimaryAssetId(); // fallback if not in expected path
        }

        // Trim subfolders
        int32 SlashIndex = INDEX_NONE;
        if (SystemName.FindChar(TEXT('/'), SlashIndex))
        {
            SystemName.LeftInline(SlashIndex);
        }

        const FName ExpectedName = *FString::Printf(TEXT("PDA_%s"), *SystemName);

        const FPrimaryAssetId ExpectedId(
            TEXT("GorgeousBlueprintSystems"),
            ExpectedName
        );
        
        return ExpectedId;
    }

    UFUNCTION(BlueprintCallable, Category = "General Core System", meta = (DisplayName = "Get Primary Asset ID"))
    FPrimaryAssetId GetPrimaryAssetId_BP() const
    {
        return GetPrimaryAssetId();
    }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "General Core System", meta = (DisplayName = "Get PDA Asset Name"))
    FName GetPDAAssetName() const
    {
        FString SystemName = GetPathName();
        if (!SystemName.Split(TEXT("/Systems/"), nullptr, &SystemName))
        {
            return NAME_None;
        }

        int32 SlashIndex = INDEX_NONE;
        if (SystemName.FindChar(TEXT('/'), SlashIndex))
        {
            SystemName.LeftInline(SlashIndex);
        }

        return *FString::Printf(TEXT("PDA_%s"), *SystemName);
    }

    UFUNCTION(BlueprintCallable, Category = "General Core System")
    static TArray<UGeneralSystemConfiguration_PDA*> GetAllSystemConfigurations();

    // The authoritative component of this system that handles server side stuff.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System Configuration")
    TSubclassOf<UGeneralSystem_AC> AuthorativeComponent;
    
    // The interaction component of this system that handles client side stuff
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System Configuration")
    TSubclassOf<UGeneralSystem_AC> InteractorComponent;
};
