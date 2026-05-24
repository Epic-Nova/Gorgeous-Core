// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GorgeousStatFoundationListenerStructures.generated.h"

class AGorgeousPlayerController;
class AActor;

/** 
 * Wrapper struct to allow TMap of TArray in UPROPERTY. 
 * Mirrored from Signal Bridge system for architectural consistency.
 */
USTRUCT()
struct FGorgeousStatControllerArray_S
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AGorgeousPlayerController>> Controllers;
};

/**
 * An authority-only tracking struct for the Gorgeous Stat System.
 * Stores which controllers are currently interested in specific actors' stats.
 */
USTRUCT()
struct FGorgeousStatListener_S
{
	GENERATED_BODY()

public:
	/** The actor this listener registry is tracking. */
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	/** Controllers that want to receive all stat updates for this actor. */
	UPROPERTY()
	TArray<TObjectPtr<AGorgeousPlayerController>> RegisteredListeners;

	/** Controllers that only want specific stats for this actor. */
	UPROPERTY()
	TMap<FGameplayTag, FGorgeousStatControllerArray_S> TagSpecificListeners;
};
