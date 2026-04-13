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
#include "DataSchemaMapping/GorgeousDataSchemaMappingAssetTypeActions.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorToolkit.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Styling/AppStyle.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousDataSchemaMappingAssetTypeActions"

FGorgeousDataSchemaMappingAssetTypeActions::FGorgeousDataSchemaMappingAssetTypeActions(uint32 InAssetCategories, TArray<FText> InSubMenus)
	: AssetCategories(InAssetCategories)
	, SubMenus(MoveTemp(InSubMenus))
{
}

FText FGorgeousDataSchemaMappingAssetTypeActions::GetName() const
{
	return LOCTEXT("DataSchemaMapAssetTypeName", "Gorgeous Data Schema Mapping");
}

FColor FGorgeousDataSchemaMappingAssetTypeActions::GetTypeColor() const
{
	return FColor(214, 134, 52);
}

UClass* FGorgeousDataSchemaMappingAssetTypeActions::GetSupportedClass() const
{
	return UGorgeousDataSchemaMapping_DA::StaticClass();
}

uint32 FGorgeousDataSchemaMappingAssetTypeActions::GetCategories()
{
	return AssetCategories;
}

const TArray<FText>& FGorgeousDataSchemaMappingAssetTypeActions::GetSubMenus() const
{
	return SubMenus;
}

const FSlateBrush* FGorgeousDataSchemaMappingAssetTypeActions::GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return FAppStyle::Get().GetBrush(TEXT("ClassIcon.DataTable"));
}

const FSlateBrush* FGorgeousDataSchemaMappingAssetTypeActions::GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.DataTable"));
}

void FGorgeousDataSchemaMappingAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type ToolkitMode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		if (UGorgeousDataSchemaMapping_DA* SchemaMap = Cast<UGorgeousDataSchemaMapping_DA>(Object))
		{
			const TSharedRef<FGorgeousDataSchemaMappingEditorToolkit> NewEditor = MakeShared<FGorgeousDataSchemaMappingEditorToolkit>();
			NewEditor->Initialize(ToolkitMode, EditWithinLevelEditor, SchemaMap);
		}
	}
}

#undef LOCTEXT_NAMESPACE
