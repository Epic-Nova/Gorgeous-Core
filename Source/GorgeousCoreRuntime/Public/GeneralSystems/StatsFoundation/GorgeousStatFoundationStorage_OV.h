// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GameplayTagContainer.h"
#include "GorgeousStatFoundationListenerStructures.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStructures.h"
#include "GorgeousStatFoundationStorage_OV.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGorgeousStatChangedDelegate, FGameplayTag, StatTag, float, NewValue);

/**
 * Global replicated storage for ALL Gorgeous stats across ALL actors.
 * Managed as a singleton in the GameState network root.
 * Uses custom payload replication to ensure clients only receive stats they are authorized to see.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousStatFoundationStorage_OV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:
	UGorgeousStatFoundationStorage_OV();

	/** Sets a stat value for a specific actor. Authority only. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void SetStatValue(AActor* Actor, FGameplayTag Tag, float Value, AGorgeousPlayerController* Requester = nullptr);

	/** Gets a stat value for a specific actor. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	float GetStatValue(AActor* Actor, FGameplayTag Tag) const;

	/** Checks if a stat exists for an actor. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	bool HasStat(AActor* Actor, FGameplayTag Tag) const;

	/** Adds a specific controller to the allowed list for an actor's stat. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void AddAllowedController(AActor* Actor, FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/** Removes a specific controller from the allowed list for an actor's stat. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void RemoveAllowedController(AActor* Actor, FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/** Broadcast when any stat changes. */
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|Stats")
	FGorgeousStatChangedDelegate OnStatChanged;

protected:
	//~ Begin UGorgeousObjectVariable Interface
	virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override;
	virtual bool BuildCustomAutoReplicationPayload_Implementation(FName PropertyName, TArray<uint8>& OutPayload, const struct FGorgeousAutoReplicationConditionContext& ConditionContext) override;
	virtual bool ApplyCustomAutoReplicationPayload_Implementation(FName PropertyName, const TArray<uint8>& Payload, const struct FGorgeousAutoReplicationConditionContext& ConditionContext) override;
	virtual bool CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const override;
	//~ End UGorgeousObjectVariable Interface

	/** 
	 * Internal storage map: Actor -> Stats. 
	 * Replicated via Custom Payload to allow per-controller filtering.
	 */
	TMap<TWeakObjectPtr<AActor>, FGorgeousStatValues_S> AllActorStats;

	/** Authority-only tracking of listeners. */
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, FGorgeousStatListener_S> DictionaryAssociations;

	/** Local-only cache for change detection on clients. */
	TMap<TWeakObjectPtr<AActor>, FGorgeousStatValues_S> LastKnownValues;

	/** Helper to evaluate access for a specific controller, actor, and stat. */
	bool EvaluateStatAccess(AGorgeousPlayerController* Controller, AActor* Actor, FGameplayTag Tag) const;

	/** Helper to find or create a listener registry for an actor (Authority only). */
	struct FGorgeousStatListener_S& GetOrCreateListenerRegistry(AActor* Actor);
};
