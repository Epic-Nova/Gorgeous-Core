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
#include "AssetRegistry/AssetData.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

/** Defines how assets in a category are rendered. */
enum class EGorgeousLibraryViewMode
{
	Grid,
	List
};

/** Describes a single filter tag that can be toggled to refine the asset search. */
struct FGorgeousLibraryFilter
{
	/** Stable identifier for this filter. */
	FName FilterId;

	/** Human-readable name shown in the filter bar. */
	FText DisplayName;

	/** Group identifier for this filter (e.g. "Type", "Rarity"). */
	FName FilterCategory;

	/** Optional color for the filter icon. */
	FLinearColor Color = FLinearColor::White;

	/** Optional icon brush name hint (from participant style). */
	FName IconName = NAME_None;

	FGorgeousLibraryFilter() = default;
	FGorgeousLibraryFilter(FName InId, FText InName, FName InCategory = NAME_None, FLinearColor InColor = FLinearColor::White, FName InIcon = NAME_None)
		: FilterId(InId), DisplayName(InName), FilterCategory(InCategory), Color(InColor), IconName(InIcon)
	{}
};

/** Describes a custom action button that can be displayed in a category header. */
struct FGorgeousLibraryCategoryAction
{
	/** Stable identifier for this action. */
	FName ActionId;

	/** Human-readable name shown as a tooltip or button text. */
	FText DisplayName;

	/** Detailed tooltip explaining what this action does. */
	FText Tooltip;

	/** Optional icon brush name hint (from participant style). */
	FName IconName = NAME_None;

	FGorgeousLibraryCategoryAction() = default;
	FGorgeousLibraryCategoryAction(FName InId, FText InName, FText InTooltip, FName InIcon = NAME_None)
		: ActionId(InId), DisplayName(InName), Tooltip(InTooltip), IconName(InIcon)
	{}
};

/**
 * Describes a single category that a Gorgeous Library participant exposes.
 *
 * Each category maps to one section in theLibraryWizard right-hand panel.
 * The Library view queries the Asset Registry using AssetClassPath and renders
 * the resulting assets as tiles inside the section.
 *
 * @author Nils Bergemann
 */
struct GORGEOUSCOREEDITOR_API FGorgeousLibraryCategoryDescriptor
{
	/** Internal stable identifier for this category (e.g. "InventoryTemplates"). */
	FName CategoryId;

	/** Human-readable name shown as the section header in the library panel. */
	FText DisplayName;

	/**
	 * The primary asset class to scan for in the Asset Registry.
	 * Example: FTopLevelAssetPath(TEXT("/Script/GorgeousInventoryEditor"), TEXT("GorgeousInventoryTemplate_DA"))
	 */
	FTopLevelAssetPath AssetClassPath;

	/** Preferred rendering mode for this category. */
	EGorgeousLibraryViewMode ViewMode = EGorgeousLibraryViewMode::Grid;

	/**
	 * When true, a SSearchBox is displayed above the tile list in this category.
	 * The search text is matched case-insensitively against the asset name.
	 */
	bool bHasSearchFilter = false;

	/** When true, a toggleable filter bar with custom tags is displayed. */
	bool bEnableAdvancedFiltering = false;

	/**
	 * When true, clicking a tile in this category opens the Gorgeous Setup Wizard
	 * to generate a new asset from the selected template.
	 * When false, clicking does nothing (read-only browse mode).
	 */
	bool bOpensSetupWizard = false;

	/** Custom action buttons to display in the section header. */
	TArray<FGorgeousLibraryCategoryAction> CategoryActions;
};