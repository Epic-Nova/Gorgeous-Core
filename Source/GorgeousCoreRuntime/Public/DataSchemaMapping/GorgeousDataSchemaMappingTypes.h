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
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousDataSchemaMappingTypes.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Enumeration defining the kind of source data for a data schema mapping.
 * 
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousDataSchemaSourceKind_E : uint8
{
	Source_AnyUObject UMETA(DisplayName = "Any UObject Asset"),
	Source_DataAsset UMETA(DisplayName = "Data Asset"),
	Source_DataTableRows UMETA(DisplayName = "Data Table Rows"),
	Source_BlueprintClassDefaults UMETA(DisplayName = "Blueprint Class Defaults")
};

/**
 * Enumeration defining the kinds of transformations that can be applied to source field values during data migration.
 * 
 * @author Nils Bergemann
 * @note Each transform kind may utilize the provided parameters differently, e.g., as literal values, prefixes/suffixes, search/replace text, or numeric factors/offsets.
 */
UENUM(BlueprintType)
enum class EGorgeousDataSchemaTransformKind_E : uint8
{
	Transform_None = 0 UMETA(DisplayName = "None"),
	Transform_SetLiteral = 9 UMETA(DisplayName = "Set Literal"),
	Transform_UseFallbackLiteralIfEmpty = 10 UMETA(DisplayName = "Fallback Literal If Empty"),
	Transform_TrimWhitespace = 1 UMETA(DisplayName = "Trim Whitespace"),
	Transform_ToLower = 2 UMETA(DisplayName = "Lowercase"),
	Transform_ToUpper = 3 UMETA(DisplayName = "Uppercase"),
	Transform_AddPrefix = 4 UMETA(DisplayName = "Add Prefix"),
	Transform_AddSuffix = 5 UMETA(DisplayName = "Add Suffix"),
	Transform_ReplaceText = 6 UMETA(DisplayName = "Replace Text"),
	Transform_MultiplyNumeric = 7 UMETA(DisplayName = "Multiply Numeric"),
	Transform_AddNumericOffset = 8 UMETA(DisplayName = "Add Numeric Offset"),
	Transform_InvertBoolean = 11 UMETA(DisplayName = "Invert Boolean")
};

