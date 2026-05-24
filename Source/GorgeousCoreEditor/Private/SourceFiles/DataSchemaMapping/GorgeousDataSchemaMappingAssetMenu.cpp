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
#include "DataSchemaMapping/GorgeousDataSchemaMappingAssetMenu.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorService.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "ContentBrowserMenuContexts.h"
#include "HAL/PlatformProcess.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousDataSchemaMappingAssetMenu"

namespace
{
	FToolMenuOwnerScoped GDataSchemaMappingMenuOwner{TEXT("GorgeousDataSchemaMappingAssetMenu")};

	void AddMigrationEntries(FToolMenuSection& Section, const TArray<FAssetData> SelectedAssets, const TArray<TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>> MatchingMaps)
	{
		for (const TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>& MapPtr : MatchingMaps)
		{
			UGorgeousDataSchemaMapping_DA* SchemaMap = MapPtr.Get();
			if (!SchemaMap)
			{
				continue;
			}

			const TSoftObjectPtr<UGorgeousDataSchemaMapping_DA> SoftSchemaMap(SchemaMap);
			const FText ActionLabel = SchemaMap->GetResolvedMigrationActionLabel();
			const FText ActionTooltip = FText::Format(
				LOCTEXT("ExecuteMigrationTooltipFmt", "Execute migration map '{0}' for the selected assets."),
				FText::FromString(SchemaMap->GetName()));

			Section.AddMenuEntry(
				FName(*FString::Printf(TEXT("GorgeousDataSchemaMigration_%s"), *SchemaMap->GetFName().ToString())),
				ActionLabel,
				ActionTooltip,
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Duplicate"),
				FUIAction(FExecuteAction::CreateLambda([SoftSchemaMap, SelectedAssets]()
				{
					if (UGorgeousDataSchemaMapping_DA* LoadedSchemaMap = SoftSchemaMap.LoadSynchronous())
					{
						FGorgeousDataSchemaMappingEditorService::ExecuteMigration(LoadedSchemaMap, SelectedAssets);
					}
				})));

			Section.AddMenuEntry(
				FName(*FString::Printf(TEXT("GorgeousDataSchemaMigrationPreview_%s"), *SchemaMap->GetFName().ToString())),
				FText::Format(LOCTEXT("PreviewActionLabelFmt", "Preview {0}"), ActionLabel),
				FText::Format(
					LOCTEXT("PreviewMigrationTooltipFmt", "Run a dry-run preview for migration map '{0}' without creating assets and open the generated summary report."),
					FText::FromString(SchemaMap->GetName())),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Info"),
				FUIAction(FExecuteAction::CreateLambda([SoftSchemaMap, SelectedAssets]()
				{
					if (UGorgeousDataSchemaMapping_DA* LoadedSchemaMap = SoftSchemaMap.LoadSynchronous())
					{
						FString ReportPath;
						FGorgeousDataSchemaMappingEditorService::PreviewMigration(LoadedSchemaMap, SelectedAssets, &ReportPath);
						if (!ReportPath.IsEmpty())
						{
							FPlatformProcess::LaunchFileInDefaultExternalApplication(*ReportPath);
						}
					}
				})));
		}
	}

	void AddDynamicMigrationSection(UToolMenu* Menu)
	{
		if (!Menu)
		{
			return;
		}

		const UContentBrowserAssetContextMenuContext* Context = Menu->FindContext<UContentBrowserAssetContextMenuContext>();
		if (!Context || Context->SelectedAssets.IsEmpty())
		{
			return;
		}

		const TArray<TWeakObjectPtr<UGorgeousDataSchemaMapping_DA>> MatchingMaps =
			FGorgeousDataSchemaMappingEditorService::FindMatchingMigrationMaps(Context->SelectedAssets);
		if (MatchingMaps.IsEmpty())
		{
			return;
		}

		FToolMenuSection& Section = Menu->AddSection(
			"GorgeousDataSchemaMappingMigrationSection",
			LOCTEXT("GorgeousMigrationHeading", "Gorgeous Migration"));

		AddMigrationEntries(Section, Context->SelectedAssets, MatchingMaps);
	}
}

void FGorgeousDataSchemaMappingAssetMenu::Register()
{
	if (!UToolMenus::IsToolMenuUIEnabled())
	{
		return;
	}

	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	if (UToolMenu* Menu = ToolMenus->ExtendMenu("ContentBrowser.AssetContextMenu.AssetActionsSubMenu"))
	{
		Menu->AddDynamicSection(
			"GorgeousDataSchemaMappingDynamicSection",
			FNewToolMenuDelegate::CreateStatic(&AddDynamicMigrationSection));
	}
}

void FGorgeousDataSchemaMappingAssetMenu::Unregister()
{
	if (!UToolMenus::IsToolMenuUIEnabled())
	{
		return;
	}

	const UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	ToolMenus->UnregisterOwner(GDataSchemaMappingMenuOwner.GetOwner());
}

#undef LOCTEXT_NAMESPACE
