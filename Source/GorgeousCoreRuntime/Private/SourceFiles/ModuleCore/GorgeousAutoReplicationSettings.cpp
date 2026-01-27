// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#include "ModuleCore/GorgeousAutoReplicationSettings.h"
#include "AutoReplication/GorgeousAutoReplicationGraph.h"

UGorgeousAutoReplicationSettings::UGorgeousAutoReplicationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bEnableIris(false)
	, bEnableReplicationGraph(true)
	, AutoReplicationGraphClass(TSoftClassPtr<UReplicationGraph>(UGorgeousAutoReplicationGraph::StaticClass()))
	, bForceIrisAtRuntime(false)
	, bForceReplicationGraphAtRuntime(false)
{
}

const UGorgeousAutoReplicationSettings* UGorgeousAutoReplicationSettings::Get()
{
	return GetDefault<UGorgeousAutoReplicationSettings>();
}
