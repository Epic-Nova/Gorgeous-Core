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
#include "GameFramework/WorldSettings.h"
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousWorldSettings.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A custom subclass of AWorldSettings used to manage world-specific settings and configuration.
 * 
 * This class extends AWorldSettings to provide extra functionality for managing world-specific data and configurations. 
 * The `AGorgeousWorldSettings` class enables the management of additional world-related settings such as NPC spawn points, 
 * item spawns, and other important world data through the `AdditionalGorgeousData` map. Custom behaviors are also provided 
 * for `BeginPlay()` and `PostEditChangeProperty()` functions to handle world initialization and property changes.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousWorldSettings : public AWorldSettings
, public IGorgeousQualityOfLifeNodeTarget_I
{
	GENERATED_BODY()
	
public:

	AGorgeousWorldSettings();

	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() { return AutoReplicationMixin; }
	const FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() const { return AutoReplicationMixin; }

	/** Registers or updates an AutoReplication entry at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous World Settings|Networking")
	bool RegisterAutoReplicationEntry(FName Key, TSubclassOf<UGorgeousObjectVariable> DefaultClass, bool bReplicate, bool bOverrideStreamConfig, FGorgeousAutoReplicationStreamConfig StreamConfigOverride);

	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the world settings begin play.
	 * 
	 * This function is called when the world settings are initialized. It is ideal for setting up world-specific data, 
	 * such as NPC spawn locations, item spawns, and other world-related properties.
	 */
	virtual void BeginPlay() override;
	
	//<-------------------------------------------------------------------------->

	/** Enables networking for world-setting AutoReplication values. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous World Settings|Networking")
	bool bActivateNetworkingCapabilities;

	/**
	 * Additional settings/configuration data for the current world.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous World Settings")
	TMap<FName, FGorgeousObjectVariableEntry> AdditionalGorgeousData;

	/** Trunk that persists serialized default payloads for this world's object variables. */
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous World Settings|Defaults", meta = (ShowOnlyInnerProperties))
	FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	FGorgeousAutoReplicationMixin AutoReplicationMixin;
	
	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
