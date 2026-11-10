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
#include "AssetRegistration/GorgeousAssetRegistrationHelpers.h"
#include "AssetRegistration/GorgeousAssetTypeAction.h"
//<-------------------------------------------------------------------------->

/**
 * Macro to register a new asset category in the Unreal Editor, which will be used to group all custom asset types related to Gorgeous Things under a common category in the content browser.
 *
 * @param CategoryName The name of the asset category to register.
 * @param LocalizedDisplayName The localized display name of the asset category.
 */
#define GORGEOUS_REGISTER_ASSET_CATEGORY(CategoryName, LocalizedDisplayName) \
{ \
	GorgeousAssetRegistration::GGorgeousThingsCategory = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAdvancedAssetCategory(FName(CategoryName), FText::FromString(LocalizedDisplayName)); \
}

/**
 * Macro to register any asset type action through the shared Gorgeous registration tracking.
 *
 * @param AssetTypeActionExpr An expression that resolves to TSharedRef<IAssetTypeActions>.
 */
#define REGISTER_GORGEOUS_ASSET_TYPE_ACTION(AssetTypeActionExpr) \
{ \
	TSharedRef<IAssetTypeActions> AssetTypeAction = AssetTypeActionExpr; \
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(AssetTypeAction); \
	GorgeousAssetRegistration::TrackAssetTypeAction(FName(TEXT(UE_MODULE_NAME)), AssetTypeAction); \
}

/**
 * Macro to register a custom asset type in the Unreal Editor, using the provided metadata structure.
 *
 * @param AssetTypeInfo A structure containing metadata about the asset type, such as its display name, supported class, type color, and icon information.
 */
#define REGISTER_GORGEOUS_ASSET(AssetTypeInfo) \
{ \
	const FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = AssetTypeInfo; \
	REGISTER_GORGEOUS_ASSET_TYPE_ACTION(GorgeousAssetRegistration::MakeAssetTypeAction(AssetTypeActionInfo)); \
}

/**
 * Macro to create a metadata structure for a custom asset type, which can be used to register the asset type in the Unreal Editor.
 *
 * @param DisplayName The localized display name of the asset type.
 * @param SupportedClass The UClass that this asset type supports.
 * @param TypeColor The color used to visually represent this asset type in the content browser.
 * @param IconKey The key used to look up the icon for this asset type in the style set.
 * @param ... Sub-Category names that this asset type belongs to, which can be used for organizational purposes in the content browser.
 */
#define GORGEOUS_MAKE_INFO(DisplayName, SupportedClass, TypeColor, IconKey, ...) \
	GorgeousAssetRegistration::MakeInfo(GorgeousAssetRegistration::GGorgeousThingsCategory, GorgeousStyleRegistration::GGorgeousStyleSet, DisplayName, SupportedClass, TypeColor, { __VA_ARGS__ }, IconKey)

/**
 * Macro to create a metadata structure for a custom asset type, which can be used to register the asset type in the Unreal Editor, with direct brushes for icon and thumbnail.
 *
 * @param DisplayName The localized display name of the asset type.
 * @param SupportedClass The UClass that this asset type supports.
 * @param TypeColor The color used to visually represent this asset type in the content browser.
 * @param IconBrush The FSlateBrush used as the icon for this asset type in the content browser.
 * @param ThumbnailBrush The FSlateBrush used as the thumbnail for this asset type in the content browser.
 * @param ... Sub-Category names that this asset type belongs to, which can be used for organizational purposes in the content browser.
 */
#define GORGEOUS_MAKE_INFO_WITH_BRUSHES(DisplayName, SupportedClass, TypeColor, IconBrush, ThumbnailBrush, ...) \
	GorgeousAssetRegistration::MakeInfoWithBrushes(GorgeousAssetRegistration::GGorgeousThingsCategory, DisplayName, SupportedClass, TypeColor, { __VA_ARGS__ }, IconBrush, ThumbnailBrush)

/**
 * Macro to create metadata for a custom asset type with a custom editor opener.
 */
#define GORGEOUS_MAKE_INFO_WITH_EDITOR(DisplayName, SupportedClass, TypeColor, IconKey, OpenEditor, ...) \
	GorgeousAssetRegistration::MakeInfoWithEditor(GorgeousAssetRegistration::GGorgeousThingsCategory, GorgeousStyleRegistration::GGorgeousStyleSet, DisplayName, SupportedClass, TypeColor, { __VA_ARGS__ }, IconKey, OpenEditor)

/**
 * Macro to create metadata for a custom asset type with direct brushes and a custom editor opener.
 */
#define GORGEOUS_MAKE_INFO_WITH_BRUSHES_AND_EDITOR(DisplayName, SupportedClass, TypeColor, IconBrush, ThumbnailBrush, OpenEditor, ...) \
	GorgeousAssetRegistration::MakeInfoWithBrushesAndEditor(GorgeousAssetRegistration::GGorgeousThingsCategory, DisplayName, SupportedClass, TypeColor, { __VA_ARGS__ }, IconBrush, ThumbnailBrush, OpenEditor)

/**
 * Macro to unregister a previously registered custom asset type from the Unreal Editor.
 *
 * @param AssetTypeAction The shared reference to the IAssetTypeActions instance that was used to register the asset type, which will be unregistered and removed from the list of registered asset type actions.
 */
#define UNREGISTER_GORGEOUS_ASSET(AssetTypeAction) \
    GorgeousAssetRegistration::UnregisterAssetTypeAction(AssetTypeAction);

/** Unregister only the invoking module's actions, including registrations from other translation units. */
#define UNREGISTER_GORGEOUS_ASSETS \
    GorgeousAssetRegistration::UnregisterAssetTypeActions(FName(TEXT(UE_MODULE_NAME)));
