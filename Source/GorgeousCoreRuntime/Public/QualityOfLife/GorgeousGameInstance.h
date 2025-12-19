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
//<-------------------------------------------------------------------------->
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousGameInstance.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of UGameInstance used to handle game-specific initialization, settings, and configuration.
 * 
 * This class extends UGameInstance to provide additional functionality tailored for the GorgeousCore runtime. It includes the ability
 * to manage additional configuration data specific to the game instance through the `AdditionalGorgeousData` property.
 * 
 * The class provides overrides for the `Init()` and `PostEditChangeProperty()` functions, which allow custom initialization 
 * and handling of property changes, respectively.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousGameInstance : public UGameInstance
	, public IGorgeousAutoReplicationRPCResponder_I
	, public IGorgeousQualityOfLifeNodeTarget_I
{
	GENERATED_BODY()

public:

	UGorgeousGameInstance();

	virtual void HandleAutoReplicationRPC_Implementation(const FGorgeousQueuedRPC& QueuedRPC) override;

	FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() { return AutoReplicationMixin; }
	const FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() const { return AutoReplicationMixin; }

	//<============================--- Overrides ---=============================>

	/** 
	 * Initializes the game instance.
	 * 
	 * This function is called when the game instance is initialized. It is intended to be overridden 
	 * to add custom initialization logic for the game instance, such as loading resources or setting up settings.
	 */
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	
	//<-------------------------------------------------------------------------->
	
	/** Enables the networking pathway for AutoReplication data so entries register with the replicated array at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game Instance|Networking")
	bool bActivateNetworkingCapabilities;

	/** 
	 * Additional settings/configuration data for the current game instance.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game Instance")
	TMap<FName, FGorgeousAutoReplicationEntry> AdditionalGorgeousData;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Game Instance|Networking")
	void OnAutoReplicationRPCReceived(const FGorgeousQueuedRPC& QueuedRPC, bool bWasHandled);

	/** Local backing store for replicated AutoReplication slots (game instances never replicate but the mixin expects valid storage). */
	UPROPERTY(Transient)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	/** Helper that wires mixin flows across all AutoReplication entry points. */
	FGorgeousAutoReplicationMixin AutoReplicationMixin;

private:
	void EnsureRootVariablesFallbackToGameInstance();
	void HandleRootRegistryChanged();

	bool bIsEnsuringRootFallback = false; // prevent reentrant root fallback loops from delegate broadcasts
	FDelegateHandle RootRegistryChangedHandle;
};
