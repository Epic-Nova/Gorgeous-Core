#pragma once
#include "Engine/DataAsset.h"
#include "GorgeousMigrationAuditTypes.generated.h"

/** Isolated reflected data for mapper ownership and real editor transaction regressions. */
UCLASS()
class UGorgeousMigrationAuditAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Audit") float Value = 17.0f;
    UPROPERTY(EditAnywhere, Category="Audit") TObjectPtr<UGorgeousMigrationAuditAsset> Reference;
};
