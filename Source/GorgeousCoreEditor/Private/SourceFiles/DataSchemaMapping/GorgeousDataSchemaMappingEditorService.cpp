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
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorService.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMapper.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
#include "Helpers/GorgeousPropertyPathHelper.h"
#include "Helpers/GorgeousLoggingHelper.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HAL/FileManager.h"
#include "IContentBrowserSingleton.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ObjectTools.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousDataSchemaMappingEditorService"

namespace
{
	bool IsMapCompatibleWithAsset(const UGorgeousDataSchemaMapping_DA* SchemaMap, const FAssetData& SelectedAsset)
	{
		if (!SchemaMap)
		{
			return false;
		}

		UObject* LoadedObject = SelectedAsset.GetAsset();
		if (!LoadedObject)
		{
			return false;
		}

		bool bSourceKindCompatible = false;
		switch (SchemaMap->SourceDefinition.SourceKind)
		{
		case EGorgeousDataSchemaSourceKind_E::Source_DataTableRows:
			bSourceKindCompatible = Cast<UDataTable>(LoadedObject) != nullptr;
			break;
		case EGorgeousDataSchemaSourceKind_E::Source_DataAsset:
			bSourceKindCompatible = LoadedObject->IsA(UDataAsset::StaticClass());
			break;
		case EGorgeousDataSchemaSourceKind_E::Source_BlueprintClassDefaults:
			bSourceKindCompatible = Cast<UBlueprint>(LoadedObject) != nullptr;
			break;
		case EGorgeousDataSchemaSourceKind_E::Source_AnyUObject:
		default:
			bSourceKindCompatible = true;
			break;
		}

		if (!bSourceKindCompatible)
		{
			return false;
		}

		if (SchemaMap->SourceDefinition.SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows
			&& !SchemaMap->SourceDefinition.RequiredDataTableRowStruct.IsNull())
		{
			const UDataTable* DataTable = Cast<UDataTable>(LoadedObject);
			UScriptStruct* RequiredRowStruct = SchemaMap->SourceDefinition.RequiredDataTableRowStruct.LoadSynchronous();
			if (!DataTable || !RequiredRowStruct)
			{
				return false;
			}

			const UScriptStruct* RowStruct = DataTable->GetRowStruct();
			if (!RowStruct || !RowStruct->IsChildOf(RequiredRowStruct))
			{
				return false;
			}

			return true;
		}

		if (SchemaMap->SourceDefinition.SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows)
		{
			return true;
		}

		if (SchemaMap->SourceDefinition.SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataAsset)
		{
			UClass* RequiredDataAssetClass = SchemaMap->SourceDefinition.RequiredSourceDataAssetClass.LoadSynchronous();
			if (!RequiredDataAssetClass)
			{
				return true;
			}

			return LoadedObject->GetClass()->IsChildOf(RequiredDataAssetClass);
		}

		UClass* RequiredSourceClass = SchemaMap->SourceDefinition.RequiredSourceClass.LoadSynchronous();
		if (!RequiredSourceClass)
		{
			return true;
		}

		if (const UBlueprint* Blueprint = Cast<UBlueprint>(LoadedObject))
		{
			return Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(RequiredSourceClass);
		}

		return LoadedObject->GetClass()->IsChildOf(RequiredSourceClass);
	}

	void CapturePropertyValueEntries(const FProperty* Property, const void* ContainerPtr, const FString& Prefix, UObject* OwnerObject, TArray<FGorgeousDataSchemaSourceValueEntry_S>& OutEntries)
	{
		if (!Property || !ContainerPtr)
		{
			return;
		}

		const FString PropertySegment = GorgeousPropertyPath::GetPreferredPropertySegment(Property);
		const FString PropertyName = Prefix.IsEmpty() ? PropertySegment : FString::Printf(TEXT("%s.%s"), *Prefix, *PropertySegment);
		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ContainerPtr);
		if (!ValuePtr)
		{
			return;
		}

