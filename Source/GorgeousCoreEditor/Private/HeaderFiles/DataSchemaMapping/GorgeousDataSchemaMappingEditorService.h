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
#include "Internationalization/Text.h"
#include "AssetRegistry/AssetData.h"
#include "Containers/UnrealString.h"
//<-------------------------------------------------------------------------->

class UGorgeousDataSchemaMapping_DA;
//<-------------------------------------------------->

/**
 * Editor-only helper that discovers migration maps and executes schema migrations.
 */
class FGorgeousDataSchemaMappingEditorService
{
public:
	static TArray<TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>> FindMatchingMigrationMaps(const TArray<FAssetData>& SelectedAssets);
	static void PreviewMigration(const UGorgeousDataSchemaMapping_DA* SchemaMap, const TArray<FAssetData>& SelectedAssets, FString* OutReportPath = nullptr);
	static void ExecuteMigration(const UGorgeousDataSchemaMapping_DA* SchemaMap, const TArray<FAssetData>& SelectedAssets);
	static bool CanExecuteBulkMigrationFromSchemaMap(const UGorgeousDataSchemaMapping_DA* SchemaMap, FText* OutFailureReason = nullptr);
	static void ExecuteBulkMigrationFromSchemaMap(const UGorgeousDataSchemaMapping_DA* SchemaMap);
	static bool CanOpenDryRunSummaryFromSchemaMap(const UGorgeousDataSchemaMapping_DA* SchemaMap, FText* OutFailureReason = nullptr);
	static bool ExecuteDryRunSummaryFromSchemaMap(const UGorgeousDataSchemaMapping_DA* SchemaMap, FString* OutReportPath = nullptr, FText* OutFailureReason = nullptr);
};