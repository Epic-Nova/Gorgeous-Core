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
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorToolkit.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorPanel.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorService.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "HAL/PlatformProcess.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousDataSchemaMappingEditorToolkit"

const FName FGorgeousDataSchemaMappingEditorToolkit::MainTabId(TEXT("GorgeousCore.DataSchemaMappingEditor.Main"));

void FGorgeousDataSchemaMappingEditorToolkit::Initialize(const EToolkitMode::Type InToolkitMode, const TSharedPtr<IToolkitHost>& InToolkitHost, UGorgeousDataSchemaMapping_DA* InSchemaMapAsset)
{
	if (!InSchemaMapAsset)
	{
		return;
	}

	EditingAsset = InSchemaMapAsset;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(InSchemaMapAsset);

	EditorPanel = SNew(SGorgeousDataSchemaMappingEditorPanel)
		.SchemaMapAsset(InSchemaMapAsset)
		.DetailsView(DetailsView);

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("GorgeousCore.DataSchemaMappingEditor.Layout.v1")
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split(
				FTabManager::NewStack()
				->AddTab(MainTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)));

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	InitAssetEditor(
		InToolkitMode,
		InToolkitHost,
		FName(TEXT("GorgeousCoreDataSchemaMappingEditor")),
		Layout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		InSchemaMapAsset);

	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

void FGorgeousDataSchemaMappingEditorToolkit::ExtendToolbar()
{
	if (ToolbarExtender.IsValid())
	{
		return;
	}

	ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FGorgeousDataSchemaMappingEditorToolkit::FillToolbar));

	AddToolbarExtender(ToolbarExtender);
}

void FGorgeousDataSchemaMappingEditorToolkit::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.AddWidget(
		SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
				.Visibility(this, &FGorgeousDataSchemaMappingEditorToolkit::GetBulkMigrateButtonVisibility)
				.IsEnabled(this, &FGorgeousDataSchemaMappingEditorToolkit::CanBulkMigrateFromSchema)
				.ToolTipText(this, &FGorgeousDataSchemaMappingEditorToolkit::GetBulkMigrateTooltipText)
				.OnClicked_Lambda([this]()
				{
					HandleBulkMigrateClicked();
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("PlayWorld.PlayInViewport"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BulkMigrateToolbarLabel", "Bulk Migrate"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton")
				.Visibility(this, &FGorgeousDataSchemaMappingEditorToolkit::GetBulkMigrateButtonVisibility)
				.IsEnabled(this, &FGorgeousDataSchemaMappingEditorToolkit::CanOpenDryRunSummary)
				.ToolTipText(this, &FGorgeousDataSchemaMappingEditorToolkit::GetOpenDryRunSummaryTooltipText)
				.OnClicked_Lambda([this]()
				{
					HandleOpenDryRunSummaryClicked();
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.Search"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OpenDryRunSummaryToolbarLabel", "Open Dry Run Summary"))
					]
				]
			],
		FName(TEXT("BulkMigrateToolbarWidget")),
		false,
		HAlign_Fill);
}

void FGorgeousDataSchemaMappingEditorToolkit::HandleBulkMigrateClicked()
{
	if (!EditingAsset)
	{
		return;
	}

	FGorgeousDataSchemaMappingEditorService::ExecuteBulkMigrationFromSchemaMap(EditingAsset);
}

bool FGorgeousDataSchemaMappingEditorToolkit::CanBulkMigrateFromSchema() const
{
	return EditingAsset
		&& FGorgeousDataSchemaMappingEditorService::CanExecuteBulkMigrationFromSchemaMap(EditingAsset, nullptr);
}

FText FGorgeousDataSchemaMappingEditorToolkit::GetBulkMigrateTooltipText() const
{
	if (!EditingAsset)
	{
		return LOCTEXT("BulkMigrateTooltipNoAsset", "Bulk Migrate is unavailable because no schema asset is active.");
	}

	FText FailureReason;
	if (!FGorgeousDataSchemaMappingEditorService::CanExecuteBulkMigrationFromSchemaMap(EditingAsset, &FailureReason))
	{
		return FText::Format(LOCTEXT("BulkMigrateTooltipUnavailableFmt", "Bulk Migrate is unavailable: {0}"), FailureReason);
	}

	return LOCTEXT("BulkMigrateTooltipReady", "Execute migration directly from the configured source DataTable and create target assets under TargetContentRootPath.");
}

