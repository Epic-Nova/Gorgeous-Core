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
#include "GorgeousCoreEditorModule.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreEditorUtilitiesMinimalShared.h"
#include "GorgeousCoreMinimalShared.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreEditorModule Implementation
//=============================================================================

//@TODO: Planned for future versions

void FGorgeousCoreEditorModule::StartupModule()
{
	UGorgeousAssetRegistration* AssetRegistration = GEditor->GetEditorSubsystem<UGorgeousAssetRegistration>();

	EAssetTypeCategories::Type GorgeousThingsAssetCategory;
	AssetRegistration->RegisterNewCategory(INVTEXT("Gorgeous Things"), GorgeousThingsAssetCategory);
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Object Variable"), UGorgeousObjectVariable::StaticClass(),
			FColor::Blue, {INVTEXT("Gorgeous Core")});
		
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Conditional Object Chooser"), UGorgeousConditionalObjectChooser::StaticClass(),
		FColor::Cyan, {INVTEXT("Gorgeous Core"), INVTEXT("Conditional Object Choosers")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Boolean Condition"), UGorgeousBooleanCondition::StaticClass(),
		FColor::Turquoise, {INVTEXT("Gorgeous Core"), INVTEXT("Conditional Object Choosers"), INVTEXT("Conditions")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous IsValid Condition"), UGorgeousIsValidCondition::StaticClass(),
		FColor::Turquoise, {INVTEXT("Gorgeous Core"), INVTEXT("Conditional Object Choosers"), INVTEXT("Conditions")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Gameplay Tag Condition"), UGorgeousGameplayTagCondition::StaticClass(),
		FColor::Turquoise, {INVTEXT("Gorgeous Core"), INVTEXT("Conditional Object Choosers"), INVTEXT("Conditions")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}

	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Game Instance"), UGorgeousGameInstance::StaticClass(),
		FColor::Blue, {INVTEXT("Gorgeous Core"), INVTEXT("Quality of Life")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Game Mode"), AGorgeousGameMode::StaticClass(),
		FColor::Blue, {INVTEXT("Gorgeous Core"), INVTEXT("Quality of Life")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Game State"), AGorgeousGameState::StaticClass(),
		FColor::Blue, {INVTEXT("Gorgeous Core"), INVTEXT("Quality of Life")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Player Controller"), AGorgeousPlayerController::StaticClass(),
		FColor::Blue, {INVTEXT("Gorgeous Core"), INVTEXT("Quality of Life")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous Player State"), AGorgeousPlayerState::StaticClass(),
		FColor::Blue, {INVTEXT("Gorgeous Core"), INVTEXT("Quality of Life")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	{
		FGorgeousAssetTypeActionInfo_S AssetTypeActionInfo = FGorgeousAssetTypeActionInfo_S(INVTEXT("Gorgeous World Settings"), AGorgeousWorldSettings::StaticClass(),
		FColor::Blue, {INVTEXT("Gorgeous Core"), INVTEXT("Quality of Life")});
	
		AssetRegistration->RegisterNewAsset(AssetTypeActionInfo, {GorgeousThingsAssetCategory});
	}
	
	/*FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(
		UGorgeousObjectVariable::StaticClass()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousObjectVariableDetailCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();*/
}

void FGorgeousCoreEditorModule::ShutdownModule()
{
	/*if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(UGorgeousObjectVariable::StaticClass()->GetFName());
	}*/
}

TArray<FName> FGorgeousCoreEditorModule::GetDependentPlugins()
{
	return TArray<FName>();
}

IMPLEMENT_MODULE(FGorgeousCoreEditorModule, GorgeousCoreEditor)
