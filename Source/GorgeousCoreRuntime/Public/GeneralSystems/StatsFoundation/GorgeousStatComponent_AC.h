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
//<--------------------------=== Module Includes ===------------------------->
#include "Components/ActorComponent.h"
#include "GorgeousStatFoundationStructures.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousStatComponent_AC.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Stat Component
| Functional Name: UGorgeousStatComponent_AC
| Parent Class: UActorComponent
| Class Suffix: _AC
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Component that adds Gorgeous Stat functionality to an actor. Automatically
| manages a replicated stat storage and integrates with the Signal Bridge.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class UGorgeousStatFoundationStorage_OV;
//<------------------------------------------------------------->
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent), DisplayName = "Gorgeous Stat Component",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/GorgeousStatComponent_AC",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousStatComponent_AC : public UActorComponent
{
	GENERATED_BODY()

	// Constructs the Stat Foundation actor component.
public:
	UGorgeousStatComponent_AC();

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Initializes the component after play begins.
	virtual void BeginPlay() override;

	// Registers replicated component properties.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Initializes component state after registration.
	virtual void OnRegister() override;

	// Releases component state before unregistration.
	virtual void OnUnregister() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Returns the underlying replicated statistic storage.
	 *
	 * @return The component's statistic storage object.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	UGorgeousStatFoundationStorage_OV* GetStatStorage() const { return StatStorage; }

	/**
	 * Sets a statistic value directly. Authority only.
	 *
	 * @param Tag The statistic to update.
	 * @param Value The value to assign.
	 * @param Requester The controller requesting the update, if any.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void SetStat(FGameplayTag Tag, float Value, AGorgeousPlayerController* Requester = nullptr);

	/**
	 * Modifies a statistic value by an amount. Authority only.
	 *
	 * @param Tag The statistic to update.
	 * @param Delta The amount to add to the current value.
	 * @param Requester The controller requesting the update, if any.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void ModifyStat(FGameplayTag Tag, float Delta, AGorgeousPlayerController* Requester = nullptr);

	/**
	 * Returns a statistic value.
	 *
	 * @param Tag The statistic to read.
	 * @return The current value of the statistic.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	float GetStat(FGameplayTag Tag) const;

	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:

	/**
	 * Returns the number of active Stat Foundation components in the world.
	 *
	 * @return The number of active statistic components.
	 */
	static int32 GetTotalActiveComponents();

	/**
	 * Returns the number of statistic modifiers applied since the process started.
	 *
	 * @return The number of applied statistic modifiers.
	 */
	static int32 GetTotalModifiersApplied();
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
protected:
	// Internal storage object, managed by Gorgeous Auto Replication.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<UGorgeousStatFoundationStorage_OV> StatStorage;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
protected:
	/**
	 * Handles a statistic value change reported by storage.
	 *
	 * @param Tag The statistic that changed.
	 * @param NewValue The new statistic value.
	 */
	UFUNCTION()
	void HandleStatChanged(FGameplayTag Tag, float NewValue);

	/**
	 * Handles a signal received from the Signal Bridge.
	 *
	 * @param SignalTag The received signal tag.
	 * @param Payload The received signal payload.
	 */
	UFUNCTION()
	void OnSignalReceived(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
protected:
	/** Initializes the storage object. */
	void InitializeStatStorage();

	/** Registers listeners for all stats that have signals configured. */
	void RegisterSignalListeners();
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};
