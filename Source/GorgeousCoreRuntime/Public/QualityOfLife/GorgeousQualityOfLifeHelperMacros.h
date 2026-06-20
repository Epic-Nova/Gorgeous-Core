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

#pragma once

#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "Misc/Guid.h"
#include "UObject/UObjectThreadContext.h"

class AActor;
class AGorgeousPlayerState;
class AGorgeousGameState;

/** Common helper dropped into QoL class constructors to bind the mixin and ensure the self reference entry exists. */
#define UE_QOL_INITIALIZE_ADDITIONAL_DATA_NEEDS_REWORK() \
	if (HasAnyFlags(RF_ClassDefaultObject) || HasAnyFlags(RF_ArchetypeObject)) \
	{ \
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData); \
	} \
	if (GIsEditor) \
	{ \
		FCoreUObjectDelegates::PreLoadMap.AddStatic(&FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalDataOnLevelSwitch); \
	} \
	AutoReplicationMixin.Bind(this, &AdditionalGorgeousData, &ReplicatedAutoReplicationVariables); \
	if (!FUObjectThreadContext::Get().IsRoutingPostLoad) \
	{ \
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
	}

#define UE_QOL_INITIALIZE_ADDITIONAL_DATA()

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
		if (!FUObjectThreadContext::Get().IsRoutingPostLoad) \
		{ \
			FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
		} \
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
		if (!FUObjectThreadContext::Get().IsRoutingPostLoad) \
		{ \
			FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
		} \
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

/** Declares a standard BeginPlay implementation for PlayerController with RPC relay setup. */
#define UE_QOL_DEFINE_BEGIN_PLAY_WITH_RELAY(Class) \
	void Class::BeginPlay() \
	{ \
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
		UE_DECLARE_AUTOREPLICATION_CLASS_INIT_INVOKE_ADDITIONAL_DATA_WITH_RELAY \
		Super::BeginPlay(); \
	}

/** Extended version of BeginPlay for PlayerController that allows injecting extra code at the end of the function. */
#define UE_QOL_DEFINE_BEGIN_PLAY_WITH_RELAY_AND_EXTRA(Class, ExtraCode) \
	void Class::BeginPlay() \
	{ \
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
		UE_DECLARE_AUTOREPLICATION_CLASS_INIT_INVOKE_ADDITIONAL_DATA_WITH_RELAY \
		Super::BeginPlay(); \
		{ \
			ExtraCode \
		} \
	}

/**
 * Defines EndPlay to remove the owner from the shared SelfReference OV the moment the actor
 * is destroyed on any machine.  Used by both AGorgeousPlayerController and AGorgeousPlayerState.
 */
#define UE_QOL_DEFINE_END_PLAY(Class) \
	void Class::EndPlay(const EEndPlayReason::Type EndPlayReason) \
	{ \
		FGorgeousQualityOfLifeStatics::RemoveOwnerFromSelfReference(this, AdditionalGorgeousData); \
		Super::EndPlay(EndPlayReason); \
	}

/**
 * Defines PostLogin and Logout overrides for the GameMode.
 * Fires OnPlayerLoggedIn / OnPlayerLoggedOut BlueprintImplementableEvents so BP subclasses
 * can react without overriding the C++ function.
 * OV cleanup for the leaving PlayerController is handled by UE_QOL_DEFINE_END_PLAY in
 * AGorgeousPlayerController and does NOT need to be done here.
 */
// UE_QOL_DEFINE_GAME_MODE_LOGIN_CALLBACKS requires AGorgeousPlayerState to be a complete type
// at the instantiation site, include "QualityOfLife/GorgeousPlayerState.h" before using this macro.
#define UE_QOL_DEFINE_GAME_MODE_LOGIN_CALLBACKS(Class) \
	void Class::PostLogin(APlayerController* NewPlayer) \
	{ \
		Super::PostLogin(NewPlayer); \
		/* Super::PostLogin internally broadcasts FGameModeEvents::GameModePostLoginEvent, which the */ \
		/* LocalPlayerRegistry GIS listens to in order to auto-assign the stable ID for NewPlayer.  */ \
		/* RefreshReplicatedStableId must therefore run AFTER Super so the GIS entry is guaranteed   */ \
		/* to exist before we try to push it into ReplicatedGorgeousStableId.                        */ \
		if (AGorgeousPlayerState* GorgeousPS = NewPlayer ? Cast<AGorgeousPlayerState>(NewPlayer->PlayerState) : nullptr) \
		{ \
			GorgeousPS->RefreshReplicatedStableId(); \
		} \
		OnPlayerLoggedIn(NewPlayer); \
	} \
	void Class::Logout(AController* Exiting) \
	{ \
		OnPlayerLoggedOut(Exiting); \
		Super::Logout(Exiting); \
	}

