// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousAssetRegistrationStructures.h"
//<-------------------------------------------------------------------------->

/**
 * Asset type action handler for Gorgeous Core custom assets.
 * 
 * This class defines how a specific asset type appears and behaves within the Unreal Engine Editor.
 * It provides metadata such as display name, color, supported class, categories, and icon/thumbnail
 * brushes used in the content browser. This enhances the user experience when working with custom
 * Gorgeous Core assets by providing clear visual cues and proper categorization.
 *
 * Key features include:
 * - All data-driven via FGorgeousAssetTypeActionInfo_S for flexible reuse.
 *
 * This class is intended to be constructed with a FGorgeousAssetTypeActionInfo_S structure,
 * containing the relevant metadata for the associated asset type.
 *
 * @author Nils Bergemann
 */
class GORGEOUSCOREEDITORUTILITIES_API FGorgeousAssetTypeAction final : public FAssetTypeActions_Base
{
public:

	/**
	 * Constructor for the GorgeousAssetTypeAction class that accepts a metadata structure.
	 *
	 * @param InAssetTypeActionInfo Metadata describing how the asset type should behave in the editor.
	 */
	explicit FGorgeousAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& InAssetTypeActionInfo);
	
	//<============================--- Overrides ---=============================>

	/** 
	 * Returns the display name of this asset type in the editor.
	 *
	 * @return Localized text representing the asset type's name.
	 */
	virtual FText GetName() const override;

	/**
	 * Returns the class this asset type supports.
	 *
	 * @return A pointer to the supported UClass.
	 */
	virtual UClass* GetSupportedClass() const override;

	/**
	 * Returns the color used to visually represent this asset type in the content browser.
	 *
	 * @return An FColor representing the type color.
	 */
	virtual FColor GetTypeColor() const override;

	/**
	 * Returns the categories in which this asset type appears.
	 *
	 * @return Bitmask of EAssetTypeCategories.
	 */
	virtual uint32 GetCategories() override;

	/**
	 * Returns the submenus under which this asset appears in the "Add New" menu.
	 *
	 * @return A constant reference to an array of submenu display names.
	 */
	virtual const TArray<FText>& GetSubMenus() const override;

	/**
	 * Returns the thumbnail brush for the given asset data.
	 * If null, the editor will fall back to the class thumbnail brush.
	 *
	 * @param InAssetData The data for the asset.
	 * @param InClassName The name of the class associated with the asset.
	 * @return Pointer to an FSlateBrush representing the thumbnail.
	 */
	virtual const FSlateBrush* GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const override;

	/**
	 * Returns the icon brush for the given asset data.
	 * If null, the editor will fall back to the class icon brush.
	 *
	 * @param InAssetData The data for the asset.
	 * @param InClassName The name of the class associated with the asset.
	 * @return Pointer to an FSlateBrush representing the icon.
	 */
	virtual const FSlateBrush* GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const override;
		
	//<-------------------------------------------------------------------------->

private:

	/**
	 * Metadata container describing all relevant editor-facing characteristics
	 * of this asset type. Set during construction and used by override functions.
	 */
	FGorgeousAssetTypeActionInfo_S AssetTypeActionInfos;
};