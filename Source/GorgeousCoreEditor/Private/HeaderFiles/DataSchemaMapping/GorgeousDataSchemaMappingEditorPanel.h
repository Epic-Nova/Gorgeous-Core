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
#include "SCheckBoxList.h"
#include "Input/Reply.h"
#include "Types/SlateEnums.h"
#include "UObject/Object.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
struct FPropertyChangedEvent;
enum class EGorgeousDataSchemaTransformKind_E : uint8;
class FProperty;
class IDetailsView;
class SBox;
class SButton;
class SExpandableArea;
class SSplitter;
class SVerticalBox;
class UGorgeousDataSchemaMapping_DA;
class UStruct;
//<-------------------------------------------------->

/**
 * Main Slate panel for schema map authoring with side-by-side source/target catalogs.
 */
class SGorgeousDataSchemaMappingEditorPanel final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousDataSchemaMappingEditorPanel) {}
		SLATE_ARGUMENT(UGorgeousDataSchemaMapping_DA*, SchemaMapAsset)
		SLATE_ARGUMENT(TSharedPtr<IDetailsView>, DetailsView)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void RefreshPathCatalogs();
	void RebuildMappingRows();
	void RebuildMappingDiagnostics();
	TSharedRef<SWidget> BuildVariableSelectorWidget(int32 MappingIndex, bool bSourcePath);
	TSharedRef<SWidget> BuildVariablePickerMenu(int32 MappingIndex, bool bSourcePath);
	FText GetMappingVariablePathText(int32 MappingIndex, bool bSourcePath) const;
	FText GetMappingVariableTypeText(int32 MappingIndex, bool bSourcePath) const;
	FLinearColor GetMappingVariableTypeTint(int32 MappingIndex, bool bSourcePath) const;
	FReply HandleRemoveMappingRow(int32 MappingIndex);
	void SetMappingVariablePath(int32 MappingIndex, bool bSourcePath, const FString& NewPath);

	TSharedRef<SWidget> BuildTransformKindMenu(int32 MappingIndex);
	FText GetTransformKindText(int32 MappingIndex) const;
	void SetTransformKind(int32 MappingIndex, EGorgeousDataSchemaTransformKind_E NewKind);
	ECheckBoxState GetMappingRequiredState(int32 MappingIndex) const;
	void SetMappingRequiredState(int32 MappingIndex, ECheckBoxState NewState);
	FText GetTransformParameterAText(int32 MappingIndex) const;
	FText GetTransformParameterBText(int32 MappingIndex) const;
	void HandleTransformParameterACommitted(int32 MappingIndex, const FText& NewText, ETextCommit::Type CommitType);
	void HandleTransformParameterBCommitted(int32 MappingIndex, const FText& NewText, ETextCommit::Type CommitType);
	FText GetTransformParameterALabel(int32 MappingIndex) const;
	FText GetTransformParameterBLabel(int32 MappingIndex) const;
	bool IsTransformParameterAEnabled(int32 MappingIndex) const;
	bool IsTransformParameterBEnabled(int32 MappingIndex) const;
	EVisibility GetTransformParameterAVisibility(int32 MappingIndex) const;
	EVisibility GetTransformParameterBVisibility(int32 MappingIndex) const;
	EVisibility GetTransformParameterSectionVisibility(int32 MappingIndex) const;

	TSharedRef<SWidget> BuildCatalogListWidget(const TArray<FString>& Paths, const UStruct* RootStruct, const FText& EmptyText) const;
	TSharedRef<SWidget> BuildReadOnlyCatalogVariableRow(const FString& VariablePath, const UStruct* RootStruct) const;
	FText GetCatalogVariableTypeText(const FString& VariablePath, const UStruct* RootStruct) const;
	FLinearColor GetCatalogVariableTypeTint(const FString& VariablePath, const UStruct* RootStruct) const;

	void RefreshSchemaDetailViews(const UStruct* SourceRootStruct, const UStruct* TargetRootStruct);
	void CollectStructPropertyPaths(const UStruct* InStructType, const FString& Prefix, int32 Depth, TSet<const UStruct*>& VisitedStructs, TArray<FString>& OutPaths) const;
	void MarkSchemaMapModified() const;
	void HandleDetailsPropertiesChanged(const FPropertyChangedEvent& PropertyChangedEvent);
	const UStruct* ResolveSourceRootStruct() const;
	const UStruct* ResolveTargetRootStruct() const;
	bool TryResolvePropertyPath(const UStruct* RootStruct, const FString& PropertyPath, const FProperty*& OutProperty, FString& OutError) const;
	bool ArePropertiesCompatible(const FProperty* SourceProperty, const FProperty* TargetProperty) const;
	FString GetPropertyTypeLabel(const FProperty* Property) const;
	bool TryFindAutoMatchedTargetPath(const FString& SourcePath, FString& OutTargetPath) const;
	FText GetMappingDiagnosticsText() const;
	EVisibility GetMappingDiagnosticsVisibility() const;

	FText GetTargetRootPathText() const;
	void HandleTargetRootPathCommitted(const FText& NewText, ETextCommit::Type CommitType);
	void SetTargetRootPath(const FString& NewPath);
	FReply HandleBrowseTargetRootPathClicked();
	void HandleTargetRootPathPicked(const FString& PickedPath);
	void HandleSchemaSettingsExpansionChanged(bool bExpanded);
	void HandleSchemaSettingsSlotResized(float NewSize);
	float GetSchemaSettingsSlotSize() const;
	float GetSchemaSettingsMinExpandedSlotSize() const;
	float GetSchemaSettingsCollapsedSlotSize() const;
	FText GetSourceRowsDiagnosticText() const;
	EVisibility GetSourceRowsDiagnosticVisibility() const;

	FReply HandleAddEmptyMappingRow();
	FReply HandleAddSelectedMappingRow();

private:
	static constexpr int32 MaxPropertyTraversalDepth = 4;

	UGorgeousDataSchemaMapping_DA* SchemaMapAsset = nullptr;
	TArray<FString> CachedSourcePaths;
	TArray<FString> CachedTargetPaths;
	TArray<FText> MappingDiagnostics;

	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SVerticalBox> MappingRowsContainer;
	TSharedPtr<SBox> SourceCatalogContainer;
	TSharedPtr<SBox> TargetCatalogContainer;
	TSharedPtr<SButton> BrowseTargetRootPathButton;
	TSharedPtr<SSplitter> MainVerticalSplitter;
	TSharedPtr<SExpandableArea> SchemaSettingsArea;
	float SchemaSettingsExpandedSize = 0.25f;
	bool bSchemaSettingsExpanded = false;
};
