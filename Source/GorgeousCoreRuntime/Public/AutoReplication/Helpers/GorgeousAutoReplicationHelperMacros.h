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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

/**
 * Initializes the AutoReplication mixin and invokes instanced functionality for every configured entry.
 * If the class has an AutoReplicationRPCRelay member, it will be set up to forward property payloads to the mixin.
 */
#define UE_DECLARE_AUTOREPLICATION_CLASS_INIT_INVOKE_ADDITIONAL_DATA \
	AutoReplicationMixin.InitializeAdditionalData(bActivateNetworkingCapabilities); \
	for (auto& GorgeousSetting : AdditionalGorgeousData) \
	{ \
		if (UGorgeousObjectVariable* const DefaultVar = GorgeousSetting.Value.DefaultValue) \
		{ \
			if(!DefaultVar->UniqueIdentifier.IsValid()) \
			{ \
				DefaultVar->UniqueIdentifier = FGuid::NewGuid(); \
			} \
			DefaultVar->InvokeInstancedFunctionality(DefaultVar->UniqueIdentifier); \
		} \
	}

/**
 * Extended version that also sets up the RPC relay component for property payload forwarding.
 * Use this in classes that have an AutoReplicationRPCRelay member (like AGorgeousPlayerController).
 */
#define UE_DECLARE_AUTOREPLICATION_CLASS_INIT_INVOKE_ADDITIONAL_DATA_WITH_RELAY \
	AutoReplicationMixin.InitializeAdditionalData(bActivateNetworkingCapabilities); \
	if (AutoReplicationRPCRelay) \
	{ \
		AutoReplicationRPCRelay->SetTargetMixin(&AutoReplicationMixin); \
		AutoReplicationMixin.SetRPCRelayComponent(AutoReplicationRPCRelay); \
	} \
	for (auto& GorgeousSetting : AdditionalGorgeousData) \
	{ \
		if (UGorgeousObjectVariable* const DefaultVar = GorgeousSetting.Value.DefaultValue) \
		{ \
			if(!DefaultVar->UniqueIdentifier.IsValid()) \
			{ \
				DefaultVar->UniqueIdentifier = FGuid::NewGuid(); \
			} \
			DefaultVar->InvokeInstancedFunctionality(DefaultVar->UniqueIdentifier); \
		} \
	}

/**
 * Implements the default OnRep handler that simply refreshes cached handles via the mixin.
 */
#define UE_DECLARE_AUTOREPLICATION_CLASS_ON_REP_VARIABLES(Class) \
	void Class::OnRep_GorgeousAutoReplicationVariables() \
	{ \
		AutoReplicationMixin.RefreshCachedValues(); \
	}

/**
 * Registers the replicated AutoReplication array with Unreal's replication system.
 */
#define UE_DECLARE_AUTOREPLICATION_CLASS_GET_LIFETIME_REPLICATED_PROPS(Class) \
	void Class::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const \
	{ \
		Super::GetLifetimeReplicatedProps(OutLifetimeProps); \
		DOREPLIFETIME(Class, ReplicatedAutoReplicationVariables); \
	}
