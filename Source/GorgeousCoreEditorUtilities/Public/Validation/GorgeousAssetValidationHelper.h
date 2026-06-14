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

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorValidatorSubsystem.h"
#include "Engine/Engine.h"
#endif

/**
 * FGorgeousAssetValidationHelper
 * 
 * Reusable utility for validating assets by class type on startup.
 * This helper is designed to be used by the editor module to validate
 * specific asset types across all Gorgeous plugins.
 */
class FGorgeousAssetValidationHelper
{
public:
	/**
	 * Validates assets of a specific class type within the Gorgeous ecosystem.
	 * 
	 * @param AssetClass The class type to validate (e.g., UGeneralSystemConfiguration_PDA, UDataRegistry)
	 * @param AssetCount Output parameter: receives the number of assets validated
	 * @param PluginContentPath Optional: restrict validation to a specific plugin path (e.g., "/GorgeousCore", "/GorgeousPlaylist")
	 * 
	 * @return True if validation completed successfully, false otherwise
	 */
	static bool ValidateAssetsByClass(
		const UClass* AssetClass,
		int32& OutAssetCount,
		const FString& PluginContentPath = TEXT(""))
	{
#if WITH_EDITOR
		if (!AssetClass || !GEditor)
		{
			OutAssetCount = 0;
			return false;
		}

		FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = ARM.Get();

		TArray<FAssetData> AssetDataArray;
		FARFilter Filter;
		Filter.ClassPaths.Add(AssetClass->GetClassPathName());
		Filter.bRecursiveClasses = true;
		Filter.bRecursivePaths = true;

		// If a specific plugin content path is provided, restrict the search to that path
		if (!PluginContentPath.IsEmpty())
		{
			Filter.PackagePaths.Add(*PluginContentPath);
		}

		AssetRegistry.GetAssets(Filter, AssetDataArray);
		OutAssetCount = AssetDataArray.Num();

		if (AssetDataArray.Num() == 0)
		{
			return true; // Success, but no assets found
		}

		// Perform validation using the editor's validator subsystem
		if (UEditorValidatorSubsystem* ValidatorSubsystem = GEditor->GetEditorSubsystem<UEditorValidatorSubsystem>())
		{
			FValidateAssetsSettings Settings;
			Settings.bSkipExcludedDirectories = false;
			Settings.bLoadAssetsForValidation = false;

			FValidateAssetsResults Results;

			ValidatorSubsystem->ValidateAssetsWithSettings(
				AssetDataArray,
				Settings,
				Results
			);

			return true;
		}

		return false;
#else
		OutAssetCount = 0;
		return false;
#endif
	}

	/**
	 * Validates multiple asset classes in sequence.
	 * 
	 * @param AssetClasses Array of class types to validate
	 * @param PluginContentPath Optional: restrict validation to a specific plugin path
	 */
	static void ValidateAssetsByClasses(
		const TArray<const UClass*>& AssetClasses,
		const FString& PluginContentPath = TEXT(""))
	{
#if WITH_EDITOR
		for (const UClass* AssetClass : AssetClasses)
		{
			if (AssetClass)
			{
				int32 AssetCount = 0;
				ValidateAssetsByClass(AssetClass, AssetCount, PluginContentPath);
			}
		}
#endif
	}
};



