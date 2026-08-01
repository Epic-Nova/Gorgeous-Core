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

//<=============================--- Includes ---============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousPrimaryDataAsset.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Defines how a primary asset participates in Asset Manager registration.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousAssetRegistrationConfig_S
{
	GENERATED_BODY()

	// If false, this asset type will NOT be automatically registered or validated for registration.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager")
	bool bShouldRegister = true;

	// If true, validation will fail if it's not registered. If false, it ignores registration status.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager", meta=(EditCondition="bShouldRegister"))
	bool bRequireRegistration = true;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Primary Data Asset
| Functional Name: UGorgeousPrimaryDataAsset
| Parent Class: UPrimaryDataAsset
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base class for primary data assets that participate in automatic Asset
| Manager registration and content discovery.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Abstract, BlueprintType, Config = Game,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/GorgeousPrimaryDataAsset",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UGorgeousPrimaryDataAsset() {}

	/**
	 * Returns the primary asset type for this class.
	 * Overridden by children to define their registry name (e.g. "UI_State").
	 */
	virtual FPrimaryAssetType GetPrimaryAssetType() const { return FPrimaryAssetType(); }

	/**
	 * Returns the preferred directories for this asset type relative to the plugin content root.
	 * Can be overridden globally via DefaultGame.ini.
	 */
	virtual TArray<FString> GetPreferredScanPaths() const
	{
		return PreferredScanPaths.Num() > 0 ? PreferredScanPaths : GetDefaultScanPaths();
	}

	// Registration configuration for Asset Manager validation.
	UPROPERTY(EditAnywhere, Category = "Asset Manager")
	FGorgeousAssetRegistrationConfig_S RegistrationConfig;

protected:
	/** Internal default paths if no config override is present. */
	virtual TArray<FString> GetDefaultScanPaths() const { return {}; }

	/**
	 * Project-wide directory overrides for this asset type.
	 * Set this in DefaultGame.ini under the specific class section.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Asset Manager")
	TArray<FString> PreferredScanPaths;

	/** UPrimaryDataAsset interface */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		if (!RegistrationConfig.bShouldRegister)
		{
			return FPrimaryAssetId(); // Return invalid ID if it explicitly opts out
		}

		FPrimaryAssetType Type = GetPrimaryAssetType();
		if (Type.IsValid())
		{
			return FPrimaryAssetId(Type, GetFName());
		}
		return Super::GetPrimaryAssetId();
	}
};