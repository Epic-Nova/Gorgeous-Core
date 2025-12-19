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
#include "GameFramework/GameMode.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousGameMode.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of AGameMode used to manage game mode-specific logic and settings.
 * 
 * This class extends AGameMode to provide additional functionality tailored for the GorgeousCore runtime.
 * It is used to manage game mode-specific data and logic, such as player data and other relevant information.
 * The class provides overrides for the `BeginPlay()` and `PostEditChangeProperty()` functions, allowing custom behavior 
 * during the start of the game and when properties are changed in the editor, respectively.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousGameMode : public AGameMode
	, public IGorgeousAutoReplicationRPCResponder_I
	, public IGorgeousQualityOfLifeNodeTarget_I
{
	GENERATED_BODY()
	
public:

	AGorgeousGameMode();

	virtual void HandleAutoReplicationRPC_Implementation(const FGorgeousQueuedRPC& QueuedRPC) override;

	//<============================--- Overrides ---=============================>
	FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() { return AutoReplicationMixin; }
	const FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() const { return AutoReplicationMixin; }

	
	/** 
	 * Called when the game mode begins play.
	 * 
	 * This function is invoked when the game mode starts. It is intended to be overridden to implement custom
	 * logic for game mode initialization, such as managing players or configuring game settings.
	 */
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	
	//<-------------------------------------------------------------------------->

	/** Enables mixin-level networking for AutoReplication data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game Mode|Networking")
	bool bActivateNetworkingCapabilities;

	/** 
	 * Additional data for the current class.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game Mode")
	TMap<FName, FGorgeousAutoReplicationEntry> AdditionalGorgeousData; 

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game Mode|Networking")
	void OnAutoReplicationRPCReceived(const FGorgeousQueuedRPC& QueuedRPC, bool bWasHandled);

	/** Indexed replicated payload used by the AutoReplication mixin. */
	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	/** Mixin that keeps the AdditionalGorgeousData map and replicated payload in sync. */
	FGorgeousAutoReplicationMixin AutoReplicationMixin;

	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
