// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "IAssetTypeActions.h"
#include "AssetTypeCategories.h"
#include "Styling/SlateStyle.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousAssetRegistrationStructures.h"
//<-------------------------------------------------------------------------->

namespace GorgeousAssetRegistration
{
	// Shorthand constant for the asset category used for all Gorgeous assets. Registered in FGorgeousCoreEditorModule::GorgeousStartupModule().
	static EAssetTypeCategories::Type GGorgeousThingsCategory = EAssetTypeCategories::Misc;
	
	// The shared pointer to the asset type actions that will hold all registered asset type actions for Gorgeous custom assets, to ensure they can be unregistered properly on module shutdown.
	static TArray<TSharedPtr<IAssetTypeActions>> GRegisteredAssetTypeActions;
	
	/**
	 * Helper function to create a metadata structure for a custom asset type, which can be used to register the asset type in the Unreal Editor.
	 *
	 * @param AssetCategories The asset category or categories that this asset type belongs to, used for organizational purposes in the content browser.
	 * @param Style The style set from which to retrieve the icon and thumbnail brushes for this asset type.
	 * @param DisplayName The localized display name of the asset type.
	 * @param SupportedClass The UClass that this asset type supports.
	 * @param TypeColor The color used to visually represent this asset type in the content browser.
	 * @param SubMenus Sub-Category names that this asset type belongs to, which can be used for organizational purposes in the content browser.
	 * @param IconKey The key used to look up the icon for this asset type in the style set. The function will look for both "GorgeousCore.{IconKey}.Icon" and "GorgeousCore.{IconKey}.Thumbnail" brushes in the style set.
	 *
	 * @return A populated FGorgeousAssetTypeActionInfo_S structure with the provided metadata and retrieved brushes.
	 */
	FORCEINLINE FGorgeousAssetTypeActionInfo_S MakeInfo(const EAssetTypeCategories::Type AssetCategories, const TSharedPtr<FSlateStyleSet>& Style, const FText& DisplayName, UClass* SupportedClass, const FColor& TypeColor, TArray<FText> SubMenus,
	                                                    const FName& IconKey)
	{
		FGorgeousAssetTypeActionInfo_S Info;
		Info.DisplayName = DisplayName;
		Info.SupportedClass = SupportedClass;
		Info.TypeColor = TypeColor;
		Info.Categories = AssetCategories;
		Info.SubMenus = MoveTemp(SubMenus);
		if (Style.IsValid() && !IconKey.IsNone())
		{
			Info.ThumbnailBrush = Style->GetBrush(*FString::Printf(TEXT("GorgeousCore.%s.Thumbnail"), *IconKey.ToString()));
			Info.IconBrush = Style->GetBrush(*FString::Printf(TEXT("GorgeousCore.%s.Icon"), *IconKey.ToString()));
		}
		else
		{
			Info.ThumbnailBrush = nullptr;
			Info.IconBrush = nullptr;
		}
		return Info;
	};
	
	/**
	 * Helper function to create a metadata structure for a custom asset type, which can be used to register the asset type in the Unreal Editor, with direct brushes for icon and thumbnail.
	 *
	 * @param AssetCategories The asset category or categories that this asset type belongs to, used for organizational purposes in the content browser.
	 * @param DisplayName The localized display name of the asset type.
	 * @param SupportedClass The UClass that this asset type supports.
	 * @param TypeColor The color used to visually represent this asset type in the content browser.
	 * @param SubMenus Sub-Category names that this asset type belongs to, which can be used for organizational purposes in the content browser.
	 * @param IconBrush The FSlateBrush used as the icon for this asset type in the content browser.
	 * @param ThumbnailBrush The FSlateBrush used as the thumbnail for this asset type in the content browser.
	 *
	 * @return A populated FGorgeousAssetTypeActionInfo_S structure with the provided metadata and brushes.
	 */
	FORCEINLINE FGorgeousAssetTypeActionInfo_S MakeInfoWithBrushes(const EAssetTypeCategories::Type AssetCategories, const FText& DisplayName, UClass* SupportedClass, const FColor& TypeColor, TArray<FText> SubMenus,
	                                                               const FSlateBrush* IconBrush, const FSlateBrush* ThumbnailBrush)
	{
		FGorgeousAssetTypeActionInfo_S Info;
		Info.DisplayName = DisplayName;
		Info.SupportedClass = SupportedClass;
		Info.TypeColor = TypeColor;
		Info.Categories = AssetCategories;
		Info.SubMenus = MoveTemp(SubMenus);
		Info.IconBrush = IconBrush;
		Info.ThumbnailBrush = ThumbnailBrush;
		return Info;
	};
}