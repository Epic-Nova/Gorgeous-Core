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
#include "EditorSubsystem.h"
#include "AssetTypeCategories.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousAssetTypeAction.h"
#include "GorgeousAssetRegistrationStructures.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousAssetRegistration_ES.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousFactory;
//<-------------------------------------------------->

/**
 * Editor subsystem for managing and registering custom asset types and categories.
 *
 * This class acts as the central interface for dynamically registering new asset types,
 * as well as defining custom asset categories within the Unreal Engine editor. It encapsulates
 * the handling of `FAssetTypeActions` and simplifies the integration of Gorgeous assets
 * into the content browser at runtime or module startup.
 *
 * Example use cases:
 * - Registering a new custom asset type under a specific category.
 * - Dynamically adding Gorgeous asset menus to the Content Browser.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Gorgeous Asset Registration")
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousAssetRegistration : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:

	/**
	 * Registers a new custom asset type with the editor.
	 *
	 * Creates a new asset type action and integrates it into the editor asset system.
	 *
	 * @param AssetTypeActionInfo A structure containing metadata about the asset type (name, class, color, submenus, etc.).
	 * @param AssetTypeCategories A list of asset type categories where the new asset should appear.
	 */
	void RegisterNewAsset(FGorgeousAssetTypeActionInfo_S& AssetTypeActionInfo, const TArray<EAssetTypeCategories::Type>& AssetTypeCategories);

	/**
	 * Registers a new asset category in the editor.
	 *
	 * Allows custom asset types to appear under newly defined categories in the Content Browser.
	 *
	 * @param CategoryDisplayName The name to display for the new category.
	 * @param NewAssetTypeCategory The resulting enum value representing the newly created category.
	 */
	void RegisterNewCategory(const FText& CategoryDisplayName, EAssetTypeCategories::Type& NewAssetTypeCategory);

private:

	/**
	 * Holds references to all registered Gorgeous asset type actions.
	 *
	 * This array is used to retain ownership and ensure lifetime management
	 * of dynamically registered asset type actions.
	 */
	TArray<TSharedPtr<FGorgeousAssetTypeAction>> GorgeousAssets;
};