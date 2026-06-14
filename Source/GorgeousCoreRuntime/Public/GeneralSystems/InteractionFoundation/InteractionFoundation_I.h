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

class GORGEOUSCORERUNTIME_API IInteractionFoundation_I
{
	GENERATED_BODY()

public:
	
	/**
	 * Requests the interaction types that the implementing object supports. This can be used by interaction systems to determine if and how to interact with this object.
	 * 
	 * @return An array of gameplay tags representing the interaction types supported by this object.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	FGameplayTagContainer RequestInteractionTags() const;

	/**
	 * Checks weather interaction with the implementing object is currently possible or not.
	 * 
	 * @return True if interaction is currently possible, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	bool CanInteract(AActor* InteractingActor) const;

	/**
	 * Requests additional metadata from the implementing object that can be used to determine how to interact with it or to provide additional context for the interaction.
	 * 
	 * @param InteractingActor The actor that is ending the focus on this object.
	 * @param bRefreshRequest Indicates if the Focus is a refresh request of the interaction data. Typically true after the first focus and false on the initial focus.
	 * @return An instanced struct containing the interaction metadata. The actual struct type can be defined by the implementing object and should be determined by the interaction tags it provides.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	FInstancedStruct Focus(AActor* InteractingActor, bool bRefreshRequest) const;

	/**
	 * Called when the focus on the implementing object is ending. This can be used to clean up any context or state that was set during the Focus function.
	 * 
	 * @param InteractingActor The actor that is ending the focus on this object.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void Unfocus(AActor* InteractingActor) const;
	
	/**
	 * Executes the interaction with the implementing object. This function should contain the actual logic of what happens when an interaction is performed with this object.
	 * 
	 * @param InteractingActor The actor that is performing the interaction. 
	 * @param HitResult The hit result of the trace or focus that triggered this interaction. This provides spatial context such as impact point and component.
	 */	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void Interact(AActor* InteractingActor, const FHitResult& HitResult);

	/**
	 * Called when the secondary interaction button is pressed. This can be used for context-specific interactions that are different from the primary interaction (e.g. aiming, alternate fire mode, etc.)
	 * 
	 * @param InteractingActor The actor that is performing the secondary interaction.
	 * @param KeyTag A gameplay tag representing the specific secondary interaction button that was pressed. 
	 * @param HitResult The hit result of the trace or focus that triggered this interaction. This provides spatial context such as impact point and component.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractSecondaryButton(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult);
	
	/**
	 * Executes a hold interaction with the implementing object. (e.g. charging, opening a door, etc.)
	 * Typically called from Event Tick while the interaction is being held. Does not indicate that the interaction is completed.
	 * 
	 * @param InteractingActor The actor that is performing the interaction. 
	 * @param HoldDuration The duration for which the interaction has been held. (e.g. how much a door is opened, how charged an attack is, etc.)
	 * @param RemainingDuration The remaining duration until the hold interaction is completed. (e.g. how much time is left until a door is fully opened, an attack is fully charged, etc.)
	 * @param KeyTag A gameplay tag representing the specific secondary interaction button that was pressed. Empty when the primary interaction button is affected.
	 * @param HitResult The hit result of the trace or focus that triggered this interaction. This provides spatial context such as impact point and component.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractHold(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult);
	
	/**
	 * Called when the interaction with the implementing object is ending. 
	 * 
	 * @param InteractingActor The actor that is ending the interaction.
	 * @param KeyTag A gameplay tag representing the specific secondary interaction button that was pressed. Empty when the primary interaction button is affected.
	 * @param HitResult The hit result of the trace or focus that triggered this interaction. This provides spatial context such as impact point and component.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractRelease(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Called when a hold interaction with the implementing object is canceled before completion. (e.g. releasing a door before it's fully opened, canceling a charge attack, etc.)
	 * 
	 * @param InteractingActor The actor that is canceling the hold interaction.
	 * @param HoldDuration The duration for which the interaction was held before cancellation.
	 * @param RemainingDuration The remaining duration until the hold interaction is completed. (e.g. how much time is left until a door is fully opened, an attack is fully charged, etc.)
	 * @param KeyTag A gameplay tag representing the specific secondary interaction button that was pressed. Empty when the primary interaction button is affected.
	 * @param HitResult The hit result of the trace or focus that triggered this interaction. This provides spatial context such as impact point and component.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void InteractCancel(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult);
};
