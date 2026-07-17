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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GeneralSystemConfiguration_PDA.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: General System Configuration
| Functional Name: UGeneralSystemConfiguration_PDA
| Parent Class: UGorgeousPrimaryDataAsset
| Class Suffix: _PDA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for General System Configuration.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class UGeneralSystem_AC;
//<------------------------------------------------------------->
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/GeneralSystemConfiguration_PDA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGeneralSystemConfiguration_PDA : public UGorgeousPrimaryDataAsset
{
    GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

    // Returns the primary asset type used for system configurations.
    virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("GorgeousBlueprintSystems"); }
    // Returns the default content paths scanned for system configurations.
    virtual TArray<FString> GetDefaultScanPaths() const override { return { TEXT("Systems") }; }

    // Derives the primary asset identifier from the system configuration path.
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
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Returns this configuration's primary asset identifier.
	 *
	 * @return The primary asset identifier for this configuration.
	 */
    UFUNCTION(BlueprintCallable, Category = "General Core System", meta = (DisplayName = "Get Primary Asset ID"))
    FPrimaryAssetId GetPrimaryAssetId_BP() const
    {
        return GetPrimaryAssetId();
    }

	/**
	 * Returns the derived primary data asset name for this configuration.
	 *
	 * @return The derived asset name, or NAME_None when the path is invalid.
	 */
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

	/**
	 * Returns every registered system configuration asset.
	 *
	 * @return The registered system configuration assets.
	 */
    UFUNCTION(BlueprintCallable, Category = "General Core System")
    static TArray<UGeneralSystemConfiguration_PDA*> GetAllSystemConfigurations();
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

    // The authoritative component of this system that handles server side stuff.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System Configuration")
    TSubclassOf<UGeneralSystem_AC> AuthorativeComponent;

    // The interaction component of this system that handles client side stuff
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System Configuration")
    TSubclassOf<UGeneralSystem_AC> InteractorComponent;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};