/**
 * Defines AddPlayerState and RemovePlayerState overrides for the GameState.
 * Fires the GS-level OnPlayerStateAdded / OnPlayerStateRemoved events AND forwards to the
 * matching AGorgeousPlayerState events (OnAddedToPlayerArray / OnRemovedFromPlayerArray)
 * so individual player states can react entirely in BP without any C++ overrides.
 * Requires AGorgeousPlayerState to be a complete type at the instantiation site —
 * include "QualityOfLife/GorgeousPlayerState.h" before using this macro.
 */
#define UE_QOL_DEFINE_GAME_STATE_PLAYER_STATE_CALLBACKS(Class) \
	void Class::AddPlayerState(APlayerState* PlayerState) \
	{ \
		Super::AddPlayerState(PlayerState); \
		OnPlayerStateAdded(PlayerState); \
		if (AGorgeousPlayerState* GorgeousPS = Cast<AGorgeousPlayerState>(PlayerState)) \
		{ \
			GorgeousPS->OnAddedToPlayerArray(); \
		} \
	} \
	void Class::RemovePlayerState(APlayerState* PlayerState) \
	{ \
		if (AGorgeousPlayerState* GorgeousPS = Cast<AGorgeousPlayerState>(PlayerState)) \
		{ \
			GorgeousPS->OnRemovedFromPlayerArray(); \
		} \
		OnPlayerStateRemoved(PlayerState); \
		Super::RemovePlayerState(PlayerState); \
	}

/** Defines a Blueprint-callable registration function for runtime AutoReplication entries. */
#define UE_QOL_DEFINE_REGISTER_AUTOREPLICATION_ENTRY(Class) \
	bool Class::RegisterAutoReplicationEntry(FName Key, TSubclassOf<UGorgeousObjectVariable> DefaultClass, bool bReplicate, bool bOverrideStreamConfig, FGorgeousAutoReplicationStreamConfig StreamConfigOverride) \
	{ \
		if (HasAnyFlags(RF_ClassDefaultObject) || Key.IsNone() || !DefaultClass) \
		{ \
			return false; \
		} \
		if (const AActor* AsActor = Cast<AActor>(this)) \
		{ \
			if (!AsActor->HasAuthority()) \
			{ \
				return false; \
			} \
		} \
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities); \
		FGorgeousObjectVariableEntry& Entry = AdditionalGorgeousData.FindOrAdd(Key); \
		UGorgeousObjectVariable* DefaultValue = Entry.DefaultValue; \
		if (!DefaultValue || !DefaultValue->IsA(DefaultClass)) \
		{ \
			DefaultValue = NewObject<UGorgeousObjectVariable>(this, DefaultClass, NAME_None, RF_Transactional); \
		} \
		Entry.DefaultValue = DefaultValue; \
		Entry.bReplicate = bReplicate; \
		Entry.bOverrideStreamConfig = bOverrideStreamConfig; \
		if (bOverrideStreamConfig) \
		{ \
			Entry.StreamConfigOverride = StreamConfigOverride; \
		} \
		if (DefaultValue) \
		{ \
			if (!DefaultValue->UniqueIdentifier.IsValid()) \
			{ \
				DefaultValue->UniqueIdentifier = FGuid::NewGuid(); \
			} \
			DefaultValue->InvokeInstancedFunctionality(DefaultValue->UniqueIdentifier); \
		} \
		if (!Entry.bReplicate || (DefaultValue && !DefaultValue->SupportsAutoReplicationFeatures())) \
		{ \
			Entry.Handle.Reset(); \
			Entry.Handle.Assign(Key, FGorgeousAutoReplicationHandle::InvalidReplicationIndex, this); \
			const FGorgeousAutoReplicationStreamConfig* StreamOverridePtr = Entry.bOverrideStreamConfig ? &Entry.StreamConfigOverride : nullptr; \
			Entry.Handle.CacheValue(Entry.DefaultValue, StreamOverridePtr, Entry.bReplicate); \
			if (Entry.DefaultValue) \
			{ \
				Entry.DefaultValue->ApplyReplicatedIdentifier(Entry.DefaultValue->UniqueIdentifier); \
			} \
			return true; \
		} \
		AutoReplicationMixin.InitializeAdditionalData(bActivateNetworkingCapabilities); \
		return true; \
	}
