// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/DataAsset.h"
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingStructures.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousDataSchemaMapping_DA.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Core schema map asset used by all Gorgeous plugins to migrate data from arbitrary
 * source holders into target assets through reflection-safe field mappings.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousDataSchemaMapping_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (ShowOnlyInnerProperties, DisplayName = "Source Definition"))
	FGorgeousDataSchemaSourceDefinition_S SourceDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (ShowOnlyInnerProperties, DisplayName = "Target Definition"))
	FGorgeousDataSchemaTargetDefinition_S TargetDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (DisplayName = "Field Mappings"))
	TArray<FGorgeousDataSchemaFieldMapping_S> FieldMappings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (ShowOnlyInnerProperties, DisplayName = "Migration Definition"))
	FGorgeousDataSchemaMigrationDefinition_S MigrationDefinition;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	bool IsMigrationActionEnabled() const;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	FString GetNormalizedTargetContentRootPath() const;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	FText GetResolvedMigrationActionLabel() const;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	bool IsDirectDataTableExecutionEnabled() const;

	UFUNCTION()
	TArray<FString> GetSourceFieldPathOptions() const;

	UFUNCTION()
	TArray<FString> GetTargetFieldPathOptions() const;
};
