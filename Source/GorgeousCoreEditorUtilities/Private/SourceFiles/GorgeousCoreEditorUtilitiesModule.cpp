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
#include "GorgeousCoreEditorUtilitiesModule.h"
#include "AssetRegistration/GorgeousAssetRegistration_ES.h"
#include "ObjectVariables/GorgeousObjectVariable.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
//<-------------------------=== Module Includes ===-------------------------->
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreEditorUtilitiesModule Implementation
//=============================================================================

void FGorgeousCoreEditorUtilitiesModule::StartupModule()
{

}

void FGorgeousCoreEditorUtilitiesModule::ShutdownModule()
{
}

TArray<FName> FGorgeousCoreEditorUtilitiesModule::GetDependentPlugins()
{
	return TArray<FName>();
}

IMPLEMENT_MODULE(FGorgeousCoreEditorUtilitiesModule, GorgeousCoreEditor)
