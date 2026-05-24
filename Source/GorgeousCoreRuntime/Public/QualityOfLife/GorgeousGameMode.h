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
#include "GameFramework/GameMode.h"
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
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

	/** Registers or updates an AutoReplication entry at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Game Mode|Networking")
	bool RegisterAutoReplicationEntry(FName Key, TSubclassOf<UGorgeousObjectVariable> DefaultClass, bool bReplicate, bool bOverrideStreamConfig, FGorgeousAutoReplicationStreamConfig StreamConfigOverride);

	
	/** 
	 * Called when the game mode begins play.
	 * 
	 * This function is invoked when the game mode starts. It is intended to be overridden to implement custom
	 * logic for game mode initialization, such as managing players or configuring game settings.
	 */
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void Test();

	/**
	 * Called on the server when a player successfully logs in.
	 * Fires BEFORE the PlayerController's BeginPlay, so the OV self-registration
	 * for the new PC happens slightly later (in its own BeginPlay).
	 */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Called on the server when a player logs out or disconnects. */
	virtual void Logout(AController* Exiting) override;
	
	//<-------------------------------------------------------------------------->

	/** Enables mixin-level networking for AutoReplication data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game Mode|Networking")
	bool bActivateNetworkingCapabilities;

	/**
	 * Additional data for the current class.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game Mode")
	TMap<FName, FGorgeousObjectVariableEntry> AdditionalGorgeousData;

	/** Trunk that stores serialized default payloads for this game mode's object variables. */
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous Game Mode|Defaults", meta = (ShowOnlyInnerProperties))
	FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game Mode|Networking")
	void OnAutoReplicationRPCReceived(const FGorgeousQueuedRPC& QueuedRPC, bool bWasHandled);

	/**
	 * Fired on the server after a new PlayerController has logged in.
	 * At this point the PC exists but its BeginPlay (and therefore its OV
	 * self-registration) has not yet fired.  Use this for gameplay setup that
	 * does not depend on the QoL reference system.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game Mode|Players")
	void OnPlayerLoggedIn(APlayerController* NewPlayer);

	/**
	 * Fired on the server when a player logs out or disconnects.
	 * The PlayerController and its self-reference OV entry have already been
	 * cleaned up by the time this event fires.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game Mode|Players")
	void OnPlayerLoggedOut(AController* ExitingController);

	/** Indexed replicated payload used by the AutoReplication mixin. */
	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	/** Mixin that keeps the AdditionalGorgeousData map and replicated payload in sync. */
	FGorgeousAutoReplicationMixin AutoReplicationMixin;
	
	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