/**
 * Structure for Source field definitions in a data schema mapping, 
 * including properties relevant for identifying and validating source items based on the specified SourceKind.
 * 
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaSourceDefinition_S
{
	GENERATED_BODY()

	// The kind of source data this schema mapping expects. This determines which additional properties are relevant for identifying and validating source items.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source")
	EGorgeousDataSchemaSourceKind_E SourceKind = EGorgeousDataSchemaSourceKind_E::Source_AnyUObject;

	// The UObject to draw source mapping properties from for source fields.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_AnyUObject || SourceKind == EGorgeousDataSchemaSourceKind_E::Source_BlueprintClassDefaults", EditConditionHides))
	TSoftClassPtr<UObject> RequiredSourceClass;

	// The DataAsset to draw from for source fields.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataAsset", EditConditionHides))
	TSoftClassPtr<UDataAsset> RequiredSourceDataAssetClass;

	// The DataTable structure to draw from for source fields.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows", EditConditionHides))
	TSoftObjectPtr<UScriptStruct> RequiredDataTableRowStruct;

	// The DataTable to use for direct schema execution from the schema editor (Bulk Migrate).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows", EditConditionHides, ToolTip = "Optional DataTable used for direct schema execution from the schema editor (Bulk Migrate)."))
	TSoftObjectPtr<UDataTable> SourceDataTable;
};

/**
 * Structure for Target field definitions in a data schema mapping, 
 * 
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaTargetDefinition_S
{
	GENERATED_BODY()

	// The class of the target assets to be created by this schema mapping. Must be a subclass of UObject.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	TSoftClassPtr<UObject> TargetClass;

	// A Relative content folder path where assets for BulkMigration are should be saved.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ToolTip = "Content root where migrated assets will be created, e.g. /Game/Systems/MyItems."))
	FString TargetContentRootPath;

	// An optional source field path used to derive the output asset name. If empty, source label is used.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ToolTip = "Optional source field path used to derive the output asset name. If empty, source label is used."))
	FString TargetAssetNameSourcePath;
};

/**
 * Structure for migration-related definitions in a data schema mapping,
 * including properties for enabling migration actions, defining menu action labels, and configuring folder creation behavior during migration.
 * 
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMigrationDefinition_S
{
	GENERATED_BODY()

	// If enabled registers a content browser menu action for this schema mapping to migrate source items into target assets based on the defined field mappings and migration settings.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Migration")
	bool bEnableContentBrowserMigrationAction = false;

	// The label to use for the content browser menu action when bEnableContentBrowserMigrationAction is true. If empty, a default label based on the schema asset name will be used.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Migration", meta = (EditCondition = "bEnableContentBrowserMigrationAction", EditConditionHides))
	FText MenuActionLabel;

	/**
	 * If enabled, migrated assets will be organized into subfolders under the specified TargetContentRootPath based on their source item labels or a specified source field value, depending on the TargetAssetNameSourcePath setting. 
	 * This can help keep migrated assets organized when dealing with a large number of source items or when source items belong to different categories.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Migration", meta = (ToolTip = "If enabled, each migrated source item creates assets inside its own folder under TargetContentRootPath."))
	bool bCreateFolderPerSourceItem = false;
};

/**
 * Structure for defining field mappings in a data schema mapping, 
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaTransformDefinition_S
{
	GENERATED_BODY()

	/** 
	 * The transform kind to apply to source field values during migration. 
	 * Determines how the provided parameters will be used to transform the source value before assigning it to the target field.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform")
	EGorgeousDataSchemaTransformKind_E TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_None;

	/**
	 * Its usage depends on the selected TransformKind, 
	 * e.g., as a literal value for SetLiteral, a prefix/suffix for AddPrefix/AddSuffix, search text for ReplaceText, or a numeric factor/offset for MultiplyNumeric/AddNumericOffset.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform", meta = (ToolTip = "Primary transform parameter. Used as literal value, fallback literal, prefix/suffix, search text, factor, or offset depending on TransformKind."))
	FString ParameterA;

	// Its usage depends on the selected TransformKind, e.g., as replacement text for ReplaceText.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform", meta = (ToolTip = "Secondary transform parameter. Used by Replace Text as replacement text."))
	FString ParameterB;
};

/**
 * Structure for defining field mappings in a data schema mapping,
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaFieldMapping_S
{
	GENERATED_BODY()

	// The source field path to read data from, relative to the source item (e.g., a specific property path on a UObject or a column name in a DataTable).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping", meta = (GetOptions = "GetSourceFieldPathOptions"))
	FString SourcePath;

	// The target field path to write data to, relative to the target asset (e.g., a specific property path on the target UObject).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping", meta = (GetOptions = "GetTargetFieldPathOptions"))
	FString TargetPath;

	// If true, this field mapping is required and the migration will fail if the source field is missing or empty for any source item.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping")
	bool bRequired = false;

	// An optional transformation definition to apply to the source field value before assigning it to the target field during migration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping|Transform")
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
};

/**
 * Structure representing a source value entry for a specific source field path, 
 * used in the context of data schema mapping and migration to hold the extracted value from the source item for that path.
 * 
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaSourceValueEntry_S
{
	GENERATED_BODY()

	// The source field path this value entry corresponds to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	FString Path;

	/** 
	 * The extracted value from the source item for the specified path, stored as a string for simplicity. 
	 * The actual type and format may depend on the source field type and may require parsing or conversion during migration.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	FString ExportedValue;
};

/**
 * Structure representing a snapshot of source data for a specific source item,
 *
 * This structure is used to hold the extracted source data in a format that can be easily accessed and transformed during migration.
 * 
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaSourceSnapshot_S
{
	GENERATED_BODY()

	// A label for the source item this snapshot represents, used for identification and as a potential basis for naming migrated assets.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	FString SourceLabel;

	// An array of value entries representing the extracted values for the defined source field paths in the data schema mapping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	TArray<FGorgeousDataSchemaSourceValueEntry_S> ValueEntries;

	/** 
	 * Attempts to find the value entry for the specified source field path and returns its exported value.
	 * 
	 * @param InPath The source field path to search for.
	 * @param OutValue The output parameter to store the found value.
	 * @return True if the value was found, false otherwise.
	 */
	bool TryGetValueByPath(const FString& InPath, FString& OutValue) const;
};

/**
 * Structure representing the result of migrating a single source item in a data schema mapping.
 *
 * This structure is used to provide detailed feedback on the outcome of migrating each individual source item, which can be aggregated into an overall migration result.
 * 
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMigrationItemResult_S
{
	GENERATED_BODY()

	// A label for the source item this migration result corresponds to, used for identification and reporting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString SourceLabel;

	// The path of the created asset if migration succeeded, or an empty string if migration failed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString CreatedAssetPath;

	// True if migration succeeded for this source item, false if it failed due to missing required fields, transformation errors, or other issues.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSucceeded = false;

	// The number of fields that were evaluated during migration for this source item, including both required and optional fields.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 EvaluatedFieldCount = 0;

	// The number of fields that were actually changed (i.e., had a non-empty source value and were successfully assigned to the target field) during migration for this source item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 ChangedFieldCount = 0;

	// Any warnings encountered during migration for this source item, such as missing optional fields or transformation issues that did not cause migration to fail.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> Warnings;

	// Any errors encountered during migration for this source item, such as missing required fields or transformation errors that caused migration to fail.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> Errors;
};

/**
 * Structure representing the overall result of a data schema migration operation.
 * 
 * This structure is used to provide comprehensive feedback on the outcome of a migration operation, 
 * allowing developers to understand not only whether the migration succeeded or failed overall.
 * 
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMigrationResult_S
{
	GENERATED_BODY()

	/** 
	 * True if the overall migration operation succeeded (i.e., all required fields were present and successfully migrated for all source items), 
	 * false if it failed due to any issues with required fields, transformations, or other errors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSucceeded = false;

	// The total count of assets that were successfully created during the migration operation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 CreatedAssetCount = 0;

	// An array of results for each individual source item that was processed during migration, providing detailed information on the outcome for each item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FGorgeousDataSchemaMigrationItemResult_S> ItemResults;

	/** 
	 * Any global errors that were encountered during the migration process that are not specific to individual source items, 
	 * such as issues with accessing source data, writing assets, or other systemic problems.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> GlobalErrors;
};