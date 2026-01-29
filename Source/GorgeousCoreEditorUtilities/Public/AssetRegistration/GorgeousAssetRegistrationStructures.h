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

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

/**
 * Metadata describing how a custom asset should appear and behave in the editor.
 * Used by deprecated asset type actions and editor registration helpers.
 */
struct FGorgeousAssetTypeActionInfo_S
{
	FText DisplayName = FText::GetEmpty();
	TWeakObjectPtr<UClass> SupportedClass = nullptr;
	FColor TypeColor = FColor::White;
	uint32 Categories = 0;
	TArray<FText> SubMenus;
	const FSlateBrush* ThumbnailBrush = nullptr;
	const FSlateBrush* IconBrush = nullptr;
};
