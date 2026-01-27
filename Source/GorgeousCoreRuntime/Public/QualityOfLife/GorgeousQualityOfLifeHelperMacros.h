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

#pragma once

#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "Misc/Guid.h"

/** Common helper dropped into QoL class constructors to bind the mixin and ensure the self reference entry exists. */
#define UE_QOL_INITIALIZE_ADDITIONAL_DATA() \
	if (HasAnyFlags(RF_ClassDefaultObject)) \
	{ \
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData); \
	} \
	AutoReplicationMixin.Bind(this, &AdditionalGorgeousData, &ReplicatedAutoReplicationVariables); \
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities);

/** Declares a standard QoL constructor that wires the networking default and self-reference bootstrap. */
#define UE_QOL_DEFINE_CONSTRUCTOR(Class, bDefaultNetworking) \
	Class::Class() \
		: bActivateNetworkingCapabilities(bDefaultNetworking) \
	{ \
		UE_QOL_INITIALIZE_ADDITIONAL_DATA(); \
	}

/** Generates the HandleAutoReplicationRPC override shared across QoL classes. */
#define UE_QOL_DEFINE_HANDLE_AUTOREPLICATION_RPC(Class) \
	void Class::HandleAutoReplicationRPC_Implementation(const FGorgeousQueuedRPC& QueuedRPC) \
	{ \
		const bool bHandled = AutoReplicationMixin.ExecuteAutoReplicationRPC(QueuedRPC); \
		OnAutoReplicationRPCReceived(QueuedRPC, bHandled); \
	}

/** Defines PostInitProperties so every CDO refreshes its self reference subobject. */
#define UE_QOL_DEFINE_POST_INIT_PROPERTIES(Class) \
	void Class::PostInitProperties() \
	{ \
		Super::PostInitProperties(); \
		if (HasAnyFlags(RF_ClassDefaultObject)) \
		{ \
			FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData); \
			return; \
		} \
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
	}

/** Defines PostLoad so deserialized assets re-establish their self reference. */
#define UE_QOL_DEFINE_POST_LOAD(Class) \
	void Class::PostLoad() \
	{ \
		Super::PostLoad(); \
		if (HasAnyFlags(RF_ClassDefaultObject)) \
		{ \
			FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData); \
			return; \
		} \
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
	}

/** Convenience macro that declares both PostInitProperties and PostLoad overrides. */
#define UE_QOL_DEFINE_POST_INIT_AND_LOAD(Class) \
	UE_QOL_DEFINE_POST_INIT_PROPERTIES(Class) \
	UE_QOL_DEFINE_POST_LOAD(Class)

/** Defines PostEditChangeProperty to guard the map against manual removals and regenerate unique IDs. */
#define UE_QOL_DEFINE_POST_EDIT_CHANGE_PROPERTY(Class) \
	void Class::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) \
	{ \
		Super::PostEditChangeProperty(PropertyChangedEvent); \
		const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None; \
		if (PropertyName == GET_MEMBER_NAME_CHECKED(Class, AdditionalGorgeousData)) \
		{ \
			FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
			for (auto& GorgeousSetting : AdditionalGorgeousData) \
			{ \
				if (UGorgeousObjectVariable* const DefaultVar = GorgeousSetting.Value.DefaultValue) \
				{ \
					DefaultVar->UniqueIdentifier = FGuid::NewGuid(); \
				} \
			} \
		} \
	}

/** Declares a standard BeginPlay implementation used by QoL Actor classes. */
#define UE_QOL_DEFINE_BEGIN_PLAY(Class) \
	void Class::BeginPlay() \
	{ \
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
		UE_DECLARE_AUTOREPLICATION_CLASS_INIT_INVOKE_ADDITIONAL_DATA \
		Super::BeginPlay(); \
	}
