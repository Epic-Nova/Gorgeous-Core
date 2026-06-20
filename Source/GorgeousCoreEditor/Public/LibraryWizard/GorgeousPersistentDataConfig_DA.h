#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "GorgeousPersistentDataConfig_DA.generated.h"

/**
 * Memory-only proxy asset that allows viewing and editing the GorgeousPersistentData.json
 * directly from the Gorgeous Library UI.
 */
UCLASS(BlueprintType)
class GORGEOUSCOREEDITOR_API UGorgeousPersistentDataConfig_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual bool IsEditorOnly() const override { return true; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (ClampMin = "1"))
	int32 ValidationInterval = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	int32 ValidationCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	bool bHasRunInitialValidation = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cache")
	TArray<FGorgeousOfflineSystemCacheEntry> OfflineSystemCache;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cache")
	TArray<FGorgeousPluginUpdateCacheEntry> PluginUpdateCache;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
