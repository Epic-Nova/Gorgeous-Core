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
#include "GorgeousCoreEditorModule.h"
#include "GorgeousObjectVariableDetailCustomization.h"
#include "ObjectVariables/GorgeousObjectVariable.h"

void FGorgeousCoreEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(UGorgeousObjectVariable::StaticClass()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousObjectVariableDetailCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void FGorgeousCoreEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(UGorgeousObjectVariable::StaticClass()->GetFName());
	}
}

IMPLEMENT_MODULE(FGorgeousCoreEditorModule, GorgeousCoreEditor)