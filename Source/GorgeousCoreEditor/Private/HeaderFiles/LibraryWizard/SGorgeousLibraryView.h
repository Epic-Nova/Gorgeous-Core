// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|             Gorgeous Core - Library Wizard View Widget                     |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
<=========================================================================*/

#pragma once

//<=====--- Includes ---=====>
//<----- Engine Includes ----->
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Views/STableRow.h"
#include "Filters/SFilterBar.h"
//<----- Module Includes ----->
#include "LibraryWizard/GorgeousLibraryTypes.h"
//<-------------------------->

//<=====--- Forward Declarations ---=====>
class IGorgeousLibraryParticipant;
//<--------------------------------------->

/**
 * The main Gorgeous Library editor tab widget.
 *
 * Layout:
 *   [Left panel, ~220px]      [Right panel, remainder]
 *   +--------------------+     +------------------------------------+
 *   | Gorgeous Inventory |     | Gorgeous Inventory                 |
 *   | Gorgeous Skills    |     | ─── Inventory Templates ──────     |
 *   | ...                |     | [Tile] [Tile] ...                  |
 *   |                    |     | ─── Inventory Items ──────         |
 *   |                    |     | [🔍 Search...          ]           |
 *   |                    |     | [Tile] [Tile] ...                  |
 *   +--------------------+     +------------------------------------+
 *
 * Participants are discovered by iterating all registered module interfaces
 * from UGorgeousPluginHelper and filtering for non-null LibraryParticipant ptrs.
 * Modules without a participant are silently skipped, no warning toast.
 */
class GORGEOUSCOREEDITOR_API SGorgeousLibraryView : public SCompoundWidget
{
public:

	/** Delegate for when a filter tag is toggled. */
	DECLARE_DELEGATE_ThreeParams(FOnFilterTagToggled, bool /*bIsChecked*/, FName /*FilterId*/, bool /*bClearOthers*/);

	SLATE_BEGIN_ARGS(SGorgeousLibraryView) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	//<=====--- Build ---=====>

	/** Builds the full two-panel layout. */
	TSharedRef<SWidget> BuildLayout();

	/** Builds the left-side participant list. */
	TSharedRef<SWidget> BuildParticipantList();

	/**
	 * Builds the right-side detail panel for the currently selected participant.
	 * Returns an empty placeholder if no participant is selected.
	 */
	TSharedRef<SWidget> BuildDetailPanel();

	/** Builds one category section (header + optional search bar + tile list). */
	TSharedRef<SWidget> BuildCategorySection(const FGorgeousLibraryCategoryDescriptor& Category, const TArray<TSharedPtr<FAssetData>>& Assets);
	//<-------------------------->

	//<=====--- Participant List Callbacks ---=====>

	/** Called when the user clicks a participant row in the left panel. */
	void OnParticipantSelected(TSharedPtr<IGorgeousLibraryParticipant> Participant);

	/** Generates a single row widget for the participant list. */
	TSharedRef<ITableRow> OnGenerateParticipantRow(TSharedPtr<IGorgeousLibraryParticipant> Participant, const TSharedRef<STableViewBase>& OwnerTable);

	/** Called when the participant context menu opens. */
	TSharedPtr<SWidget> OnParticipantContextMenuOpening();
	//<-------------------------->

	//<=====--- Asset View Callbacks ---=====>

	/** Generates an asset tile widget for a tile view (Grid mode). */
	TSharedRef<ITableRow> OnGenerateAssetTile(TSharedPtr<FAssetData> InAsset, const TSharedRef<STableViewBase>& OwnerTable);

	/** Generates an asset row widget for a list view (List mode). */
	TSharedRef<ITableRow> OnGenerateAssetRow(TSharedPtr<FAssetData> InAsset, const TSharedRef<STableViewBase>& OwnerTable);

	/** Called when the user double-clicks an asset. */
	void OnAssetDoubleClicked(TSharedPtr<FAssetData> InAsset, FName CategoryId);

	/** Called when a context menu is requested for one or more assets. */
	TSharedPtr<SWidget> OnContextMenuOpening(FName CategoryId);
	//<-------------------------->

	//<=====--- Filtering & View Commands ---=====>
	
	/** Called when the Filter Bar state changes. */
	void OnFilterChanged(FName CategoryId);

	/** Called when a category action button (e.g. "Create") is clicked. */
	FReply OnCategoryActionClicked(FName CategoryId, FName ActionId);

	/** Called when the 'Filter' button is clicked to toggle the filter bar. */
	FReply OnToggleFilterBar(FName CategoryId);

	/** Returns the visibility of the filter bar for a category. */
	EVisibility GetFilterBarVisibility(FName CategoryId) const;
	//<-------------------------->

	//<=====--- Search ---=====>

	/** Called when the user types in a category's search box. */
	void OnSearchTextChanged(const FText& NewText, FName CategoryId);

	/** Legacy handler for templates that still use the wizard. */
	void OnTemplateTileDoubleClicked(TSharedPtr<FAssetData> InAsset);

	/** Refreshes all item assets for the current participant from the Asset Registry. */
	void RefreshAssetsForParticipant(TSharedPtr<IGorgeousLibraryParticipant> Participant);
	//<-------------------------->

	//<=====--- State ---=====>

	/** List of all registered participants discovered at startup. */
	TArray<TSharedPtr<IGorgeousLibraryParticipant>> AllParticipants;

	/** The participant currently selected in the sidebar. */
	TSharedPtr<IGorgeousLibraryParticipant> SelectedParticipant;

	/** The participant sidebar list. */
	TSharedPtr<SListView<TSharedPtr<IGorgeousLibraryParticipant>>> ParticipantListView;

	/**
	 * Per-category cached assets.
	 * Key = CategoryId, Value = all assets matching that category's AssetClassPath.
	 */
	TMap<FName, TArray<TSharedPtr<FAssetData>>> CategoryAssets;

	/**
	 * Per-category filtered assets (after applying search text).
	 * Key = CategoryId, Value = filtered subset of CategoryAssets[CategoryId].
	 */
	TMap<FName, TArray<TSharedPtr<FAssetData>>> FilteredCategoryAssets;

	/** Per-category search strings (cleared when participant changes). */
	TMap<FName, FString> CategorySearchText;

	/** Per-category filter bar widgets. */
	TMap<FName, TSharedPtr<SBasicFilterBar<TSharedPtr<FAssetData>>>> CategoryFilterBars;

	/** Per-category active filter collections. */
	TMap<FName, TSharedPtr<TFilterCollection<TSharedPtr<FAssetData>>>> CategoryFilterCollections;

	/** Per-category filter bar visibility state. */
	TMap<FName, bool> CategoryFilterBarOpen;

	/** Per-category tile view widgets (Grid mode). */
	TMap<FName, TSharedPtr<STileView<TSharedPtr<FAssetData>>>> CategoryTileViews;

	/** Per-category list view widgets (List mode). */
	TMap<FName, TSharedPtr<SListView<TSharedPtr<FAssetData>>>> CategoryListViews;

	/** Outer container for swapping the right-panel content. */
	TSharedPtr<SBox> RightPanelContainer;
};
