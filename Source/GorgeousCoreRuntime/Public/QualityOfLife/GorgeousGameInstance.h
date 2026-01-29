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
//<-------------------------------------------------------------------------->
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
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
	, public IGorgeousQualityOfLifeNodeTarget_I
{
	GENERATED_BODY()

public:

	UGorgeousGameInstance();
	
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
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	//<-------------------------------------------------------------------------->
	
	/**
	 * Additional settings/configuration data for the current game instance.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Game Instance")
	TMap<FName, FGorgeousObjectVariableEntry> AdditionalGorgeousData;

	/** Serialized trunk that keeps authoritative default payloads for object variables authored on this instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous Game Instance|Defaults", meta = (ShowOnlyInnerProperties))
	FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;

private:
	void EnsureRootVariablesFallbackToGameInstance();
	void HandleRootRegistryChanged();

	bool bIsEnsuringRootFallback = false; // prevent reentrant root fallback loops from delegate broadcasts
	FDelegateHandle RootRegistryChangedHandle;
};