		FGorgeousDataSchemaSourceValueEntry_S Entry;
		Entry.Path = PropertyName;
		Property->ExportTextItem_Direct(Entry.ExportedValue, ValuePtr, nullptr, OwnerObject, PPF_None);
		OutEntries.Add(MoveTemp(Entry));

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			for (TFieldIterator<FProperty> ChildIt(StructProperty->Struct, EFieldIteratorFlags::IncludeSuper); ChildIt; ++ChildIt)
			{
				const FProperty* ChildProperty = *ChildIt;
				if (!ChildProperty || ChildProperty->HasAnyPropertyFlags(CPF_Transient))
				{
					continue;
				}

				CapturePropertyValueEntries(ChildProperty, ValuePtr, PropertyName, OwnerObject, OutEntries);
			}
		}
	}

	void CaptureObjectSnapshot(const UObject* SourceObject, const FString& SourceLabel, FGorgeousDataSchemaSourceSnapshot_S& OutSnapshot)
	{
		OutSnapshot.SourceLabel = SourceLabel;
		for (TFieldIterator<FProperty> PropertyIt(SourceObject->GetClass(), EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			const FProperty* Property = *PropertyIt;
			if (!Property || Property->HasAnyPropertyFlags(CPF_Transient))
			{
				continue;
			}

			CapturePropertyValueEntries(Property, SourceObject, FString(), const_cast<UObject*>(SourceObject), OutSnapshot.ValueEntries);
		}
	}

	void CaptureStructSnapshot(const UScriptStruct* RowStruct, const void* RowData, const FString& SourceLabel, FGorgeousDataSchemaSourceSnapshot_S& OutSnapshot)
	{
		OutSnapshot.SourceLabel = SourceLabel;
		for (TFieldIterator<FProperty> PropertyIt(RowStruct, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			const FProperty* Property = *PropertyIt;
			if (!Property || Property->HasAnyPropertyFlags(CPF_Transient))
			{
				continue;
			}

			CapturePropertyValueEntries(Property, RowData, FString(), nullptr, OutSnapshot.ValueEntries);
		}
	}

	void ExtractSnapshotsForAsset(
		const FAssetData& SelectedAsset,
		const UGorgeousDataSchemaMapping_DA* SchemaMap,
		TArray<FGorgeousDataSchemaSourceSnapshot_S>& OutSnapshots,
		TArray<FString>& OutErrors)
	{
		UObject* LoadedObject = SelectedAsset.GetAsset();
		if (!LoadedObject)
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to load selected asset '%s'."), *SelectedAsset.AssetName.ToString()));
			return;
		}

		if (!IsMapCompatibleWithAsset(SchemaMap, SelectedAsset))
		{
			OutErrors.Add(FString::Printf(
				TEXT("Asset '%s' is not compatible with schema map '%s'."),
				*SelectedAsset.AssetName.ToString(),
				*SchemaMap->GetName()));
			return;
		}

		if (const UDataTable* DataTable = Cast<UDataTable>(LoadedObject))
		{
			const UScriptStruct* RowStruct = DataTable->GetRowStruct();
			if (!RowStruct)
			{
				OutErrors.Add(FString::Printf(TEXT("DataTable '%s' has no row struct."), *DataTable->GetName()));
				return;
			}

			for (const TPair<FName, uint8*>& RowPair : DataTable->GetRowMap())
			{
				FGorgeousDataSchemaSourceSnapshot_S Snapshot;
				CaptureStructSnapshot(RowStruct, RowPair.Value, FString::Printf(TEXT("%s_%s"), *DataTable->GetName(), *RowPair.Key.ToString()), Snapshot);
				OutSnapshots.Add(MoveTemp(Snapshot));
			}

			return;
		}

		if (const UBlueprint* Blueprint = Cast<UBlueprint>(LoadedObject))
		{
			if (!Blueprint->GeneratedClass)
			{
				OutErrors.Add(FString::Printf(TEXT("Blueprint '%s' has no generated class."), *Blueprint->GetName()));
				return;
			}

			const UObject* ClassDefaultObject = Blueprint->GeneratedClass->GetDefaultObject();
			if (!ClassDefaultObject)
			{
				OutErrors.Add(FString::Printf(TEXT("Blueprint '%s' has no class default object."), *Blueprint->GetName()));
				return;
			}

			FGorgeousDataSchemaSourceSnapshot_S Snapshot;
			CaptureObjectSnapshot(ClassDefaultObject, Blueprint->GetName(), Snapshot);
			OutSnapshots.Add(MoveTemp(Snapshot));
			return;
		}

		FGorgeousDataSchemaSourceSnapshot_S Snapshot;
		CaptureObjectSnapshot(LoadedObject, LoadedObject->GetName(), Snapshot);
		OutSnapshots.Add(MoveTemp(Snapshot));
	}

	FString ResolveTargetAssetName(const FGorgeousDataSchemaSourceSnapshot_S& Snapshot, const FString& AssetNameSourcePath)
	{
		if (!AssetNameSourcePath.IsEmpty())
		{
			FString SourceValue;
			if (Snapshot.TryGetValueByPath(AssetNameSourcePath, SourceValue))
			{
				FString TrimmedValue = SourceValue.TrimStartAndEnd();
				TrimmedValue.RemoveFromStart(TEXT("\""));
				TrimmedValue.RemoveFromEnd(TEXT("\""));
				if (!TrimmedValue.IsEmpty())
				{
					return ObjectTools::SanitizeObjectName(TrimmedValue);
				}
			}
		}

		return ObjectTools::SanitizeObjectName(Snapshot.SourceLabel);
	}

	FString ResolveTargetDestinationPath(const UGorgeousDataSchemaMapping_DA* SchemaMap, const FString& RootPath, const FGorgeousDataSchemaSourceSnapshot_S& Snapshot)
	{
		if (!SchemaMap || !SchemaMap->MigrationDefinition.bCreateFolderPerSourceItem)
		{
			return RootPath;
		}

		const FString FolderName = ObjectTools::SanitizeObjectName(Snapshot.SourceLabel);
		if (FolderName.IsEmpty())
		{
			return RootPath;
		}

		return FString::Printf(TEXT("%s/%s"), *RootPath, *FolderName);
	}

	struct FGorgeousDataSchemaPreviewDiffLine_S
	{
		FString SourceLabel;
		FString SourcePath;
		FString TargetPath;
		FString ValueBefore;
		FString ValueAfter;
	};

	FString SanitizeForSingleLine(const FString& Input)
	{
		FString Output = Input;
		Output.ReplaceInline(TEXT("\r"), TEXT(" "));
		Output.ReplaceInline(TEXT("\n"), TEXT(" "));
		Output.ReplaceInline(TEXT("\t"), TEXT(" "));
		Output.ReplaceInline(TEXT("|"), TEXT("/"));
		return Output;
	}

	FString GetHistoryFilePath()
	{
		const FString HistoryDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GorgeousDataSchemaMapping"));
		IFileManager::Get().MakeDirectory(*HistoryDirectory, true);
		return FPaths::Combine(HistoryDirectory, TEXT("MigrationHistory.log"));
	}

	int32 CountFailedItems(const FGorgeousDataSchemaMigrationResult_S& MigrationResult)
	{
		return MigrationResult.ItemResults.FilterByPredicate([](const FGorgeousDataSchemaMigrationItemResult_S& ItemResult)
		{
			return !ItemResult.bSucceeded;
		}).Num();
	}

	int32 SumChangedFields(const FGorgeousDataSchemaMigrationResult_S& MigrationResult)
	{
		int32 ChangedFieldCount = 0;
		for (const FGorgeousDataSchemaMigrationItemResult_S& ItemResult : MigrationResult.ItemResults)
		{
			ChangedFieldCount += ItemResult.ChangedFieldCount;
		}
		return ChangedFieldCount;
	}

	void AppendMigrationHistoryLine(
		const FString& RunType,
		const UGorgeousDataSchemaMapping_DA* SchemaMap,
		const TArray<FAssetData>& SelectedAssets,
		const FGorgeousDataSchemaMigrationResult_S& MigrationResult,
		const FString& OptionalReportPath = FString())
	{
		const FString Timestamp = FDateTime::UtcNow().ToString(TEXT("%Y-%m-%dT%H:%M:%SZ"));
		const int32 FailedItemCount = CountFailedItems(MigrationResult);
		const int32 ChangedFieldCount = SumChangedFields(MigrationResult);

		FString HistoryLine = FString::Printf(
			TEXT("[%s] RunType=%s | Schema=%s | SelectedAssets=%d | Items=%d | Created=%d | Failed=%d | ChangedFields=%d | Success=%s"),
			*Timestamp,
			*SanitizeForSingleLine(RunType),
			*SanitizeForSingleLine(SchemaMap ? SchemaMap->GetPathName() : TEXT("<null>")),
			SelectedAssets.Num(),
			MigrationResult.ItemResults.Num(),
			MigrationResult.CreatedAssetCount,
			FailedItemCount,
			ChangedFieldCount,
			MigrationResult.bSucceeded ? TEXT("true") : TEXT("false"));

		if (!OptionalReportPath.IsEmpty())
		{
			HistoryLine += FString::Printf(TEXT(" | Report=%s"), *SanitizeForSingleLine(OptionalReportPath));
		}

		if (!MigrationResult.GlobalErrors.IsEmpty())
		{
			HistoryLine += FString::Printf(TEXT(" | GlobalErrors=%d"), MigrationResult.GlobalErrors.Num());
		}

		HistoryLine += LINE_TERMINATOR;
		FFileHelper::SaveStringToFile(HistoryLine, *GetHistoryFilePath(), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}

	bool ValidateDirectSchemaBulkMigration(
		const UGorgeousDataSchemaMapping_DA* SchemaMap,
		FText* OutFailureReason,
		UDataTable** OutResolvedDataTable = nullptr)
	{
		auto SetFailure = [OutFailureReason](const FText& FailureReason)
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FailureReason;
			}
		};

		if (OutResolvedDataTable)
		{
			*OutResolvedDataTable = nullptr;
		}

		if (!SchemaMap)
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_NoSchema", "Schema map is invalid."));
			return false;
		}

		if (SchemaMap->SourceDefinition.SourceKind != EGorgeousDataSchemaSourceKind_E::Source_DataTableRows)
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_SourceKind", "Direct bulk migration is available only for Data Table Rows source kind."));
			return false;
		}

		if (SchemaMap->SourceDefinition.SourceDataTable.IsNull())
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_NoDataTable", "No DataTable is configured in SourceDefinition.SourceDataTable."));
			return false;
		}

		UDataTable* SourceDataTable = SchemaMap->SourceDefinition.SourceDataTable.LoadSynchronous();
		if (!SourceDataTable)
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_DataTableLoadFailed", "Configured DataTable could not be loaded."));
			return false;
		}

		if (SchemaMap->SourceDefinition.RequiredDataTableRowStruct.IsValid())
		{
			UScriptStruct* RequiredRowStruct = SchemaMap->SourceDefinition.RequiredDataTableRowStruct.LoadSynchronous();
			const UScriptStruct* DataTableRowStruct = SourceDataTable->GetRowStruct();
			if (!RequiredRowStruct || !DataTableRowStruct || !DataTableRowStruct->IsChildOf(RequiredRowStruct))
			{
				SetFailure(LOCTEXT("BulkMigrationValidation_RowStructMismatch", "Configured DataTable row struct does not satisfy RequiredDataTableRowStruct."));
				return false;
			}
		}

		UClass* TargetClass = SchemaMap->TargetDefinition.TargetClass.LoadSynchronous();
		if (!TargetClass || TargetClass->HasAnyClassFlags(CLASS_Abstract))
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_TargetClass", "Target class is missing or abstract."));
			return false;
		}

		if (TargetClass == UObject::StaticClass())
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_TargetBaseUObject", "Target class cannot be base UObject."));
			return false;
		}

		if (SchemaMap->GetNormalizedTargetContentRootPath().IsEmpty())
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_TargetRootPath", "TargetContentRootPath is invalid or empty."));
			return false;
		}

		if (SchemaMap->FieldMappings.IsEmpty())
		{
			SetFailure(LOCTEXT("BulkMigrationValidation_NoMappings", "No field mappings are configured."));
			return false;
		}

		if (OutResolvedDataTable)
		{
			*OutResolvedDataTable = SourceDataTable;
		}

		if (OutFailureReason)
		{
			*OutFailureReason = FText::GetEmpty();
		}

		return true;
	}

	struct FGorgeousDataSchemaValidationIssue_S
	{
		FString Severity;
		FString Code;
		FString Message;
		int32 RowIndex = INDEX_NONE;
		FString SourcePath;
		FString TargetPath;
	};

	void AddValidationIssue(
		TArray<FGorgeousDataSchemaValidationIssue_S>& OutIssues,
		const TCHAR* InSeverity,
		const TCHAR* InCode,
		const FString& InMessage,
		const int32 InRowIndex = INDEX_NONE,
		const FString& InSourcePath = FString(),
		const FString& InTargetPath = FString())
	{
		FGorgeousDataSchemaValidationIssue_S& NewIssue = OutIssues.AddDefaulted_GetRef();
		NewIssue.Severity = InSeverity;
		NewIssue.Code = InCode;
		NewIssue.Message = InMessage;
		NewIssue.RowIndex = InRowIndex;
		NewIssue.SourcePath = InSourcePath;
		NewIssue.TargetPath = InTargetPath;
	}

	void BuildSchemaValidationIssues(const UGorgeousDataSchemaMapping_DA* SchemaMap, TArray<FGorgeousDataSchemaValidationIssue_S>& OutIssues)
	{
		OutIssues.Reset();

		if (!SchemaMap)
		{
			AddValidationIssue(OutIssues, TEXT("Error"), TEXT("schema.invalid"), TEXT("Schema map is invalid."));
			return;
		}

		if (SchemaMap->SourceDefinition.SourceKind != EGorgeousDataSchemaSourceKind_E::Source_DataTableRows)
		{
			AddValidationIssue(
				OutIssues,
				TEXT("Error"),
				TEXT("source.kind"),
				TEXT("Direct schema migration actions are available only for Source Kind = Data Table Rows."));
		}

		UDataTable* SourceDataTable = nullptr;
		if (SchemaMap->SourceDefinition.SourceDataTable.IsNull())
		{
			AddValidationIssue(
				OutIssues,
				TEXT("Error"),
				TEXT("source.datatable.missing"),
				TEXT("No DataTable is configured in SourceDefinition.SourceDataTable."));
		}
		else
		{
			SourceDataTable = SchemaMap->SourceDefinition.SourceDataTable.LoadSynchronous();
			if (!SourceDataTable)
			{
				AddValidationIssue(
					OutIssues,
					TEXT("Error"),
					TEXT("source.datatable.load_failed"),
					TEXT("Configured SourceDataTable could not be loaded."));
			}
		}

		if (SourceDataTable && SchemaMap->SourceDefinition.RequiredDataTableRowStruct.IsValid())
		{
			UScriptStruct* RequiredRowStruct = SchemaMap->SourceDefinition.RequiredDataTableRowStruct.LoadSynchronous();
			const UScriptStruct* DataTableRowStruct = SourceDataTable->GetRowStruct();
			if (!RequiredRowStruct || !DataTableRowStruct || !DataTableRowStruct->IsChildOf(RequiredRowStruct))
			{
				AddValidationIssue(
					OutIssues,
					TEXT("Error"),
					TEXT("source.row_struct.mismatch"),
					TEXT("Configured SourceDataTable row struct does not satisfy RequiredDataTableRowStruct."));
			}
		}

		UClass* TargetClass = SchemaMap->TargetDefinition.TargetClass.LoadSynchronous();
		if (!TargetClass || TargetClass->HasAnyClassFlags(CLASS_Abstract))
		{
			AddValidationIssue(
				OutIssues,
				TEXT("Error"),
				TEXT("target.class.invalid"),
				TEXT("Target class is missing or abstract."));
		}
		else if (TargetClass == UObject::StaticClass())
		{
			AddValidationIssue(
				OutIssues,
				TEXT("Error"),
				TEXT("target.class.base_object"),
				TEXT("Target class cannot be base UObject."));
		}

		if (SchemaMap->GetNormalizedTargetContentRootPath().IsEmpty())
		{
			AddValidationIssue(
				OutIssues,
				TEXT("Error"),
				TEXT("target.root_path.invalid"),
				TEXT("TargetContentRootPath is invalid or empty."));
		}

		if (SchemaMap->FieldMappings.IsEmpty())
		{
			AddValidationIssue(
				OutIssues,
				TEXT("Error"),
				TEXT("mapping.empty"),
				TEXT("No field mappings are configured."));
			return;
		}

		TSet<FString> SourcePathSet;
		for (const FString& SourcePathOption : SchemaMap->GetSourceFieldPathOptions())
		{
			SourcePathSet.Add(SourcePathOption.ToLower());
		}

		TSet<FString> TargetPathSet;
		for (const FString& TargetPathOption : SchemaMap->GetTargetFieldPathOptions())
		{
			TargetPathSet.Add(TargetPathOption.ToLower());
		}

		TMap<FString, int32> TargetPathToFirstRow;

		for (int32 MappingIndex = 0; MappingIndex < SchemaMap->FieldMappings.Num(); ++MappingIndex)
		{
			const FGorgeousDataSchemaFieldMapping_S& Mapping = SchemaMap->FieldMappings[MappingIndex];

			if (Mapping.SourcePath.IsEmpty())
			{
				AddValidationIssue(
					OutIssues,
					TEXT("Error"),
					TEXT("mapping.source.empty"),
					TEXT("Mapping row has an empty source path."),
					MappingIndex,
					Mapping.SourcePath,
					Mapping.TargetPath);
			}
			else if (!SourcePathSet.Contains(Mapping.SourcePath.ToLower()))
			{
				AddValidationIssue(
					OutIssues,
					TEXT("Error"),
					TEXT("mapping.source.invalid"),
					FString::Printf(TEXT("Source path '%s' is not valid for the configured source definition."), *Mapping.SourcePath),
					MappingIndex,
					Mapping.SourcePath,
					Mapping.TargetPath);
			}

			if (Mapping.TargetPath.IsEmpty())
			{
				AddValidationIssue(
					OutIssues,
					TEXT("Error"),
					TEXT("mapping.target.empty"),
					TEXT("Mapping row has an empty target path."),
					MappingIndex,
					Mapping.SourcePath,
					Mapping.TargetPath);
			}
			else if (!TargetPathSet.Contains(Mapping.TargetPath.ToLower()))
			{
				AddValidationIssue(
					OutIssues,
					TEXT("Error"),
					TEXT("mapping.target.invalid"),
					FString::Printf(TEXT("Target path '%s' is not valid for the configured target definition."), *Mapping.TargetPath),
					MappingIndex,
					Mapping.SourcePath,
					Mapping.TargetPath);
			}

			if (!Mapping.TargetPath.IsEmpty())
			{
				const FString TargetPathKey = Mapping.TargetPath.ToLower();
				if (const int32* FirstRowIndex = TargetPathToFirstRow.Find(TargetPathKey))
				{
					AddValidationIssue(
						OutIssues,
						TEXT("Error"),
						TEXT("mapping.target.duplicate"),
						FString::Printf(TEXT("Target path '%s' is assigned by multiple rows (%d and %d)."), *Mapping.TargetPath, *FirstRowIndex + 1, MappingIndex + 1),
						MappingIndex,
						Mapping.SourcePath,
						Mapping.TargetPath);
				}
				else
				{
					TargetPathToFirstRow.Add(TargetPathKey, MappingIndex);
				}
			}
		}
	}

	bool HasBlockingValidationIssues(const TArray<FGorgeousDataSchemaValidationIssue_S>& Issues)
	{
		for (const FGorgeousDataSchemaValidationIssue_S& Issue : Issues)
		{
			if (Issue.Severity.Equals(TEXT("Error"), ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	FText GetFirstBlockingIssueAsText(const TArray<FGorgeousDataSchemaValidationIssue_S>& Issues)
	{
		for (const FGorgeousDataSchemaValidationIssue_S& Issue : Issues)
		{
			if (Issue.Severity.Equals(TEXT("Error"), ESearchCase::IgnoreCase))
			{
				return FText::FromString(Issue.Message);
			}
		}

		return FText::GetEmpty();
	}

	bool ResolveStructPropertyPath(
		UStruct* CurrentStruct,
		void* CurrentContainerPtr,
		const TArray<FString>& Segments,
		const int32 SegmentIndex,
		FProperty*& OutLeafProperty,
		void*& OutLeafContainerPtr,
		FString& OutError)
	{
		if (!CurrentStruct || !CurrentContainerPtr)
		{
			OutError = TEXT("Current struct or container pointer is invalid.");
			return false;
		}

		if (!Segments.IsValidIndex(SegmentIndex))
		{
			OutError = TEXT("Segment index exceeded path bounds.");
			return false;
		}

		FProperty* ResolvedProperty = GorgeousPropertyPath::FindPropertyBySegment(CurrentStruct, Segments[SegmentIndex]);
		if (!ResolvedProperty)
		{
			OutError = FString::Printf(TEXT("Property '%s' was not found on struct '%s'."), *Segments[SegmentIndex], *CurrentStruct->GetName());
			return false;
		}

		const bool bIsLeaf = SegmentIndex == Segments.Num() - 1;
		if (bIsLeaf)
		{
			OutLeafProperty = ResolvedProperty;
			OutLeafContainerPtr = CurrentContainerPtr;
			return true;
		}

		if (FStructProperty* StructProperty = CastField<FStructProperty>(ResolvedProperty))
		{
			void* NextContainerPtr = StructProperty->ContainerPtrToValuePtr<void>(CurrentContainerPtr);
			if (!NextContainerPtr)
			{
				OutError = FString::Printf(TEXT("Struct path '%s' had a null intermediate container."), *StructProperty->GetName());
				return false;
			}

			return ResolveStructPropertyPath(StructProperty->Struct, NextContainerPtr, Segments, SegmentIndex + 1, OutLeafProperty, OutLeafContainerPtr, OutError);
		}

		if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(ResolvedProperty))
		{
			UObject* NextObject = ObjectProperty->GetObjectPropertyValue_InContainer(CurrentContainerPtr);
			if (!NextObject)
			{
				OutError = FString::Printf(TEXT("Object path '%s' is null and cannot be traversed."), *ObjectProperty->GetName());
				return false;
			}

			return ResolveStructPropertyPath(NextObject->GetClass(), NextObject, Segments, SegmentIndex + 1, OutLeafProperty, OutLeafContainerPtr, OutError);
		}

		OutError = FString::Printf(TEXT("Property '%s' is not a struct/object bridge but path traversal continues."), *ResolvedProperty->GetName());
		return false;
	}

	bool ResolvePropertyPath(
		UObject* RootObject,
		const FString& PropertyPath,
		FProperty*& OutLeafProperty,
		void*& OutLeafContainerPtr,
		FString& OutError)
	{
		OutLeafProperty = nullptr;
		OutLeafContainerPtr = nullptr;

		if (!RootObject)
		{
			OutError = TEXT("Root object is null.");
			return false;
		}

		TArray<FString> Segments;
		PropertyPath.ParseIntoArray(Segments, TEXT("."), true);
		if (Segments.IsEmpty())
		{
			OutError = TEXT("Property path was empty.");
			return false;
		}

		return ResolveStructPropertyPath(RootObject->GetClass(), RootObject, Segments, 0, OutLeafProperty, OutLeafContainerPtr, OutError);
	}

	bool TryExportResolvedTargetValue(UObject* TargetObject, const FString& TargetPath, FString& OutValue, FString& OutError)
	{
		FProperty* LeafProperty = nullptr;
		void* LeafContainerPtr = nullptr;
		if (!ResolvePropertyPath(TargetObject, TargetPath, LeafProperty, LeafContainerPtr, OutError))
		{
			return false;
		}

		void* LeafValuePtr = LeafProperty->ContainerPtrToValuePtr<void>(LeafContainerPtr);
		if (!LeafValuePtr)
		{
			OutError = FString::Printf(TEXT("Target path '%s' resolved, but value pointer was null."), *TargetPath);
			return false;
		}

		LeafProperty->ExportTextItem_Direct(OutValue, LeafValuePtr, nullptr, TargetObject, PPF_None);
		return true;
	}

	bool TryImportResolvedTargetValue(UObject* TargetObject, const FString& TargetPath, const FString& ValueToImport, FString& OutError)
	{
		FProperty* LeafProperty = nullptr;
		void* LeafContainerPtr = nullptr;
		if (!ResolvePropertyPath(TargetObject, TargetPath, LeafProperty, LeafContainerPtr, OutError))
		{
			return false;
		}

		void* LeafValuePtr = LeafProperty->ContainerPtrToValuePtr<void>(LeafContainerPtr);
		if (!LeafValuePtr)
		{
			OutError = FString::Printf(TEXT("Target path '%s' resolved, but value pointer was null."), *TargetPath);
			return false;
		}

		if (!LeafProperty->ImportText_Direct(*ValueToImport, LeafValuePtr, TargetObject, PPF_None))
		{
			OutError = FString::Printf(TEXT("Failed to import value '%s' into target path '%s'."), *ValueToImport, *TargetPath);
			return false;
		}

		return true;
	}

	void EvaluateSnapshotDryRun(
		const FGorgeousDataSchemaSourceSnapshot_S& Snapshot,
		UObject* TargetObject,
		const UGorgeousDataSchemaMapping_DA* SchemaMap,
		FGorgeousDataSchemaMigrationItemResult_S& InOutItemResult,
		TArray<FGorgeousDataSchemaPreviewDiffLine_S>& OutDiffLines)
	{
		if (!TargetObject || !SchemaMap)
		{
			InOutItemResult.Errors.Add(TEXT("Dry-run target object or schema map was invalid."));
			return;
		}

		for (const FGorgeousDataSchemaFieldMapping_S& FieldMapping : SchemaMap->FieldMappings)
		{
			if (FieldMapping.SourcePath.IsEmpty() || FieldMapping.TargetPath.IsEmpty())
			{
				InOutItemResult.Warnings.Add(TEXT("Skipped a mapping row with an empty source or target path."));
				continue;
			}

			FString SourceValue;
			if (!Snapshot.TryGetValueByPath(FieldMapping.SourcePath, SourceValue))
			{
				if (FieldMapping.bRequired)
				{
					InOutItemResult.Errors.Add(FString::Printf(
						TEXT("Required source path '%s' was not found in snapshot '%s'."),
						*FieldMapping.SourcePath,
						*Snapshot.SourceLabel));
				}
				else
				{
					InOutItemResult.Warnings.Add(FString::Printf(
						TEXT("Optional source path '%s' was not found in snapshot '%s'."),
						*FieldMapping.SourcePath,
						*Snapshot.SourceLabel));
				}
				continue;
			}

			FString TransformedValue;
			FString TransformError;
			if (!FGorgeousDataSchemaMapper::ApplyTransformDefinition(FieldMapping.TransformDefinition, SourceValue, TransformedValue, TransformError))
			{
				InOutItemResult.Errors.Add(FString::Printf(
					TEXT("Transform failed for source '%s' -> target '%s': %s"),
					*FieldMapping.SourcePath,
					*FieldMapping.TargetPath,
					*TransformError));
				continue;
			}

			FString ValueBefore;
			FString ValueBeforeError;
			if (!TryExportResolvedTargetValue(TargetObject, FieldMapping.TargetPath, ValueBefore, ValueBeforeError))
			{
				InOutItemResult.Errors.Add(FString::Printf(
					TEXT("Target path '%s' failed to export before-value: %s"),
					*FieldMapping.TargetPath,
					*ValueBeforeError));
				continue;
			}

			FString ImportError;
			if (!TryImportResolvedTargetValue(TargetObject, FieldMapping.TargetPath, TransformedValue, ImportError))
			{
				InOutItemResult.Errors.Add(FString::Printf(
					TEXT("Target path '%s' failed to import transformed value: %s"),
					*FieldMapping.TargetPath,
					*ImportError));
				continue;
			}

			FString ValueAfter;
			FString ValueAfterError;
			if (!TryExportResolvedTargetValue(TargetObject, FieldMapping.TargetPath, ValueAfter, ValueAfterError))
			{
				InOutItemResult.Errors.Add(FString::Printf(
					TEXT("Target path '%s' failed to export after-value: %s"),
					*FieldMapping.TargetPath,
					*ValueAfterError));
				continue;
			}

			++InOutItemResult.EvaluatedFieldCount;
			if (!ValueBefore.Equals(ValueAfter, ESearchCase::CaseSensitive))
			{
				++InOutItemResult.ChangedFieldCount;
				FGorgeousDataSchemaPreviewDiffLine_S& DiffLine = OutDiffLines.AddDefaulted_GetRef();
				DiffLine.SourceLabel = Snapshot.SourceLabel;
				DiffLine.SourcePath = FieldMapping.SourcePath;
				DiffLine.TargetPath = FieldMapping.TargetPath;
				DiffLine.ValueBefore = ValueBefore;
				DiffLine.ValueAfter = ValueAfter;
			}
		}
	}

	FString WriteDryRunPreviewReport(
		const UGorgeousDataSchemaMapping_DA* SchemaMap,
		const FGorgeousDataSchemaMigrationResult_S& MigrationResult,
		const TArray<FGorgeousDataSchemaPreviewDiffLine_S>& DiffLines)
	{
		const FString PreviewDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GorgeousDataSchemaMapping"), TEXT("Previews"));
		IFileManager::Get().MakeDirectory(*PreviewDirectory, true);

		FString Timestamp = FDateTime::UtcNow().ToString(TEXT("%Y%m%d_%H%M%S"));
		Timestamp.ReplaceInline(TEXT(":"), TEXT("-"));

		const FString FileName = FString::Printf(TEXT("%s_DryRun_%s.txt"), *(SchemaMap ? SchemaMap->GetName() : TEXT("SchemaMap")), *Timestamp);
		const FString ReportPath = FPaths::Combine(PreviewDirectory, FileName);
		const int32 EvaluatedFieldCount = [&MigrationResult]()
		{
			int32 LocalEvaluatedFieldCount = 0;
			for (const FGorgeousDataSchemaMigrationItemResult_S& ItemResult : MigrationResult.ItemResults)
			{
				LocalEvaluatedFieldCount += ItemResult.EvaluatedFieldCount;
			}
			return LocalEvaluatedFieldCount;
		}();
		const int32 ChangedFieldCount = SumChangedFields(MigrationResult);
		const int32 UnchangedFieldCount = EvaluatedFieldCount >= ChangedFieldCount
			? (EvaluatedFieldCount - ChangedFieldCount)
			: 0;

		TArray<FString> Lines;
		Lines.Add(FString::Printf(TEXT("Dry-Run Report | UTC %s"), *FDateTime::UtcNow().ToString(TEXT("%Y-%m-%d %H:%M:%S"))));
		Lines.Add(FString::Printf(TEXT("Schema: %s"), SchemaMap ? *SchemaMap->GetPathName() : TEXT("<null>")));
		Lines.Add(FString::Printf(TEXT("Previewed Items: %d"), MigrationResult.ItemResults.Num()));
		Lines.Add(FString::Printf(TEXT("Failed Items: %d"), CountFailedItems(MigrationResult)));
		Lines.Add(FString::Printf(TEXT("Evaluated Fields: %d"), EvaluatedFieldCount));
		Lines.Add(FString::Printf(TEXT("Changed Fields: %d"), ChangedFieldCount));
		Lines.Add(FString::Printf(TEXT("Unchanged Fields: %d"), UnchangedFieldCount));
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("Legend:"));
		Lines.Add(TEXT("  - Diff Lines include only rows where target values changed."));
		Lines.Add(TEXT("  - Before is the target class default value on a transient preview object before the mapping is applied."));
		Lines.Add(TEXT("  - After is the resulting value after source extraction, transform, and target import."));
		Lines.Add(TEXT("  - Before='' or Before='None' is often a normal default and does not imply a missing source field."));
		Lines.Add(TEXT("  - Real path/type problems appear as warnings/errors in Item Summary and can cause failed items."));
		Lines.Add(TEXT(""));

		if (!MigrationResult.GlobalErrors.IsEmpty())
		{
			Lines.Add(TEXT("Global Errors:"));
			for (const FString& GlobalError : MigrationResult.GlobalErrors)
			{
				Lines.Add(FString::Printf(TEXT("  - %s"), *SanitizeForSingleLine(GlobalError)));
			}
			Lines.Add(TEXT(""));
		}

		Lines.Add(TEXT("Item Summary:"));
		for (const FGorgeousDataSchemaMigrationItemResult_S& ItemResult : MigrationResult.ItemResults)
		{
			Lines.Add(FString::Printf(
				TEXT("  - %s | Success=%s | Evaluated=%d | Changed=%d | Warnings=%d | Errors=%d"),
				*SanitizeForSingleLine(ItemResult.SourceLabel),
				ItemResult.bSucceeded ? TEXT("true") : TEXT("false"),
				ItemResult.EvaluatedFieldCount,
				ItemResult.ChangedFieldCount,
				ItemResult.Warnings.Num(),
				ItemResult.Errors.Num()));
		}

		Lines.Add(TEXT(""));
		Lines.Add(TEXT("Diff Lines:"));
		if (DiffLines.IsEmpty())
		{
			Lines.Add(TEXT("  <No changed target values detected>"));
		}
		else
		{
			for (const FGorgeousDataSchemaPreviewDiffLine_S& DiffLine : DiffLines)
			{
				Lines.Add(FString::Printf(
					TEXT("  - Source='%s' | %s -> %s | Before='%s' | After='%s'"),
					*SanitizeForSingleLine(DiffLine.SourceLabel),
					*SanitizeForSingleLine(DiffLine.SourcePath),
					*SanitizeForSingleLine(DiffLine.TargetPath),
					*SanitizeForSingleLine(DiffLine.ValueBefore),
					*SanitizeForSingleLine(DiffLine.ValueAfter)));
			}
		}

		FFileHelper::SaveStringArrayToFile(Lines, *ReportPath);
		return ReportPath;
	}

	void AddNotification(const FText& MessageText, const EGorgeousLoggingImportance Importance)
	{
		GorgeousLogging::LogMessage_Internal(
			MessageText,
			Importance,
			TEXT("GorgeousDataSchemaMapping"),
			6.0f,
			false,
			true,
			true,
			true,
			nullptr,
			nullptr);
	}
}

TArray<TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>> FGorgeousDataSchemaMappingEditorService::FindMatchingMigrationMaps(const TArray<FAssetData>& SelectedAssets)
{
	TArray<TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>> MatchingMaps;
	if (SelectedAssets.IsEmpty())
	{
		return MatchingMaps;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FAssetData> SchemaMapAssets;
	AssetRegistry.GetAssetsByClass(UGorgeousDataSchemaMapping_DA::StaticClass()->GetClassPathName(), SchemaMapAssets, true);

	for (const FAssetData& SchemaMapAssetData : SchemaMapAssets)
	{
		UGorgeousDataSchemaMapping_DA* SchemaMap = Cast<UGorgeousDataSchemaMapping_DA>(SchemaMapAssetData.GetAsset());
		if (!SchemaMap || !SchemaMap->IsMigrationActionEnabled())
		{
			continue;
		}

		const bool bHasCompatibleSelection = SelectedAssets.ContainsByPredicate([SchemaMap](const FAssetData& SelectedAsset)
		{
			return IsMapCompatibleWithAsset(SchemaMap, SelectedAsset);
		});

		if (bHasCompatibleSelection)
		{
			MatchingMaps.Add(SchemaMap);
		}

	}

	MatchingMaps.Sort([](const TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>& A, const TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>& B)
	{
		const UGorgeousDataSchemaMapping_DA* APtr = A.Get();
		const UGorgeousDataSchemaMapping_DA* BPtr = B.Get();
		if (!APtr || !BPtr)
		{
			return false;
		}
		return APtr->GetName() < BPtr->GetName();
	});

	return MatchingMaps;
}

bool FGorgeousDataSchemaMappingEditorService::CanExecuteBulkMigrationFromSchemaMap(const UGorgeousDataSchemaMapping_DA* SchemaMap, FText* OutFailureReason)
{
	return ValidateDirectSchemaBulkMigration(SchemaMap, OutFailureReason, nullptr);
}

bool FGorgeousDataSchemaMappingEditorService::CanOpenDryRunSummaryFromSchemaMap(const UGorgeousDataSchemaMapping_DA* SchemaMap, FText* OutFailureReason)
{
	TArray<FGorgeousDataSchemaValidationIssue_S> ValidationIssues;
	BuildSchemaValidationIssues(SchemaMap, ValidationIssues);
	const bool bCanOpenDryRunSummary = !HasBlockingValidationIssues(ValidationIssues);

	if (OutFailureReason)
	{
		*OutFailureReason = bCanOpenDryRunSummary
			? FText::GetEmpty()
			: GetFirstBlockingIssueAsText(ValidationIssues);
	}

	return bCanOpenDryRunSummary;
}

bool FGorgeousDataSchemaMappingEditorService::ExecuteDryRunSummaryFromSchemaMap(
	const UGorgeousDataSchemaMapping_DA* SchemaMap,
	FString* OutReportPath,
	FText* OutFailureReason)
{
	if (OutReportPath)
	{
		OutReportPath->Reset();
	}

	FText FailureReason;
	if (!CanOpenDryRunSummaryFromSchemaMap(SchemaMap, &FailureReason))
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FailureReason;
		}

		return false;
	}

	if (!SchemaMap)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = LOCTEXT("DryRunFromSchema_NoSchema", "Schema map is invalid.");
		}

		return false;
	}

	UDataTable* SourceDataTable = SchemaMap->SourceDefinition.SourceDataTable.LoadSynchronous();
	if (!SourceDataTable)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = LOCTEXT("DryRunFromSchema_NoDataTable", "Configured SourceDataTable could not be loaded.");
		}

		return false;
	}

	FString LocalReportPath;
	PreviewMigration(SchemaMap, { FAssetData(SourceDataTable) }, &LocalReportPath);

	if (OutReportPath)
	{
		*OutReportPath = LocalReportPath;
	}

	if (OutFailureReason)
	{
		*OutFailureReason = FText::GetEmpty();
	}

	return !LocalReportPath.IsEmpty();
}

