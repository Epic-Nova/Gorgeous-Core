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
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GorgeousStatFoundationListenerStructures.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStructures.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousStatFoundationStorage_OV.generated.h"
//<-------------------------------------------------------------------------->

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGorgeousStatChangedDelegate, FGameplayTag, StatTag, float, NewValue);

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Stat Foundation Storage
| Functional Name: UGorgeousStatFoundationStorage_OV
| Parent Class: UGorgeousObjectVariable
| Class Suffix: _OV
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Global replicated storage for ALL Gorgeous stats across ALL actors.
| Managed as a singleton in the GameState network root. Uses custom payload
| replication to ensure clients only receive stats they are authorized to
| see.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/GorgeousStatFoundationStorage_OV",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousStatFoundationStorage_OV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

	// Constructs the replicated Stat Foundation storage object.
public:
	UGorgeousStatFoundationStorage_OV();

	//<============================--- Overrides ---============================>
	#pragma region Overrides
protected:

	// Activates replication-specific Stat Foundation state.
	virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override;

	// Builds an access-filtered custom replication payload.
	virtual bool BuildCustomAutoReplicationPayload_Implementation(FName PropertyName, TArray<uint8>& OutPayload, const struct FGorgeousAutoReplicationConditionContext& ConditionContext) override;

	// Applies an access-filtered custom replication payload.
	virtual bool ApplyCustomAutoReplicationPayload_Implementation(FName PropertyName, const TArray<uint8>& Payload, const struct FGorgeousAutoReplicationConditionContext& ConditionContext) override;

	// Determines whether a controller may access an AutoReplication property.
	virtual bool CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Sets a statistic value for an actor. Authority only.
	 *
	 * @param Actor The actor that owns the statistic.
	 * @param Tag The statistic to set.
	 * @param Value The value to assign.
	 * @param Requester The controller requesting the update, if any.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void SetStatValue(AActor* Actor, FGameplayTag Tag, float Value, AGorgeousPlayerController* Requester = nullptr);

	/**
	 * Returns a statistic value for an actor.
	 *
	 * @param Actor The actor that owns the statistic.
	 * @param Tag The statistic to read.
	 * @return The current value of the statistic.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	float GetStatValue(AActor* Actor, FGameplayTag Tag) const;

	/**
	 * Determines whether an actor has a statistic with the given tag.
	 *
	 * @param Actor The actor to inspect.
	 * @param Tag The statistic to find.
	 * @return True when the statistic exists, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	bool HasStat(AActor* Actor, FGameplayTag Tag) const;

	/**
	 * Adds a controller to an actor statistic's allowed list.
	 *
	 * @param Actor The actor that owns the statistic.
	 * @param Tag The statistic whose access list is updated.
	 * @param Controller The controller to allow.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void AddAllowedController(AActor* Actor, FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Removes a controller from an actor statistic's allowed list.
	 *
	 * @param Actor The actor that owns the statistic.
	 * @param Tag The statistic whose access list is updated.
	 * @param Controller The controller to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void RemoveAllowedController(AActor* Actor, FGameplayTag Tag, AGorgeousPlayerController* Controller);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Broadcast when any stat changes.
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|Stats")
	FGorgeousStatChangedDelegate OnStatChanged;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- Variables ---============================>
	#pragma region Variables
protected:

	/**
	 * Internal storage map: Actor -> Stats.
	 * Replicated via Custom Payload to allow per-controller filtering.
	 */
	TMap<TWeakObjectPtr<AActor>, FGorgeousStatValues_S> AllActorStats;

	// Authority-only tracking of listeners.
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, FGorgeousStatListener_S> DictionaryAssociations;

	/** Local-only cache for change detection on clients. */
	TMap<TWeakObjectPtr<AActor>, FGorgeousStatValues_S> LastKnownValues;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
protected:
	/** Helper to evaluate access for a specific controller, actor, and stat. */
	bool EvaluateStatAccess(AGorgeousPlayerController* Controller, AActor* Actor, FGameplayTag Tag) const;

	/** Helper to find or create a listener registry for an actor (Authority only). */
	struct FGorgeousStatListener_S& GetOrCreateListenerRegistry(AActor* Actor);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};