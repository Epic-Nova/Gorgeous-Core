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

#include "GorgeousCoreEditorUtilitiesModule.h"

//=============================================================================
// FGorgeousCoreEditorUtilitiesModule Implementation
//=============================================================================

void FGorgeousCoreEditorUtilitiesModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("FGorgeousCoreEditorUtilitiesModule has started!"));
}

void FGorgeousCoreEditorUtilitiesModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("FGorgeousCoreEditorUtilitiesModule has shut down!"));
}

IMPLEMENT_MODULE(FGorgeousCoreEditorUtilitiesModule, GorgeousCoreEditorUtilities)