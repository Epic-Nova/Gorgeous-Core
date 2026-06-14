// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GorgeousInputConsumer_I.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
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
	
	UFUNCTION(Blueprintable, BlueprintNativeEvent, Category = "Gorgeous Core|Common UI Foundation")
	bool HandleGorgeousInputAdvanced(FGameplayTag ActionTag, const FInputActionInstance& Instance);

	/**
	 * Returns the consumption priority. Higher values are processed first.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gorgeous Core|Common UI Foundation")
	int32 GetInputConsumerPriority() const;
};
