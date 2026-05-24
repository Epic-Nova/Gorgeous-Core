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
#include "AssetRegistration/GorgeousAssetTypeAction.h"

//=============================================================================
// FGorgeousAssetTypeAction Implementation
//=============================================================================

FGorgeousAssetTypeAction::FGorgeousAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& InAssetTypeActionInfo)
	: AssetTypeActionInfos(InAssetTypeActionInfo) {}

FGorgeousAssetTypeAction::~FGorgeousAssetTypeAction() {}

FText FGorgeousAssetTypeAction::GetName() const
{
	return AssetTypeActionInfos.DisplayName;
}

UClass* FGorgeousAssetTypeAction::GetSupportedClass() const
{
	return AssetTypeActionInfos.SupportedClass.Get();
}

FColor FGorgeousAssetTypeAction::GetTypeColor() const
{
	return AssetTypeActionInfos.TypeColor;
}

uint32 FGorgeousAssetTypeAction::GetCategories()
{
	return AssetTypeActionInfos.Categories;
}

const TArray<FText>& FGorgeousAssetTypeAction::GetSubMenus() const
{
	return AssetTypeActionInfos.SubMenus;
}

const FSlateBrush* FGorgeousAssetTypeAction::GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return AssetTypeActionInfos.ThumbnailBrush;
}

const FSlateBrush* FGorgeousAssetTypeAction::GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return AssetTypeActionInfos.IconBrush;
}

void FGorgeousAssetTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	if (AssetTypeActionInfos.OpenAssetEditor)
	{
		AssetTypeActionInfos.OpenAssetEditor(InObjects, EditWithinLevelEditor);
		return;
	}

	FAssetTypeActions_Blueprint::OpenAssetEditor(InObjects, EditWithinLevelEditor);
}

//=============================================================================
// FGorgeousDataAssetTypeAction Implementation
//=============================================================================

FGorgeousDataAssetTypeAction::FGorgeousDataAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& InAssetTypeActionInfo)
	: AssetTypeActionInfos(InAssetTypeActionInfo) {}

FGorgeousDataAssetTypeAction::~FGorgeousDataAssetTypeAction() {}

FText FGorgeousDataAssetTypeAction::GetName() const
{
	return AssetTypeActionInfos.DisplayName;
}

UClass* FGorgeousDataAssetTypeAction::GetSupportedClass() const
{
	return AssetTypeActionInfos.SupportedClass.Get();
}

FColor FGorgeousDataAssetTypeAction::GetTypeColor() const
{
	return AssetTypeActionInfos.TypeColor;
}

uint32 FGorgeousDataAssetTypeAction::GetCategories()
{
	return AssetTypeActionInfos.Categories;
}

const TArray<FText>& FGorgeousDataAssetTypeAction::GetSubMenus() const
{
	return AssetTypeActionInfos.SubMenus;
}

const FSlateBrush* FGorgeousDataAssetTypeAction::GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return AssetTypeActionInfos.ThumbnailBrush;
}

const FSlateBrush* FGorgeousDataAssetTypeAction::GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return AssetTypeActionInfos.IconBrush;
}

void FGorgeousDataAssetTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	if (AssetTypeActionInfos.OpenAssetEditor)
	{
		AssetTypeActionInfos.OpenAssetEditor(InObjects, EditWithinLevelEditor);
		return;
	}

	FAssetTypeActions_DataAsset::OpenAssetEditor(InObjects, EditWithinLevelEditor);
}

//=============================================================================
// FGorgeousGenericAssetTypeAction Implementation
//=============================================================================

FGorgeousGenericAssetTypeAction::FGorgeousGenericAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& InAssetTypeActionInfo)
	: AssetTypeActionInfos(InAssetTypeActionInfo) {}

FGorgeousGenericAssetTypeAction::~FGorgeousGenericAssetTypeAction() {}

FText FGorgeousGenericAssetTypeAction::GetName() const
{
	return AssetTypeActionInfos.DisplayName;
}

UClass* FGorgeousGenericAssetTypeAction::GetSupportedClass() const
{
	return AssetTypeActionInfos.SupportedClass.Get();
}

FColor FGorgeousGenericAssetTypeAction::GetTypeColor() const
{
	return AssetTypeActionInfos.TypeColor;
}

uint32 FGorgeousGenericAssetTypeAction::GetCategories()
{
	return AssetTypeActionInfos.Categories;
}

const TArray<FText>& FGorgeousGenericAssetTypeAction::GetSubMenus() const
{
	return AssetTypeActionInfos.SubMenus;
}

const FSlateBrush* FGorgeousGenericAssetTypeAction::GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return AssetTypeActionInfos.ThumbnailBrush;
}

const FSlateBrush* FGorgeousGenericAssetTypeAction::GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return AssetTypeActionInfos.IconBrush;
}

void FGorgeousGenericAssetTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	if (AssetTypeActionInfos.OpenAssetEditor)
	{
		AssetTypeActionInfos.OpenAssetEditor(InObjects, EditWithinLevelEditor);
		return;
	}

	FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
}