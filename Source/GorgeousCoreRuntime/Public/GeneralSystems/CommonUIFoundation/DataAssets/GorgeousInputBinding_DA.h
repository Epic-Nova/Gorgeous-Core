// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
#include "GorgeousInputBinding_DA.generated.h"

/**
 * Maps Gameplay Tags to Enhanced Input Action assets and metadata.
 * Allows the Gorgeous logic kernel to remain tag-based while leveraging Enhanced Input features.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousInputBinding_DA : public UGorgeousPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** UGorgeousPrimaryDataAsset Interface */
	virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("InputBinding"); }
	virtual TArray<FString> GetPreferredScanPaths() const override { return { TEXT("Input/Bindings") }; }
	/** Map of conceptual action tags to input action metadata. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TMap<FGameplayTag, FGorgeousInputBindingInfo_S> Bindings;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
