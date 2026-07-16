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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//<--------------------------=== Module Includes ===------------------------->
#include "InteractionFoundation_I.generated.h"
//<-------------------------------------------------------------------------->

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractionFoundation_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract that actors implement to participate in the Interaction Foundation, exposing their
 * interaction tags, focus data and interaction execution to the foundation library.
 *
 * @author Nils Bergemann
 */
class GORGEOUSCORERUNTIME_API IInteractionFoundation_I
{
	GENERATED_BODY()

public:

	/**
	 * Reports the interaction tags this object supports.
	 *
	 * @return Gameplay tags describing the interaction types the object provides.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	FGameplayTagContainer RequestInteractionTags() const;

	/**
	 * Reports whether the interacting actor may currently interact with this object.
	 *
	 * @param InteractingActor The actor attempting the interaction.
	 * @return True if interaction is currently permitted, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	bool CanInteract(AActor* InteractingActor) const;

	/**
	 * Provides the focus metadata used to render this object's interaction prompt.
	 *
	 * @param InteractingActor The actor requesting focus.
	 * @param bRefreshRequest True when this is a refresh of an ongoing focus rather than the initial focus.
	 * @return Instanced struct holding the object-defined focus payload.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	FInstancedStruct Focus(AActor* InteractingActor, bool bRefreshRequest) const;

	/**
	 * Notifies the object that focus has ended so it can release any focus-specific state.
	 *
	 * @param InteractingActor The actor ending focus.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void Unfocus(AActor* InteractingActor) const;

	/**
	 * Executes the primary interaction logic for this object.
	 *
	 * @param InteractingActor The actor performing the interaction.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void Interact(AActor* InteractingActor, const FHitResult& HitResult);

	/**
	 * Executes a secondary-button interaction (e.g. aim, alternate action) for this object.
	 *
	 * @param InteractingActor The actor performing the interaction.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractSecondaryButton(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Drives a hold interaction (e.g. charging, opening) called repeatedly while the button is held.
	 *
	 * @param InteractingActor The actor performing the interaction.
	 * @param HoldDuration Duration the interaction has been held so far.
	 * @param RemainingDuration Remaining duration until the hold completes.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button, empty for the primary button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractHold(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Called when the hold button is released, completing a hold interaction.
	 *
	 * @param InteractingActor The actor performing the interaction.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button, empty for the primary button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractRelease(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Called when a hold interaction is canceled before completion.
	 *
	 * @param InteractingActor The actor canceling the interaction.
	 * @param HoldDuration Duration the interaction had been held before cancellation.
	 * @param RemainingDuration Remaining duration until the hold would have completed.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button, empty for the primary button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractCancel(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult);
};
