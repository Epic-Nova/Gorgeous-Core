// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
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