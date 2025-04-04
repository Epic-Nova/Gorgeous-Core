﻿// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
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
//<-------------------------=== Engine Includes ===-------------------------->
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousObjectVariableDetailCustomization.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreEditorModule Implementation
//=============================================================================

//@TODO: Planned for future versions

void FGorgeousCoreEditorModule::StartupModule()
{
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
