// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#include "ObjectVariables/GorgeousObjectVariableRegistry_GIS.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
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
    UGameInstance* Instance=GetGameInstance();
    UWorld* World=Instance ? Instance->GetWorld() : nullptr;
    bool bOtherGameInstance=false;
    if (GEngine) for (const FWorldContext& Context:GEngine->GetWorldContexts())
        if ((Context.WorldType==EWorldType::Game || Context.WorldType==EWorldType::PIE) &&
            Context.World() && Context.OwningGameInstance && Context.OwningGameInstance!=Instance)
            bOtherGameInstance=true;
    if (bOtherGameInstance) UGorgeousRootObjectVariable::PurgeWorldOwnedRegistryEntries(World,true);
    else UGorgeousRootObjectVariable::CleanupRegistry(true);
    FGorgeousAutoReplicationCoordinator::TearDownForWorld(World);
    FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
    Super::Deinitialize();
}

void UGorgeousObjectVariableRegistry_GIS::OnLevelRemoved(ULevel* Level,UWorld* World)
{
    UGameInstance* Instance=GetGameInstance();
    // A null Level means all levels are being removed (map travel). Streaming
    // one sublevel or another PIE world's callback must not purge this session.
    if (Level || !Instance || !World || World->GetGameInstance()!=Instance) return;
    UGorgeousRootObjectVariable::PurgeWorldOwnedRegistryEntries(World,false);
}