void FGorgeousDataSchemaMappingEditorService::ExecuteBulkMigrationFromSchemaMap(const UGorgeousDataSchemaMapping_DA* SchemaMap)
{
	FText FailureReason;
	UDataTable* SourceDataTable = nullptr;
	if (!ValidateDirectSchemaBulkMigration(SchemaMap, &FailureReason, &SourceDataTable))
	{
		AddNotification(
			FText::Format(LOCTEXT("BulkMigrationExecuteBlocked", "Bulk Migrate is not available: {0}"), FailureReason),
			Logging_Error);
		return;
	}

	if (!SourceDataTable)
	{
		AddNotification(LOCTEXT("BulkMigrationExecuteNoTable", "Bulk Migrate failed: configured DataTable could not be resolved."), Logging_Error);
		return;
	}

	ExecuteMigration(SchemaMap, { FAssetData(SourceDataTable) });
}

void FGorgeousDataSchemaMappingEditorService::PreviewMigration(
	const UGorgeousDataSchemaMapping_DA* SchemaMap,
	const TArray<FAssetData>& SelectedAssets,
	FString* OutReportPath)
{
	if (OutReportPath)
	{
		OutReportPath->Reset();
	}

	if (!SchemaMap)
	{
		AddNotification(LOCTEXT("SchemaPreview_NoMap", "Schema preview failed: map is invalid."), Logging_Error);
		return;
	}

	if (SelectedAssets.IsEmpty())
	{
		AddNotification(LOCTEXT("SchemaPreview_NoSelection", "Schema preview skipped: no assets selected."), Logging_Error);
		return;
	}

	FGorgeousDataSchemaMigrationResult_S MigrationResult;
	TArray<FGorgeousDataSchemaPreviewDiffLine_S> PreviewDiffLines;

	UClass* TargetClass = SchemaMap->TargetDefinition.TargetClass.LoadSynchronous();
	if (!TargetClass || TargetClass->HasAnyClassFlags(CLASS_Abstract))
	{
		MigrationResult.GlobalErrors.Add(TEXT("Target class is missing or abstract."));
	}
	else if (TargetClass == UObject::StaticClass())
	{
		MigrationResult.GlobalErrors.Add(TEXT("Target class cannot be base UObject. Provide a concrete asset class for migration output."));
	}

	const FString TargetRootPath = SchemaMap->GetNormalizedTargetContentRootPath();
	if (TargetRootPath.IsEmpty())
	{
		MigrationResult.GlobalErrors.Add(TEXT("TargetContentRootPath is invalid or empty. Configure a /Game/... destination in the schema map."));
	}

	if (!MigrationResult.GlobalErrors.IsEmpty())
	{
		MigrationResult.bSucceeded = false;
		AppendMigrationHistoryLine(TEXT("DryRun"), SchemaMap, SelectedAssets, MigrationResult);
		AddNotification(LOCTEXT("SchemaPreview_InvalidConfig", "Schema preview failed: mapping configuration is invalid."), Logging_Error);
		return;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	for (const FAssetData& SelectedAsset : SelectedAssets)
	{
		TArray<FGorgeousDataSchemaSourceSnapshot_S> Snapshots;
		TArray<FString> SnapshotErrors;
		ExtractSnapshotsForAsset(SelectedAsset, SchemaMap, Snapshots, SnapshotErrors);

		if (!SnapshotErrors.IsEmpty())
		{
			FGorgeousDataSchemaMigrationItemResult_S ItemResult;
			ItemResult.SourceLabel = SelectedAsset.AssetName.ToString();
			ItemResult.Errors = MoveTemp(SnapshotErrors);
			ItemResult.bSucceeded = false;
			MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
			continue;
		}

		for (const FGorgeousDataSchemaSourceSnapshot_S& Snapshot : Snapshots)
		{
			FGorgeousDataSchemaMigrationItemResult_S ItemResult;
			ItemResult.SourceLabel = Snapshot.SourceLabel;

			const FString BaseAssetName = ResolveTargetAssetName(Snapshot, SchemaMap->TargetDefinition.TargetAssetNameSourcePath);
			const FString ResolvedAssetName = BaseAssetName.IsEmpty() ? TEXT("MigratedAsset") : BaseAssetName;
			const FString DestinationPath = ResolveTargetDestinationPath(SchemaMap, TargetRootPath, Snapshot);
			const FString BasePackageName = FString::Printf(TEXT("%s/%s"), *DestinationPath, *ResolvedAssetName);
			FString UniquePackageName;
			FString UniqueAssetName;
			AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, TEXT(""), UniquePackageName, UniqueAssetName);
			ItemResult.CreatedAssetPath = FString::Printf(TEXT("%s.%s"), *UniquePackageName, *UniqueAssetName);

			UObject* TransientTargetObject = NewObject<UObject>(GetTransientPackage(), TargetClass, NAME_None, RF_Transient);
			if (!TransientTargetObject)
			{
				ItemResult.Errors.Add(TEXT("Failed to create transient target object for preview."));
				ItemResult.bSucceeded = false;
				MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
				continue;
			}

			EvaluateSnapshotDryRun(Snapshot, TransientTargetObject, SchemaMap, ItemResult, PreviewDiffLines);
			ItemResult.bSucceeded = ItemResult.Errors.IsEmpty();
			MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
		}
	}

	const int32 FailedItemCount = CountFailedItems(MigrationResult);
	MigrationResult.bSucceeded = MigrationResult.GlobalErrors.IsEmpty() && FailedItemCount == 0;

	const FString ReportPath = WriteDryRunPreviewReport(SchemaMap, MigrationResult, PreviewDiffLines);
	if (OutReportPath)
	{
		*OutReportPath = ReportPath;
	}

	const FText SummaryText = FText::Format(
		LOCTEXT("SchemaPreviewSummary", "Schema dry-run finished. Previewed {0} item(s), {1} failed item(s), {2} changed field(s)."),
		FText::AsNumber(MigrationResult.ItemResults.Num()),
		FText::AsNumber(FailedItemCount),
		FText::AsNumber(SumChangedFields(MigrationResult)));

	AddNotification(SummaryText, MigrationResult.bSucceeded ? Logging_Success : Logging_Error);
	AddNotification(
		FText::Format(LOCTEXT("SchemaPreviewReportPath", "Dry-run report saved: {0}"), FText::FromString(ReportPath)),
		Logging_Information);

	AppendMigrationHistoryLine(TEXT("DryRun"), SchemaMap, SelectedAssets, MigrationResult, ReportPath);
}

