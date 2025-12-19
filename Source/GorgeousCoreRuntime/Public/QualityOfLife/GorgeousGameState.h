// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "GameFramework/GameState.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
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
	
	//<-------------------------------------------------------------------------->

	/** Enables mixin networking path for this game state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game State|Networking")
	bool bActivateNetworkingCapabilities;

	/** 
	 * Additional data for the current class.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game State")
	TMap<FName, FGorgeousAutoReplicationEntry> AdditionalGorgeousData; 

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game State|Networking")
	void OnAutoReplicationRPCReceived(const FGorgeousQueuedRPC& QueuedRPC, bool bWasHandled);

	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	FGorgeousAutoReplicationMixin AutoReplicationMixin;

	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

