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

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "GameFramework/GameState.h"
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousGameState.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of AGameState used to manage game state-specific logic and settings.
 * 
 * This class extends AGameState to provide additional functionality tailored for the GorgeousCore runtime.
 * It is used to manage game state-specific data and logic, such as match data and other relevant information 
 * during gameplay. The class provides overrides for the `BeginPlay()` and `PostEditChangeProperty()` functions,
 * enabling custom behavior during the start of the game state and when properties are modified in the editor.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousGameState : public AGameState
	, public IGorgeousAutoReplicationRPCResponder_I
	, public IGorgeousQualityOfLifeNodeTarget_I
{
	GENERATED_BODY()
	
public:

	AGorgeousGameState();

	
	virtual void HandleAutoReplicationRPC_Implementation(const FGorgeousQueuedRPC& QueuedRPC) override;

	FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() { return AutoReplicationMixin; }
	const FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() const { return AutoReplicationMixin; }

	/** Registers or updates an AutoReplication entry at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Game State|Networking")
	bool RegisterAutoReplicationEntry(FName Key, TSubclassOf<UGorgeousObjectVariable> DefaultClass, bool bReplicate, bool bOverrideStreamConfig, FGorgeousAutoReplicationStreamConfig StreamConfigOverride);
	
	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the game state begins play.
	 * 
	 * This function is called when the game state starts. It is intended to be overridden to implement custom
	 * logic for initializing the game state, such as setting up match conditions or managing state variables.
	 */
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Called on ALL machines (server + clients via replication) when a new
	 * PlayerState is added to the PlayerArray.  The PS's BeginPlay fires before
	 * this, so its OV self-registration is already complete by this point.
	 */
	virtual void AddPlayerState(APlayerState* PlayerState) override;

	/**
	 * Called on ALL machines when a PlayerState is removed from the PlayerArray
	 * (player left or disconnected).  The PS's EndPlay handles OV cleanup;
	 * this override fires the BP event and calls Super.
	 */
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	
	//<-------------------------------------------------------------------------->

	/** Enables mixin networking path for this game state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game State|Networking")
	bool bActivateNetworkingCapabilities;

	/**
	 * Additional data for the current class.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game State")
	TMap<FName, FGorgeousObjectVariableEntry> AdditionalGorgeousData;

	/** Authoritative trunk for serialized default payloads authored on this game state. */
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous Game State|Defaults", meta = (ShowOnlyInnerProperties))
	FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game State|Networking")
	void OnAutoReplicationRPCReceived(const FGorgeousQueuedRPC& QueuedRPC, bool bWasHandled);

	/**
	 * Fired on ALL machines when a new PlayerState is added to the PlayerArray.
	 * Safe to query GetQualityOfLifeReferences(PlayerState) here — the new PS
	 * will already be in the OV array.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game State|Players")
	void OnPlayerStateAdded(APlayerState* PlayerState);

	/**
	 * Fired on ALL machines when a PlayerState is removed from the PlayerArray.
	 * The PS is still valid at this point but will be destroyed shortly after.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game State|Players")
	void OnPlayerStateRemoved(APlayerState* PlayerState);

	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	FGorgeousAutoReplicationMixin AutoReplicationMixin;
	
	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

