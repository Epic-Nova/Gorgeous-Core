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
#include "ObjectVariables/GorgeousObjectVariableRegistry_GIS.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousObjectVariableRegistry_GIS Implementation
//=============================================================================

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
	if (UGameInstance* OwningGameInstance = GetGameInstance())
	{
		const TArray<FName> RegisteredRootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
		for (const FName& RootName : RegisteredRootNames)
		{
			if (UGorgeousRootObjectVariable::IsSharedNetworkingRoot(RootName))
			{
				UGorgeousRootObjectVariable::PromoteRootRegistryOwner(RootName, OwningGameInstance);
			}
		}
	}
	UGorgeousRootObjectVariable::CleanupRegistry();
}
