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
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "SignalBridgeListenerStructures.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class AGorgeousPlayerController;
class AActor;
//<------------------------------------------------------------->
/**
 * Wraps a controller array so it can be stored in a reflected map.
 *
 * @author Nils Bergemann
 */
USTRUCT()
struct FGorgeousControllerArray_S
{
	GENERATED_BODY()

	// Controllers contained by this reflected array wrapper.
	UPROPERTY()
	TArray<TObjectPtr<AGorgeousPlayerController>> Controllers;
};

/**
 * An authority-only tracking struct for the Signal Bridge.
 * Stores a list of controllers that are currently listening to a specific tag.
 *
 * @author Nils Bergemann
 */
USTRUCT()
struct FGorgeousSignalBridgeListener_S
{
	GENERATED_BODY()

public:

	// The tag this registry is for.
	UPROPERTY()
	FGameplayTag ForDispatchTag;

	// Controllers listening to ALL broadcasts of this tag.
	UPROPERTY()
	TArray<TObjectPtr<AGorgeousPlayerController>> RegisteredListeners;

	// Controllers listening only to specific actors for this tag.
	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FGorgeousControllerArray_S> ActorScopedListeners;
};