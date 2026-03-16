// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "ObjectVariables/GorgeousObjectVariableRegistry_GIS.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
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

	// Tear down the auto-replication coordinator.  The OnWorldCleanup delegate
	// registered in the module never fires in PIE teardown, so we do it here
	// where Deinitialize() is guaranteed to run.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			FGorgeousAutoReplicationCoordinator::TearDownForWorld(World);
		}
	}

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
