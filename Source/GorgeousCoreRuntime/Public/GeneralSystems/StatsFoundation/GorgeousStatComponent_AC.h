// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GorgeousStatFoundationStructures.h"
#include "GorgeousStatComponent_AC.generated.h"

class UGorgeousStatFoundationStorage_OV;

/**
 * Component that adds Gorgeous Stat functionality to an actor.
 * Automatically manages a replicated stat storage and integrates with the Signal Bridge.
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent), DisplayName = "Gorgeous Stat Component")
class GORGEOUSCORERUNTIME_API UGorgeousStatComponent_AC : public UActorComponent
{
	GENERATED_BODY()

public:
	UGorgeousStatComponent_AC();

	virtual void BeginPlay() override;

	/** Gets the underlying replicated storage. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	UGorgeousStatFoundationStorage_OV* GetStatStorage() const { return StatStorage; }

	/** Sets a stat value directly (Authority Only). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void SetStat(FGameplayTag Tag, float Value, AGorgeousPlayerController* Requester = nullptr);

	/** Modifies a stat value by an amount (Authority Only). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats")
	void ModifyStat(FGameplayTag Tag, float Delta, AGorgeousPlayerController* Requester = nullptr);

	/** Gets a stat value. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats")
	float GetStat(FGameplayTag Tag) const;

	/** Total active Stat Components currently alive in the world. */
	static int32 GetTotalActiveComponents();

	/** Total modifiers applied across all components since boot. */
	static int32 GetTotalModifiersApplied();

protected:
	/** Internal storage object, managed by Gorgeous Auto Replication. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<UGorgeousStatFoundationStorage_OV> StatStorage;

	/** Callback for stat changes from storage. */
	UFUNCTION()
	void HandleStatChanged(FGameplayTag Tag, float NewValue);

	/** Callback for signals from the Signal Bridge. */
	UFUNCTION()
	void OnSignalReceived(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);

	/** Initializes the storage object. */
	void InitializeStatStorage();

	/** Registers listeners for all stats that have signals configured. */
	void RegisterSignalListeners();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};
