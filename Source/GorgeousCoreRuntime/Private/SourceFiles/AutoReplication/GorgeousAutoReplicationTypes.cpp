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

#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ObjectVariables/GorgeousObjectVariable.h"


FGorgeousAutoReplicationActiveStream::FGorgeousAutoReplicationActiveStream(TObjectPtr<UGorgeousObjectVariable> InTarget, const FGorgeousAutoReplicationStreamConfig& InConfig)
	: Target(InTarget)
	, Config(InConfig)
	, RootNetworkChannel(NAME_None)
	, Priority(InConfig.Priority)
	, EffectiveUpdateFrequency(InConfig.GetEffectiveUpdateFrequency())
	, bRespectsAccessPolicy(InConfig.bRespectAccessPolicy)
	, bRegisteredWithReplicationGraph(false)
	, bRegisteredWithIris(false)
	, StreamGuid()
{
}

FGorgeousAutoReplicationActiveStream::FGorgeousAutoReplicationActiveStream(UGorgeousObjectVariable* InTarget,
	const FGorgeousAutoReplicationStreamConfig& InConfig)
	: Target(InTarget)
	, Config(InConfig)
	, RootNetworkChannel(NAME_None)
	, Priority(InConfig.Priority)
	, EffectiveUpdateFrequency(InConfig.GetEffectiveUpdateFrequency())
	, bRespectsAccessPolicy(InConfig.bRespectAccessPolicy)
	, bRegisteredWithReplicationGraph(false)
	, bRegisteredWithIris(false)
	, StreamGuid()
{
}

void FGorgeousAutoReplicationActiveStream::RefreshStreamGuid(UGorgeousObjectVariable* Variable)
{
	if (Variable)
	{
		if (const IGorgeousObjectVariableInteraction_I* Interaction = Cast<IGorgeousObjectVariableInteraction_I>(Variable))
		{
			StreamGuid = Interaction->Execute_GetUniqueIdentifierForObjectVariable(Variable);
		}
	}

	if (!StreamGuid.IsValid())
	{
		StreamGuid = FGuid::NewGuid();
	}
}

bool FGorgeousAutoReplicationBackendTraits::UsesIris(const EGorgeousAutoReplicationBackend Backend)
{
	switch (Backend)
	{
	case EGorgeousAutoReplicationBackend::Iris:
		return true;
	default:
		return false;
	}
}

bool FGorgeousAutoReplicationBackendTraits::UsesReplicationGraph(const EGorgeousAutoReplicationBackend Backend)
{
	switch (Backend)
	{
	case EGorgeousAutoReplicationBackend::ReplicationGraph:
		return true;
	default:
		return false;
	}
}