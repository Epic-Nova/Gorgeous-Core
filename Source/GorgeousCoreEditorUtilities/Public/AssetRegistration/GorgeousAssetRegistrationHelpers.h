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
#include "Styling/SlateStyle.h"
#include "GorgeousAssetRegistrationStructures.h"
#include "GorgeousAssetTypeAction.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "IAssetTypeActions.h"
#include "AssetTypeCategories.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
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
	 * @param IconKey The key used to look up the icon for this asset type in the style set. The function will look for both "Gorgeous.{IconKey}.Icon" and "Gorgeous.{IconKey}.Thumbnail" brushes in the style set.
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
			Info.ThumbnailBrush = Style->GetBrush(*FString::Printf(TEXT("Gorgeous.%s.Thumbnail"), *IconKey.ToString()));
			Info.IconBrush = Style->GetBrush(*FString::Printf(TEXT("Gorgeous.%s.Icon"), *IconKey.ToString()));
		}
		else
		{
			Info.ThumbnailBrush = nullptr;
			Info.IconBrush = nullptr;
		}
		return Info;
	};

	/**
	 * Helper function to create a metadata structure with a custom editor opener.
	 */
	FORCEINLINE FGorgeousAssetTypeActionInfo_S MakeInfoWithEditor(const EAssetTypeCategories::Type AssetCategories, const TSharedPtr<FSlateStyleSet>& Style, const FText& DisplayName, UClass* SupportedClass, const FColor& TypeColor,
		TArray<FText> SubMenus, const FName& IconKey, TFunction<void(const TArray<UObject*>&, TSharedPtr<IToolkitHost>)> OpenEditor)
	{
		FGorgeousAssetTypeActionInfo_S Info = MakeInfo(AssetCategories, Style, DisplayName, SupportedClass, TypeColor, MoveTemp(SubMenus), IconKey);
		Info.OpenAssetEditor = MoveTemp(OpenEditor);
		return Info;
	}

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

	/**
	 * Helper function to create a metadata structure with direct brushes and a custom editor opener.
	 */
	FORCEINLINE FGorgeousAssetTypeActionInfo_S MakeInfoWithBrushesAndEditor(const EAssetTypeCategories::Type AssetCategories, const FText& DisplayName, UClass* SupportedClass, const FColor& TypeColor, TArray<FText> SubMenus,
		const FSlateBrush* IconBrush, const FSlateBrush* ThumbnailBrush, TFunction<void(const TArray<UObject*>&, TSharedPtr<IToolkitHost>)> OpenEditor)
	{
		FGorgeousAssetTypeActionInfo_S Info = MakeInfoWithBrushes(AssetCategories, DisplayName, SupportedClass, TypeColor, MoveTemp(SubMenus), IconBrush, ThumbnailBrush);
		Info.OpenAssetEditor = MoveTemp(OpenEditor);
		return Info;
	}

	/**
	 * Creates the correct asset type action based on the supported class and requested action type.
	 */
	FORCEINLINE TSharedRef<IAssetTypeActions> MakeAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& Info)
	{
		EGorgeousAssetActionType ResolvedType = Info.ActionType;
		UClass* SupportedClass = Info.SupportedClass.Get();
		if (ResolvedType == EGorgeousAssetActionType::Auto)
		{
			if (SupportedClass && SupportedClass->IsChildOf(UBlueprint::StaticClass()))
			{
				ResolvedType = EGorgeousAssetActionType::Blueprint;
			}
			else if (SupportedClass && SupportedClass->IsChildOf(UDataAsset::StaticClass()))
			{
				ResolvedType = EGorgeousAssetActionType::DataAsset;
			}
			else
			{
				ResolvedType = EGorgeousAssetActionType::Generic;
			}
		}

		switch (ResolvedType)
		{
		case EGorgeousAssetActionType::Blueprint:
			return MakeShared<FGorgeousAssetTypeAction>(Info);
		case EGorgeousAssetActionType::DataAsset:
			return MakeShared<FGorgeousDataAssetTypeAction>(Info);
		case EGorgeousAssetActionType::Generic:
		default:
			return MakeShared<FGorgeousGenericAssetTypeAction>(Info);
		}
	}
}
