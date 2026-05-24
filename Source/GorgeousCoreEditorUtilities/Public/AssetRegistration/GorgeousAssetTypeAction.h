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
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"
#include "AssetTypeActions/AssetTypeActions_DataAsset.h"
#include "Toolkits/IToolkitHost.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousAssetRegistrationStructures.h"
//<-------------------------------------------------------------------------->

/**
 * Custom asset type action class that defines how a specific asset type should be represented and behave in the Unreal Editor, for Gorgeous Core custom assets.
 * 
 * @author Nils Bergemann
 */
class GORGEOUSCOREEDITORUTILITIES_API FGorgeousAssetTypeAction final : public FAssetTypeActions_Blueprint
{
public:

	/**
	 * Constructor for the GorgeousAssetTypeAction class that accepts a metadata structure.
	 *
	 * @param InAssetTypeActionInfo Metadata describing how the asset type should behave in the editor.
	 */
	explicit FGorgeousAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& InAssetTypeActionInfo);

	/** Destructor. */
	virtual ~FGorgeousAssetTypeAction() override;
	
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

	/**
	 * Opens the asset editor, optionally using a custom editor handler.
	 */
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override;
		
	//<-------------------------------------------------------------------------->

	
	//<============================--- Variables ---============================>
private:

	/**
	 * Metadata container describing all relevant editor-facing characteristics
	 * of this asset type. Set during construction and used by override functions.
	 */
	FGorgeousAssetTypeActionInfo_S AssetTypeActionInfos;
	//<------------------------------------------------------------------------->
};

/**
 * Asset type action class for Gorgeous data assets.
 */
class GORGEOUSCOREEDITORUTILITIES_API FGorgeousDataAssetTypeAction final : public FAssetTypeActions_DataAsset
{
public:
	explicit FGorgeousDataAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& InAssetTypeActionInfo);
	virtual ~FGorgeousDataAssetTypeAction() override;

	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual const FSlateBrush* GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const override;
	virtual const FSlateBrush* GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override;

private:
	FGorgeousAssetTypeActionInfo_S AssetTypeActionInfos;
};

/**
 * Generic asset type action class for non-blueprint, non-data asset types.
 */
class GORGEOUSCOREEDITORUTILITIES_API FGorgeousGenericAssetTypeAction final : public FAssetTypeActions_Base
{
public:
	explicit FGorgeousGenericAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& InAssetTypeActionInfo);
	virtual ~FGorgeousGenericAssetTypeAction() override;

	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual const FSlateBrush* GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const override;
	virtual const FSlateBrush* GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override;

private:
	FGorgeousAssetTypeActionInfo_S AssetTypeActionInfos;
};