// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SignalBridgeListenerStructures.generated.h"

class AGorgeousPlayerController;
class AActor;

/** Wrapper struct to allow TMap of TArray in UPROPERTY. */
USTRUCT()
struct FGorgeousControllerArray_S
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AGorgeousPlayerController>> Controllers;
};

/**
 * An authority-only tracking struct for the Signal Bridge.
 * Stores a list of controllers that are currently listening to a specific tag.
 */
USTRUCT()
struct FGorgeousSignalBridgeListener_S
{
	GENERATED_BODY()

public:
	
	/** The tag this registry is for. */
	UPROPERTY()
	FGameplayTag ForDispatchTag;

	/** Controllers listening to ALL broadcasts of this tag. */
	UPROPERTY()
	TArray<TObjectPtr<AGorgeousPlayerController>> RegisteredListeners;

	/** Controllers listening only to specific actors for this tag. */
	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FGorgeousControllerArray_S> ActorScopedListeners;
};
