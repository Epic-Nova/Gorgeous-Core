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
//<--------------------------=== Engine Includes ===------------------------->
#include "Toolkits/AssetEditorToolkit.h"
#include "Types/SlateEnums.h"
#include "UObject/GCObject.h"
#include "UObject/ObjectPtr.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class FSpawnTabArgs;
class FToolBarBuilder;
class FExtender;
class IDetailsView;
class SDockTab;
class SGorgeousDataSchemaMappingEditorPanel;
class UGorgeousDataSchemaMapping_DA;
//<-------------------------------------------------->

/**
 * Dedicated editor toolkit for Data Schema Mapping assets.
 */
class FGorgeousDataSchemaMappingEditorToolkit final : public FAssetEditorToolkit, public FGCObject
{
public:
	void Initialize(const EToolkitMode::Type InToolkitMode, const TSharedPtr<IToolkitHost>& InToolkitHost, UGorgeousDataSchemaMapping_DA* InSchemaMapAsset);

	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;

private:
	void ExtendToolbar();
	void FillToolbar(FToolBarBuilder& ToolbarBuilder);
	void HandleBulkMigrateClicked();
	bool CanBulkMigrateFromSchema() const;
	FText GetBulkMigrateTooltipText() const;
	void HandleOpenDryRunSummaryClicked();
	bool CanOpenDryRunSummary() const;
	FText GetOpenDryRunSummaryTooltipText() const;
	EVisibility GetBulkMigrateButtonVisibility() const;

	TSharedRef<SDockTab> SpawnMainTab(const FSpawnTabArgs& SpawnTabArgs);

private:
	static const FName MainTabId;
	TObjectPtr<UGorgeousDataSchemaMapping_DA> EditingAsset = nullptr;
	TSharedPtr<SGorgeousDataSchemaMappingEditorPanel> EditorPanel;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<FExtender> ToolbarExtender;
};
