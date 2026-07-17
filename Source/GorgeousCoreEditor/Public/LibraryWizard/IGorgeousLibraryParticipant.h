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
#include "LibraryWizard/GorgeousLibraryTypes.h"
#include "LibraryWizard/GorgeousSystemTemplate_DA.h"
#include "Styling/SlateBrush.h"
#include "Brushes/SlateImageBrush.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
//<-------------------------------------------------------------------------->

/*
<=====--- Class Info ---=====>
<-------- Quick Info -------->
| Display Name: Gorgeous Library Participant Interface
| Functional Name: IGorgeousLibraryParticipant
| Parent Class: (none - pure C++ interface)
| Author: Nils Bergemann
<---------------------------->
<------- Description ------->
| Pure C++ interface that Gorgeous plugin modules implement to expose their
| content to the Gorgeous Library editor tab.
|
| Registration pattern (mirrors IGorgeousInsightMatrixProvider):
|   - The implementing module stores an instance of its concrete participant.
|   - During GorgeousStartupModule() it assigns:
|       LibraryParticipant = &MyConcreteParticipant;
|     on the IGorgeousThingsModuleInterface base.
|   - SGorgeousLibraryView discovers participants by calling
|       UGorgeousPluginHelper::Get()->GetAllRegisteredModules()
|     and filtering for non-null GetLibraryParticipant() pointers.
|
| Blueprint compatibility:
|   Templates derived from UGorgeousSystemTemplate_DA are BlueprintType +
|   Blueprintable and therefore discoverable via the Asset Registry without
|   any extra registration by the participant.
<==============================>
*/
class GORGEOUSCOREEDITOR_API IGorgeousLibraryParticipant
{
public:

	virtual ~IGorgeousLibraryParticipant() = default;

	//<=====--- C++ Only ---=====>

	/**
	 * Returns the stable internal key for this participant.
	 * Typically the plugin name (e.g. "GorgeousInventory").
	 */
	virtual FName GetParticipantName() const = 0;

	/**
	 * Returns the human-readable display name shown in the Library sidebar.
	 * Defaults to FText::FromName(GetParticipantName()).
	 */
	virtual FText GetParticipantDisplayName() const
	{
		return FText::FromName(GetParticipantName());
	}

	/**
	 * Optional icon name hint for the sidebar entry.
	 * Resolved against the participant plugin's registered style set.
	 * Return NAME_None to show no icon.
	 */
	virtual FName GetParticipantIcon() const { return NAME_None; }

	/**
	 * Returns all library categories this participant contributes.
	 * Each category becomes a labelled section in the Library right panel,
	 * populated by Asset Registry queries using the category's AssetClassPath.
	 */
	virtual TArray<FGorgeousLibraryCategoryDescriptor> GetLibraryCategories() const = 0;

	/**
	 * Called when an asset in one of this participant's categories is double-clicked.
	 */
	virtual void OnAssetDoubleClicked(const FAssetData& Asset, const FGorgeousLibraryCategoryDescriptor& Category) {}

	/**
	 * Called to populate the right-click context menu for selected assets.
	 */
	virtual void OnBuildContextMenu(class FMenuBuilder& MenuBuilder, const TArray<FAssetData>& SelectedAssets, const FGorgeousLibraryCategoryDescriptor& Category) {}

	/**
	 * Returns a custom icon for the given asset.
	 * If nullptr is returned, a default asset icon is used.
	 */
	virtual TSharedPtr<struct FSlateBrush> GetAssetIcon(const FAssetData& Asset)
	{
		if (UGorgeousSystemTemplate_DA* Template = Cast<UGorgeousSystemTemplate_DA>(Asset.GetAsset()))
		{
			if (Template->TemplateIcon)
			{
				const FVector2D IconSize(64.0f, 64.0f);
				return MakeShareable(new FSlateImageBrush(Template->TemplateIcon.Get(), IconSize));
			}
		}
		return nullptr;
	}

	/**
	 * Returns the display name for the given asset (used in Grid and List views).
	 * Default implementation falls back to the asset's object name.
	 */
	virtual FText GetAssetDisplayName(const FAssetData& Asset) { return FText::FromName(Asset.AssetName); }

	/**
	 * Returns a human-readable description for the given asset (used in List view).
	 */
	virtual FText GetAssetDescription(const FAssetData& Asset) { return FText::GetEmpty(); }

	/**
	 * Returns all available filters for the given category.
	 * Filters are displayed in a premium "Pill" bar if bEnableAdvancedFiltering is true.
	 */
	virtual TArray<FGorgeousLibraryFilter> GetFilters(FName CategoryId) const { return {}; }

	/**
	 * Called when a custom category action (e.g. "+" or "Create") is clicked in the section header.
	 */
	virtual void OnCategoryActionExecuted(FName CategoryId, FName ActionId) {}
	//<-------------------------->
};