void FGorgeousDataSchemaMappingEditorService::ExecuteMigration(const UGorgeousDataSchemaMapping_DA* SchemaMap, const TArray<FAssetData>& SelectedAssets)
{
	if (!SchemaMap)
	{
		AddNotification(LOCTEXT("SchemaMigration_NoMap", "Schema migration failed: map is invalid."), Logging_Error);
		return;
	}

	if (SelectedAssets.IsEmpty())
	{
		AddNotification(LOCTEXT("SchemaMigration_NoSelection", "Schema migration skipped: no assets selected."), Logging_Error);
		return;
	}

	FGorgeousDataSchemaMigrationResult_S MigrationResult;

	UClass* TargetClass = SchemaMap->TargetDefinition.TargetClass.LoadSynchronous();
	if (!TargetClass || TargetClass->HasAnyClassFlags(CLASS_Abstract))
	{
		MigrationResult.GlobalErrors.Add(TEXT("Target class is missing or abstract."));
	}
	else if (TargetClass == UObject::StaticClass())
	{
		MigrationResult.GlobalErrors.Add(TEXT("Target class cannot be base UObject. Provide a concrete asset class for migration output."));
	}

	const FString TargetRootPath = SchemaMap->GetNormalizedTargetContentRootPath();
	if (TargetRootPath.IsEmpty())
	{
		MigrationResult.GlobalErrors.Add(TEXT("TargetContentRootPath is invalid or empty. Configure a /Game/... destination in the schema map."));
	}

	if (!MigrationResult.GlobalErrors.IsEmpty())
	{
		MigrationResult.bSucceeded = false;
		AppendMigrationHistoryLine(TEXT("Execute"), SchemaMap, SelectedAssets, MigrationResult);
		AddNotification(LOCTEXT("SchemaMigration_InvalidConfig", "Schema migration failed: mapping configuration is invalid."), Logging_Error);
		return;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	for (const FAssetData& SelectedAsset : SelectedAssets)
	{
		TArray<FGorgeousDataSchemaSourceSnapshot_S> Snapshots;
		TArray<FString> SnapshotErrors;
		ExtractSnapshotsForAsset(SelectedAsset, SchemaMap, Snapshots, SnapshotErrors);

		if (!SnapshotErrors.IsEmpty())
		{
			FGorgeousDataSchemaMigrationItemResult_S ItemResult;
			ItemResult.SourceLabel = SelectedAsset.AssetName.ToString();
			ItemResult.Errors = MoveTemp(SnapshotErrors);
			ItemResult.bSucceeded = false;
			MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
			continue;
		}

		for (const FGorgeousDataSchemaSourceSnapshot_S& Snapshot : Snapshots)
		{
			FGorgeousDataSchemaMigrationItemResult_S ItemResult;
			ItemResult.SourceLabel = Snapshot.SourceLabel;

			const FString BaseAssetName = ResolveTargetAssetName(Snapshot, SchemaMap->TargetDefinition.TargetAssetNameSourcePath);
			const FString ResolvedAssetName = BaseAssetName.IsEmpty() ? TEXT("MigratedAsset") : BaseAssetName;
			const FString DestinationPath = ResolveTargetDestinationPath(SchemaMap, TargetRootPath, Snapshot);
			const FString BasePackageName = FString::Printf(TEXT("%s/%s"), *DestinationPath, *ResolvedAssetName);

			FString UniquePackageName;
			FString UniqueAssetName;
			AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, TEXT(""), UniquePackageName, UniqueAssetName);

			UPackage* Package = CreatePackage(*UniquePackageName);
			if (!Package)
			{
				ItemResult.Errors.Add(FString::Printf(TEXT("Failed to create package '%s'."), *UniquePackageName));
				ItemResult.bSucceeded = false;
				MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
				continue;
			}

			UObject* NewAsset = NewObject<UObject>(Package, TargetClass, *UniqueAssetName, RF_Public | RF_Standalone | RF_Transactional);
			if (!NewAsset)
			{
				ItemResult.Errors.Add(FString::Printf(TEXT("Failed to create target asset '%s'."), *UniqueAssetName));
				ItemResult.bSucceeded = false;
				MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
				continue;
			}

			if (!NewAsset->GetClass()->IsChildOf(TargetClass))
			{
				ItemResult.Errors.Add(FString::Printf(
					TEXT("Created asset '%s' did not derive from target class '%s'."),
					*UniqueAssetName,
					*TargetClass->GetName()));
				NewAsset->ClearFlags(RF_Public | RF_Standalone | RF_Transactional);
				NewAsset->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional);
				ItemResult.bSucceeded = false;
				MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
				continue;
			}

			const bool bApplied = FGorgeousDataSchemaMapper::ApplySnapshotToObject(Snapshot, NewAsset, SchemaMap, ItemResult.Warnings, ItemResult.Errors);
			ItemResult.bSucceeded = bApplied && ItemResult.Errors.IsEmpty();

			if (ItemResult.bSucceeded)
			{
				AssetRegistryModule.AssetCreated(NewAsset);
				NewAsset->MarkPackageDirty();
				Package->MarkPackageDirty();
				ItemResult.CreatedAssetPath = FString::Printf(TEXT("%s.%s"), *UniquePackageName, *UniqueAssetName);
				++MigrationResult.CreatedAssetCount;
			}
			else
			{
				ItemResult.Errors.Add(TEXT("Migration output was discarded because field mapping did not apply successfully."));
				NewAsset->ClearFlags(RF_Public | RF_Standalone | RF_Transactional);
				NewAsset->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional);
			}

			MigrationResult.ItemResults.Add(MoveTemp(ItemResult));
		}
	}

	const int32 FailedItemCount = MigrationResult.ItemResults.FilterByPredicate([](const FGorgeousDataSchemaMigrationItemResult_S& ItemResult)
	{
		return !ItemResult.bSucceeded;
	}).Num();

	MigrationResult.bSucceeded = MigrationResult.GlobalErrors.IsEmpty() && FailedItemCount == 0 && MigrationResult.CreatedAssetCount > 0;

	const FText SummaryText = FText::Format(
		LOCTEXT("SchemaMigrationSummary", "Schema migration finished. Created {0} asset(s), {1} failed item(s)."),
		FText::AsNumber(MigrationResult.CreatedAssetCount),
		FText::AsNumber(FailedItemCount));

	AddNotification(SummaryText, MigrationResult.bSucceeded ? Logging_Success : Logging_Error);

	if (MigrationResult.CreatedAssetCount > 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		ContentBrowserModule.Get().SetSelectedPaths({TargetRootPath}, true);
	}

	AppendMigrationHistoryLine(TEXT("Execute"), SchemaMap, SelectedAssets, MigrationResult);
}

#undef LOCTEXT_NAMESPACE
