// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GorgeousPrimaryDataAsset.generated.h"

/**
 * Configuration structure to define how a primary asset handles Asset Manager registration.
 */
USTRUCT(BlueprintType)
struct FGorgeousAssetRegistrationConfig_S
{
	GENERATED_BODY()

	/** If false, this asset type will NOT be automatically registered or validated for registration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager")
	bool bShouldRegister = true;

	/** If true, validation will fail if it's not registered. If false, it ignores registration status. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager", meta=(EditCondition="bShouldRegister"))
	bool bRequireRegistration = true;
};

/**
 * Base class for all Gorgeous Primary Data Assets.
 * Provides metadata for automatic Asset Manager registration and discovery.
 */
UCLASS(Abstract, BlueprintType, Config = Game)
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

	/** Registration configuration for Asset Manager validation. */
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
