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
class UGorgeousGeneralSystemAction;
//<------------------------------------------------------------->
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Anatomy/#ugeneralsystemconfiguration_pda",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		)
)
class GORGEOUSCORERUNTIME_API UGeneralSystemConfiguration_PDA : public UGorgeousPrimaryDataAsset
{
    GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

#if WITH_EDITOR
    virtual void PreSave(FObjectPreSaveContext SaveContext) override;
    /** Refreshes the cooked requirement mirror; returns whether serialized data changed. */
    bool RefreshRuntimeRequirements();
#endif

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

	/**
	 * Returns the names of every registered system configuration.
	 * 
	 * @return An array of FName representing the names of all registered system configurations.
	 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "General Core System")
    static TArray<FName> GetAllSystemNames();

	/**
	 * Checks if a registered system configuration with the specified name exists.
	 *
	 * @param SystemName The name of the system configuration to check for.
	 * @return True if a registered system configuration with the specified name exists, false otherwise.
	 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "General Core System")
    static bool IsSystemPresent(FName SystemName);

    /** 
     * Activates this system after recursively activating its required systems. 
     * 
     * @return True if the system was successfully activated, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "General Core System")
    bool ActivateSystem();

    /** Runs deactivation callbacks for this system's actions. */
    UFUNCTION(BlueprintCallable, Category = "General Core System")
    void DeactivateSystem();

    /**
     * Checks if the system is currently active.
     * 
     * @return True if the system is currently active, false otherwise.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "General Core System")
	bool IsSystemActive() const { return bIsSystemActive; }

    /** Activates every discovered system configuration in dependency order. */
    UFUNCTION(BlueprintCallable, Category = "General Core System")
    static void ActivateAllSystemConfigurations();
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

    // The authoritative component of this system that handles server side stuff.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System Configuration", meta = (EditCondition = "!bIsCodeSystem", EditConditionHides))
    TSubclassOf<UGeneralSystem_AC> AuthoritativeComponent;

    // The interaction component of this system that handles client side stuff
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System Configuration", meta = (EditCondition = "!bIsCodeSystem", EditConditionHides))
    TSubclassOf<UGeneralSystem_AC> InteractorComponent;

    // Marks systems implemented entirely in code and without generated system components.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System Configuration")
    bool bIsCodeSystem = false;

    // Extensible lifecycle actions, patterned after Game Feature Actions.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "General Core System Configuration|Actions")
    TArray<TObjectPtr<UGorgeousGeneralSystemAction>> Actions;

    /** Cooked mirror of runtime-capable plugin requirements. False means mandatory. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "General Core System Configuration|Requirements")
    TMap<FName, bool> RuntimeRequiredPlugins;

    /** Cooked mirror of system requirements. False means mandatory. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "General Core System Configuration|Requirements")
    TMap<FName, bool> RuntimeRequiredSystems;

private:
	
    /**
     * Recursively activates this system and its required systems, while tracking the activation stack to prevent circular dependencies.
     * 
	 * @param ActivationStack A set of systems currently being activated to detect circular dependencies.
	 * @return True if the system and all its required systems were successfully activated, false otherwise.
	 */
    bool ActivateSystemInternal(TSet<const UGeneralSystemConfiguration_PDA*>& ActivationStack);
	
	// Tracks whether this system is currently active.
    bool bIsSystemActive = false;
	
	// Tracks the actions that have been activated for this system configuration.
    TSet<TObjectPtr<UGorgeousGeneralSystemAction>> ActivatedActions;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};
