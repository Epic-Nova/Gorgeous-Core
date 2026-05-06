// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GorgeousPrimaryDataAsset.generated.h"

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
		FPrimaryAssetType Type = GetPrimaryAssetType();
		if (Type.IsValid())
		{
			return FPrimaryAssetId(Type, GetFName());
		}
		return Super::GetPrimaryAssetId();
	}
};
