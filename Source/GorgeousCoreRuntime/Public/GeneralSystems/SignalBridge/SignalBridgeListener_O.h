// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "SignalBridgeListener_O.generated.h"

class AGorgeousPlayerController;

/**
 * An authority-only tracking object for the Signal Bridge.
 * Stores a list of controllers that are currently listening to a specific tag.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API USignalBridgeListener_O : public UObject
{
	GENERATED_BODY()

public:
	
	/** The tag this registry is for. */
	UPROPERTY()
	FGameplayTag ForDispatchTag;

	/** The list of player controllers that should receive dispatches for this tag. */
	UPROPERTY()
	TArray<TObjectPtr<AGorgeousPlayerController>> RegisteredListeners;
};
