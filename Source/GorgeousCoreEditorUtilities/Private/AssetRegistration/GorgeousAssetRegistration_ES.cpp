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
#include "AssetRegistration/GorgeousAssetRegistration_ES.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "AssetRegistration/GorgeousAssetTypeAction.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetToolsModule.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UDEPRECATED_GorgeousAssetRegistration_ES Implementation
//=============================================================================

void UDEPRECATED_GorgeousAssetRegistration_ES::RegisterNewAsset(FGorgeousAssetTypeActionInfo_S& AssetTypeActionInfo, const TArray<EAssetTypeCategories::Type>& AssetTypeCategories)
{
	if(!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
    
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	for (const auto AssetTypeCategory : AssetTypeCategories)
			AssetTypeActionInfo.Categories |= AssetTypeCategory;
	
	const auto NewFactoryIndex = GorgeousAssets.Add(MakeShared<FGorgeousAssetTypeAction>(AssetTypeActionInfo));
	AssetTools.RegisterAssetTypeActions(GorgeousAssets[NewFactoryIndex].ToSharedRef());
}

void UDEPRECATED_GorgeousAssetRegistration_ES::RegisterNewCategory(const FText& CategoryDisplayName, EAssetTypeCategories::Type& NewAssetTypeCategory)
{
	if(!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	
	NewAssetTypeCategory = AssetTools.RegisterAdvancedAssetCategory(FName(CategoryDisplayName.ToString().Replace(TEXT(" "),  TEXT(""))), CategoryDisplayName);
}