void FGorgeousDataSchemaMappingEditorToolkit::HandleOpenDryRunSummaryClicked()
{
	if (!EditingAsset)
	{
		return;
	}

	FString ReportPath;
	FText FailureReason;
	if (!FGorgeousDataSchemaMappingEditorService::ExecuteDryRunSummaryFromSchemaMap(EditingAsset, &ReportPath, &FailureReason))
	{
		return;
	}

	if (!ReportPath.IsEmpty())
	{
		FPlatformProcess::LaunchFileInDefaultExternalApplication(*ReportPath);
	}
}

bool FGorgeousDataSchemaMappingEditorToolkit::CanOpenDryRunSummary() const
{
	return EditingAsset
		&& FGorgeousDataSchemaMappingEditorService::CanOpenDryRunSummaryFromSchemaMap(EditingAsset, nullptr);
}

FText FGorgeousDataSchemaMappingEditorToolkit::GetOpenDryRunSummaryTooltipText() const
{
	if (!EditingAsset)
	{
		return LOCTEXT("OpenDryRunSummaryTooltipNoAsset", "Dry-run summary is unavailable because no schema asset is active.");
	}

	FText FailureReason;
	if (!FGorgeousDataSchemaMappingEditorService::CanOpenDryRunSummaryFromSchemaMap(EditingAsset, &FailureReason))
	{
		return FText::Format(
			LOCTEXT("OpenDryRunSummaryTooltipUnavailableFmt", "Dry-run summary is unavailable: {0}"),
			FailureReason);
	}

	return LOCTEXT("OpenDryRunSummaryTooltipReady", "Run a direct dry-run on the configured source DataTable and open the generated summary report.");
}
EVisibility FGorgeousDataSchemaMappingEditorToolkit::GetBulkMigrateButtonVisibility() const
{
	if (!EditingAsset || !EditingAsset->IsDirectDataTableExecutionEnabled())
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

FName FGorgeousDataSchemaMappingEditorToolkit::GetToolkitFName() const
{
	return FName(TEXT("GorgeousDataSchemaMappingEditor"));
}

FText FGorgeousDataSchemaMappingEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitDisplayName", "Data Schema Mapping");
}

FText FGorgeousDataSchemaMappingEditorToolkit::GetToolkitName() const
{
	if (!EditingAsset)
	{
		return GetBaseToolkitName();
	}

	return FText::Format(
		LOCTEXT("ToolkitNameFmt", "{0} - {1}"),
		GetBaseToolkitName(),
		FText::FromString(EditingAsset->GetName()));
}

FString FGorgeousDataSchemaMappingEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("SchemaMap");
}

FLinearColor FGorgeousDataSchemaMappingEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.15f, 0.35f, 0.65f, 0.5f);
}

void FGorgeousDataSchemaMappingEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceCategory", "Data Schema Mapping"));
	const TSharedRef<FWorkspaceItem> WorkspaceCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
	InTabManager->RegisterTabSpawner(MainTabId, FOnSpawnTab::CreateSP(this, &FGorgeousDataSchemaMappingEditorToolkit::SpawnMainTab))
		.SetDisplayName(LOCTEXT("MainTabTitle", "Schema Map"))
		.SetGroup(WorkspaceCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataAsset"));
}

void FGorgeousDataSchemaMappingEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterTabSpawner(MainTabId);
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

void FGorgeousDataSchemaMappingEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EditingAsset);
}

FString FGorgeousDataSchemaMappingEditorToolkit::GetReferencerName() const
{
	return TEXT("FGorgeousDataSchemaMappingEditorToolkit");
}

TSharedRef<SDockTab> FGorgeousDataSchemaMappingEditorToolkit::SpawnMainTab(const FSpawnTabArgs& SpawnTabArgs)
{
	check(SpawnTabArgs.GetTabId() == MainTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("MainTabLabel", "Schema Mapping"))
		[
			EditorPanel.IsValid()
				? StaticCastSharedRef<SWidget>(EditorPanel.ToSharedRef())
				: SNew(STextBlock).Text(LOCTEXT("PanelMissing", "Schema map editor panel was not created."))
		];
}

#undef LOCTEXT_NAMESPACE
