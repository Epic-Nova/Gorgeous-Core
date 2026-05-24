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
#include "Factories/DataSchemaMapping/GorgeousDataSchemaMappingFactory.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetToolsModule.h"
#include "IAssetTools.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousDataSchemaMappingFactory"

UGorgeousDataSchemaMappingFactory::UGorgeousDataSchemaMappingFactory()
{
	SupportedClass = UGorgeousDataSchemaMapping_DA::StaticClass();
	bEditAfterNew = true;
	bEditorImport = false;
	bCreateNew = true;
	bText = false;
}

UObject* UGorgeousDataSchemaMappingFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ClassToCreate = InClass;
	if (!ClassToCreate || !ClassToCreate->IsChildOf(UGorgeousDataSchemaMapping_DA::StaticClass()))
	{
		ClassToCreate = UGorgeousDataSchemaMapping_DA::StaticClass();
	}

	return NewObject<UObject>(InParent, ClassToCreate, InName, Flags | RF_Transactional);
}

uint32 UGorgeousDataSchemaMappingFactory::GetMenuCategories() const
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools") || FModuleManager::Get().LoadModule("AssetTools"))
	{
		const IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		const EAssetTypeCategories::Type Category = AssetTools.FindAdvancedAssetCategory(FName("GorgeousThings"));
		if (Category != EAssetTypeCategories::None)
		{
			return Category;
		}
	}

	return EAssetTypeCategories::Misc;
}

FText UGorgeousDataSchemaMappingFactory::GetDisplayName() const
{
	return LOCTEXT("FactoryDisplayName", "Gorgeous Data Schema Mapping");
}

#undef LOCTEXT_NAMESPACE
