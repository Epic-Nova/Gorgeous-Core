// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#include "GorgeousFunctionalStructures_GIS.h"
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
#include "GorgeousFunctionalStructureDetailCustomisation.h"

void UGorgeousFunctionalStructures_GIS::RegisterFunctionalStructure(FGorgeousFunctionalStructure_S& Structure)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(Structure.StaticStruct()->GetClass()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousFunctionalStructureDetailCustomisation::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void UGorgeousFunctionalStructures_GIS::UnregisterFunctionalStructure(FGorgeousFunctionalStructure_S& Structure)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout(Structure.StaticStruct()->GetClass()->GetFName());
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}
