/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/
#include "ObjectVariables/GorgeousObjectVariableRegistry_GIS.h"

#include "ObjectVariables/GorgeousRootObjectVariable.h"

void UGorgeousObjectVariableRegistry_GIS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGorgeousRootObjectVariable::GetRootObjectVariable();
	FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UGorgeousObjectVariableRegistry_GIS::OnLevelRemoved);
}

void UGorgeousObjectVariableRegistry_GIS::Deinitialize()
{
	Super::Deinitialize();

	UGorgeousRootObjectVariable::CleanupRegistry(true);
	FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
}

void UGorgeousObjectVariableRegistry_GIS::OnLevelRemoved(ULevel* Level, UWorld* World)
{
	UGorgeousRootObjectVariable::CleanupRegistry();
}
