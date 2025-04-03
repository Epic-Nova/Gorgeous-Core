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
#include "GorgeousFunctionalStructures_GIS.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
#include "GorgeousFunctionalStructureDetailCustomisation.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousFunctionalStructures_GIS Implementation
//=============================================================================

void UGorgeousFunctionalStructures_GIS::RegisterFunctionalStructure(FGorgeousFunctionalStructure_S& Structure)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(
		Structure.StaticStruct()->GetClass()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousFunctionalStructureDetailCustomisation::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void UGorgeousFunctionalStructures_GIS::UnregisterFunctionalStructure(FGorgeousFunctionalStructure_S& Structure)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout(Structure.StaticStruct()->GetClass()->GetFName());
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}