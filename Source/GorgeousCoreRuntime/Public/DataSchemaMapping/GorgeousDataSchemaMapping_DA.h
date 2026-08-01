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
#include "DataSchemaMapping/GorgeousDataSchemaMappingTypes.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousDataSchemaMapping_DA.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Data Schema Mapping
| Functional Name: UGorgeousDataSchemaMapping_DA
| Parent Class: UPrimaryDataAsset
| Class Suffix: _DA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Data Schema Mapping.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/DataSchemaMapping/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/DataSchemaMapping/GorgeousDataSchemaMapping_DA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/DataSchemaMapping/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousDataSchemaMapping_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

	  //<=======================--- Blueprint Functions ---=======================>
#pragma region Blueprint Functions
public:

	/**
	 * Checks if the migration action defined in the MigrationDefinition is enabled.
	 *
	 * @return true if the migration action is enabled, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	bool IsMigrationActionEnabled() const;

	/**
	 * Gets the normalized target content root path based on the current migration definition.
	 *
	 * @return The normalized target content root path, or an empty string if no valid path is defined.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	FString GetNormalizedTargetContentRootPath() const;

	/**
	 * Gets the label for the resolved migration action based on the current migration definition.
	 *
	 * @return The label for the resolved migration action, or an empty text if no action is resolved.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	FText GetResolvedMigrationActionLabel() const;

	/**
	 * Checks if direct data table execution is enabled for this schema mapping asset.
	 * This determines whether the "Bulk Migrate" button is shown in the Data Schema Mapping Editor when a data table is used as the source.
	 *
	 * @return true if direct data table execution is enabled, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Data Schema Mapping")
	bool IsDirectDataTableExecutionEnabled() const;

	/**
	 * Gets the available source field path options based on the current source definition.
	 * This is used to populate dropdowns in the editor for selecting source fields.
	 *
	 * @return An array of strings representing the available source field path options.
	 */
	UFUNCTION()
	TArray<FString> GetSourceFieldPathOptions() const;

	/**
	 * Gets the available target field path options based on the current target definition.
	 * This is used to populate dropdowns in the editor for selecting target fields.
	 *
	 * @return An array of strings representing the available target field path options.
	 */
	UFUNCTION()
	TArray<FString> GetTargetFieldPathOptions() const;
	//<------------------------------------------------------------------------->
#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
#pragma region UAT/UBT Exposed Variables

	// The source definition struct that defines where and how to get the source data.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (ShowOnlyInnerProperties, DisplayName = "Source Definition"))
	FGorgeousDataSchemaSourceDefinition_S SourceDefinition;

	// The target definition struct that defines where and how to apply the data to the target object.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (ShowOnlyInnerProperties, DisplayName = "Target Definition"))
	FGorgeousDataSchemaTargetDefinition_S TargetDefinition;

	/**
	 * An array of field mappings that define how to map individual fields from the source to the target.
	 * Each mapping can include optional transform definitions to modify the data during the mapping process.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (DisplayName = "Field Mappings"))
	TArray<FGorgeousDataSchemaFieldMapping_S> FieldMappings;

	// The migration definition struct that defines the overall migration action and settings for this schema mapping.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (ShowOnlyInnerProperties, DisplayName = "Migration Definition"))
	FGorgeousDataSchemaMigrationDefinition_S MigrationDefinition;
	//<------------------------------------------------------------------------->
#pragma endregion UAT/UBT Exposed Variables
};