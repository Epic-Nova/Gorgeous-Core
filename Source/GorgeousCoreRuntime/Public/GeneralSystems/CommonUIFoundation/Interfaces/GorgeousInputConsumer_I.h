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
#include "UObject/Interface.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInputConsumer_I.generated.h"
//<-------------------------------------------------------------------------->

UINTERFACE(MinimalAPI, BlueprintType, DisplayName = "Gorgeous Input Consumer Interface")
class UGorgeousInputConsumer_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any object (Widget, Actor, Component) that wants to consume tag-based input.
 * Bridged by AGorgeousHUD to systems like Enhanced Input.
 */
class GORGEOUSCORERUNTIME_API IGorgeousInputConsumer_I
{
	GENERATED_BODY()

public:
	/**
	 * Handles a Gorgeous input tag.
	 * @param ActionTag The conceptual gameplay tag for the action.
	 * @param Value The actual input data (digital or analog).
	 * @return True if the input was consumed and should not be propagated further.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gorgeous Core|Common UI Foundation")
	bool HandleGorgeousInput(FGameplayTag ActionTag, const FInputActionValue& Value);

	/**
	 * Handles an input action with its complete Enhanced Input instance data.
	 *
	 * @param ActionTag The conceptual gameplay tag for the action.
	 * @param Instance The Enhanced Input instance that carries the action state.
	 * @return True if the input was consumed and should not propagate further.
	 */
	UFUNCTION(Blueprintable, BlueprintNativeEvent, Category = "Gorgeous Core|Common UI Foundation")
	bool HandleGorgeousInputAdvanced(FGameplayTag ActionTag, const FInputActionInstance& Instance);

	/**
	 * Returns the consumption priority. Higher values are processed first.
	 *
	 * @return The priority used when ordering input consumers.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gorgeous Core|Common UI Foundation")
	int32 GetInputConsumerPriority() const;
};