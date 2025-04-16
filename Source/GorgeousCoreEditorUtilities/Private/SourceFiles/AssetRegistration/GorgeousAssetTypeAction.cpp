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
#include "AssetRegistration/GorgeousAssetTypeAction.h"

//=============================================================================
// FGorgeousAssetTypeAction Implementation
//=============================================================================

FGorgeousAssetTypeAction::FGorgeousAssetTypeAction(const FGorgeousAssetTypeActionInfo_S& NewAssetTypeActionInfos)
{
	AssetTypeActionInfos = NewAssetTypeActionInfos;
}

FText FGorgeousAssetTypeAction::GetName() const
{
	return AssetTypeActionInfos.Name;
}

UClass* FGorgeousAssetTypeAction::GetSupportedClass() const
{
	return AssetTypeActionInfos.SupportedClass;
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

const FSlateBrush* FGorgeousAssetTypeAction::GetThumbnailBrush(const FAssetData& InAssetData,
	const FName InClassName) const
{
	return FAssetTypeActions_Base::GetThumbnailBrush(InAssetData, InClassName);
}

const FSlateBrush* FGorgeousAssetTypeAction::GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return FAssetTypeActions_Base::GetIconBrush(InAssetData, InClassName);
}

