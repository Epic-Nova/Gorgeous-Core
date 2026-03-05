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

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "GameFramework/PlayerState.h"
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
#include "QualityOfLife/GorgeousPlayerConnectionInfo_I.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousPlayerState.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of APlayerState used to manage player state and additional data.
 * 
 * This class extends APlayerState to provide extra functionality for managing player state and custom player metadata. 
 * The `AGorgeousPlayerState` class allows for the management of additional data through the `AdditionalGorgeousData` map, 
 * which can hold metadata or other relevant information about the player. The class also provides custom behavior for 
 * `BeginPlay()` and `PostEditChangeProperty()` functions, enabling initialization and modification of the player state 
 * in various scenarios, such as at the start of the game or when properties are edited in the Unreal Editor.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousPlayerState : public APlayerState
, public IGorgeousAutoReplicationRPCResponder_I
, public IGorgeousQualityOfLifeNodeTarget_I
, public IGorgeousPlayerConnectionInfo_I
{
	GENERATED_BODY()
	
	friend class AGorgeousGameState;
	friend class AGorgeousGameMode;
	
	public:
	
	AGorgeousPlayerState();

	FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() { return AutoReplicationMixin; }
	const FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() const { return AutoReplicationMixin; }

	/** Registers or updates an AutoReplication entry at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Player State|Networking")
	bool RegisterAutoReplicationEntry(FName Key, TSubclassOf<UGorgeousObjectVariable> DefaultClass, bool bReplicate, bool bOverrideStreamConfig, FGorgeousAutoReplicationStreamConfig StreamConfigOverride);

	virtual void HandleAutoReplicationRPC_Implementation(const FGorgeousQueuedRPC& QueuedRPC) override;
	
	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the player state begins play.
	 * 
	 * This function is called when the player state is initialized. It is ideal for setting up the player's state, 
	 * managing metadata, or initializing any relevant data related to the player.
	 */
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Removes this player state from the shared SelfReference OV when it is destroyed. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// IGorgeousPlayerConnectionInfo_I
	virtual bool IsRemoteNetConnection_Implementation() const override;
	virtual FUniqueNetIdRepl GetPlayerNetId_Implementation() const override;
	virtual FString GetGorgeousStablePlayerId_Implementation() const override;
	virtual int32 GetPlayerConnectionIndex_Implementation() const override;

	/**
	 * Sets a new stable ID for this player, replicating it across the network.
	 * Can be called from any machine: clients send a server RPC, the server updates
	 * directly.  The new ID replaces both the local registry entry and the replicated
	 * property so all machines receive the change via OnRep_GorgeousStableId.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Player State|Player Connection")
	void SetGorgeousStablePlayerId(const FString& NewId);
	
	//<-------------------------------------------------------------------------->

	/** Enables mixin networking for this player state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Player State|Networking")
	bool bActivateNetworkingCapabilities;

	/**
	 * Additional data for the current class.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Player State")
	TMap<FName, FGorgeousObjectVariableEntry> AdditionalGorgeousData;

	/** Trunk that persists serialized default payloads authored on this player state. */
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous Player State|Defaults", meta = (ShowOnlyInnerProperties))
	FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Player State|Networking")
	void OnAutoReplicationRPCReceived(const FGorgeousQueuedRPC& QueuedRPC, bool bWasHandled);

	/**
	 * Fired on ALL machines when the replicated stable ID changes.
	 * This fires on clients via OnRep and on the server directly after a successful rename.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Player State|Player Connection")
	void OnGorgeousStablePlayerIdChanged(const FString& NewStableId);

	/**
	 * The stable player ID replicated from the server to all clients.
	 * Updated automatically when SetGorgeousStablePlayerId is called.
	 * Prefer reading this through GetGorgeousStablePlayerId() (IGorgeousPlayerConnectionInfo_I)
	 * rather than accessing this property directly.
	 *
	 * NOTE — future replacement planned:
	 * This property is implemented as a classic replicated UPROPERTY with a manual
	 * OnRep handler, a server RPC, and a RefreshReplicatedStableId helper.  That is a
	 * significant amount of boilerplate for a single FString.
	 * Once the auto-replication system supports properties that live outside of Object
	 * Variables (i.e. plain UObject/AActor members, not just values stored inside an OV),
	 * this entire setup should be collapsed into a single auto-replicated declaration —
	 * the system will then handle change-detection, delta-compression, and fan-out to all
	 * machines automatically, the same way it already works for OV-based variables.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_GorgeousStableId, BlueprintReadOnly, Category = "Gorgeous Player State|Player Connection")
	FString ReplicatedGorgeousStableId;

	/**
	 * Fired on ALL machines when this PlayerState has been added to the GameState's PlayerArray.
	 * By the time this fires the OV self-reference for this PS is already registered, so
	 * GetQualityOfLifeReferences(PlayerState) will include this player.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Player State|Players")
	void OnAddedToPlayerArray();

	/**
	 * Fired on ALL machines just before this PlayerState is removed from the GameState's PlayerArray.
	 * The PS is still valid at this point but will be destroyed shortly after.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Player State|Players")
	void OnRemovedFromPlayerArray();

	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	FGorgeousAutoReplicationMixin AutoReplicationMixin;

	/** Routes auto-replication RPCs between this PS and the server, mirroring the relay on AGorgeousPlayerController. */
	UPROPERTY()
	TObjectPtr<UGorgeousAutoReplicationRPCRelayComponent> AutoReplicationRPCRelay;
	
	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	UFUNCTION()
	void OnRep_GorgeousStableId();

	/** Server RPC that lets any owning client request a stable ID rename. */
	UFUNCTION(Server, Reliable)
	void Server_SetGorgeousStablePlayerId(const FString& NewId);

	/**
	 * Called by AGorgeousGameState::AddPlayerState (friend) after the PS is fully set up
	 * with its owning PlayerController so the initial stable ID can be pushed into
	 * ReplicatedGorgeousStableId and replicated to all clients.
	 */
	void RefreshReplicatedStableId();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
