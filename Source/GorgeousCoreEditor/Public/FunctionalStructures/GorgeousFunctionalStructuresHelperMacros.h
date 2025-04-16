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
#include "PropertyEditorModule.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousFunctionalStructureDetailCustomisation.h"
//<-------------------------------------------------------------------------->

/**
 * Registers a custom property layout customisation to the specified Functional Structure.
 * 
 * @param StaticStructName The FName of the static struct you want to register.
 */
#define UE_DECLARE_FUNCTIONAL_STRUCTURES_REGISTER(StaticStructName) \
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor"); \
	PropertyEditorModule.RegisterCustomPropertyTypeLayout( \
		StaticStructName, \
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousFunctionalStructureDetailCustomisation::MakeInstance)); \
	PropertyEditorModule.NotifyCustomizationModuleChanged();

/**
 * Unregisters the custom property layout customization from the specified Functional Structure.
 * 
 * @param StaticStructName The FName of the static struct you want to unregister.
 */
#define UE_DECLARE_FUNCTIONAL_STRUCTURES_UNREGISTER(StaticStructName) \
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor"); \
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout(StaticStructName); \
	PropertyEditorModule.NotifyCustomizationModuleChanged();