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

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "GameFramework/PlayerState.h"
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
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
{
	GENERATED_BODY()
	
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

	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	FGorgeousAutoReplicationMixin AutoReplicationMixin;
	
	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
