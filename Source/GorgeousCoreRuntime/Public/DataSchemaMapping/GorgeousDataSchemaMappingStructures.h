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
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousDataSchemaMappingStructures.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UObject;
class UScriptStruct;
class UDataTable;
class UDataAsset;
//<-------------------------------------------------->

UENUM(BlueprintType)
enum class EGorgeousDataSchemaSourceKind_E : uint8
{
	Source_AnyUObject UMETA(DisplayName = "Any UObject Asset"),
	Source_DataAsset UMETA(DisplayName = "Data Asset"),
	Source_DataTableRows UMETA(DisplayName = "Data Table Rows"),
	Source_BlueprintClassDefaults UMETA(DisplayName = "Blueprint Class Defaults")
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaSourceDefinition_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source")
	EGorgeousDataSchemaSourceKind_E SourceKind = EGorgeousDataSchemaSourceKind_E::Source_AnyUObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_AnyUObject || SourceKind == EGorgeousDataSchemaSourceKind_E::Source_BlueprintClassDefaults", EditConditionHides))
	TSoftClassPtr<UObject> RequiredSourceClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataAsset", EditConditionHides))
	TSoftClassPtr<UDataAsset> RequiredSourceDataAssetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows", EditConditionHides))
	TSoftObjectPtr<UScriptStruct> RequiredDataTableRowStruct;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source", meta = (EditCondition = "SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows", EditConditionHides, ToolTip = "Optional DataTable used for direct schema execution from the schema editor (Bulk Migrate)."))
	TSoftObjectPtr<UDataTable> SourceDataTable;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaTargetDefinition_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	TSoftClassPtr<UObject> TargetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ToolTip = "Content root where migrated assets will be created, e.g. /Game/Systems/MyItems."))
	FString TargetContentRootPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ToolTip = "Optional source field path used to derive the output asset name. If empty, source label is used."))
	FString TargetAssetNameSourcePath;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMigrationDefinition_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Migration")
	bool bEnableContentBrowserMigrationAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Migration")
	FText MenuActionLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Migration", meta = (ToolTip = "If enabled, each migrated source item creates assets inside its own folder under TargetContentRootPath."))
	bool bCreateFolderPerSourceItem = false;
};

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

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaTransformDefinition_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform")
	EGorgeousDataSchemaTransformKind_E TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform", meta = (ToolTip = "Primary transform parameter. Used as literal value, fallback literal, prefix/suffix, search text, factor, or offset depending on TransformKind."))
	FString ParameterA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform", meta = (ToolTip = "Secondary transform parameter. Used by Replace Text as replacement text."))
	FString ParameterB;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaFieldMapping_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping", meta = (GetOptions = "GetSourceFieldPathOptions"))
	FString SourcePath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping", meta = (GetOptions = "GetTargetFieldPathOptions"))
	FString TargetPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping")
	bool bRequired = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping|Transform")
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaSourceValueEntry_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	FString Path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	FString ExportedValue;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaSourceSnapshot_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	FString SourceLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	TArray<FGorgeousDataSchemaSourceValueEntry_S> ValueEntries;

	bool TryGetValueByPath(const FString& InPath, FString& OutValue) const;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMigrationItemResult_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString SourceLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString CreatedAssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSucceeded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 EvaluatedFieldCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 ChangedFieldCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> Errors;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMigrationResult_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSucceeded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 CreatedAssetCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FGorgeousDataSchemaMigrationItemResult_S> ItemResults;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FString> GlobalErrors;
};
