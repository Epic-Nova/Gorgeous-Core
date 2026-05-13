// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
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
	 * @return An instanced struct containing the interaction metadata. The actual struct type can be defined by the implementing object and should be determined by the interaction tags it provides.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	FInstancedStruct Focus(AActor* InteractingActor) const;

	/**
	 * Called when the focus on the implementing object is ending. This can be used to clean up any context or state that was set during the Focus function.
	 * 
	 * @param InteractingActor The actor that is ending the focus on this object. This can be used to clean up any context or state that was set during the Focus function.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void Unfocus(AActor* InteractingActor) const;
	
	/**
	 * Executes the interaction with the implementing object. This function should contain the actual logic of what happens when an interaction is performed with this object.
	 * 
	 * @param InteractingActor The actor that is performing the interaction. This can be used to determine the context of the interaction and to apply effects or changes to the interacting actor if necessary.
	 * @param HitResult The hit result of the trace or focus that triggered this interaction. This provides spatial context such as impact point and component.
	 */	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	void Interact(AActor* InteractingActor, const FHitResult& HitResult);
};
