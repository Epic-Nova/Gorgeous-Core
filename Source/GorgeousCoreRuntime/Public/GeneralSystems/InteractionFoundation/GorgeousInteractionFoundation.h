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
#include "GeneralSystems/InteractionFoundation/InteractionFoundation_I.h"
#include "GeneralSystems/InteractionFoundation/InteractionFoundationStructures.h"
#include "GeneralSystems/DebugAssist/DebugAssistStructures.h"
#include "GameplayTagContainer.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousInteractionFoundation.generated.h"
//<-------------------------------------------------------------------------->
//@TODO: Make it a UGorgeous
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Interaction Foundation
| Functional Name: UGorgeousInteractionFoundation
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Function Library providing helper functions for 
| interaction related functionality in Blueprints.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInteractionFoundation : public UGorgeous
{
    GENERATED_BODY()

public:

    /**
     * Tries to request the interaction tags from the target actor. Returns false if the target actor does not implement the interaction interface or if the request failed for any reason.
     * 
     * @param TargetActor The actor to request the interaction tags from.
     * @param OutInteractionTags The output parameter that will contain the requested interaction tags if the function returns true.
     * 
     * @return True if the interaction tags were successfully requested, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
    static bool TryRequestInteractionTags(AActor* TargetActor, FGameplayTagContainer& OutInteractionTags);
    
    /**  
     * Tries to check if interaction with the target actor is currently possible. Returns false if the target actor does not implement the interaction interface or if the check failed for any reason.
     * 
     * @param TargetActor The actor to check for interaction possibility.
     * @param InteractingActor The actor that is trying to interact with the target actor.
     * @param bCanInteract The output parameter that will contain the result of the interaction possibility check if the function returns true.
     * 
     * @return True if the interaction possibility was successfully checked, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
    static bool TryCanInteract(AActor* TargetActor, AActor* InteractingActor, bool& bCanInteract);

    /** 
     * Tries to request the focus data from the target actor. Returns false if the target actor does not implement the interaction interface or if the request failed for any reason.
     * 
     * @param TargetActor The actor to request the focus data from.
     * @param InteractingActor The actor that is trying to focus on the target actor.
     * @param bAutoSendUnfocus Automatically calls Unfocus on the target actor with the interacting actor as parameter if the focus request is successful.
     * @param OutFocusData The output parameter that will contain the requested focus data if the function returns true.
     * @param bOutWasRefreshRequest Indicates if the focus request is a refresh request of the interaction data. Typically true after the first focus and false on the initial focus.
     * @param bOutWasUnfocus Indicates if the focus request resulted in an unfocus action after the last focused actor lost focus.
     * 
     * @return True if the focus data was successfully requested, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
    static bool TryFocus(AActor* TargetActor, AActor* InteractingActor, bool bAutoSendUnfocus, FInstancedStruct& OutFocusData, bool& bOutWasRefreshRequest, bool& bOutWasUnfocus);

    /** 
     * Tries to execute the interaction with the target actor. Returns false if the target actor does not implement the interaction interface or if the interaction failed for any reason.
     * 
     * @param TargetActor The actor to interact with.
     * @param InteractingActor The actor that is trying to interact with the target actor.
     * @param HitResult The hit result of the trace or focus that triggered this interaction. This provides spatial context such as impact point and component.
     * 
     * @return True if the interaction was successfully executed, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
    static bool TryInteract(AActor* TargetActor, AActor* InteractingActor, const FHitResult& HitResult);

    /** 
     * Tries to perform a sphere trace interaction based on the provided trace parameters and interaction tag. Returns false if the trace did not hit any valid interactable target or if the interaction failed for any reason.
     * 
     * @param WorldContextObject The world context object for locating the world in which to perform the trace.
     * @param HitResult The hit result of a previous trace or focus that defined the interaction target.
     * @param InteractionTag The gameplay tag representing the type of interaction to perform.
     * 
     * @return True if the hit result contains a valid interactable target and the interaction was successfully executed, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Try Interact (Sphere Trace)"))
    static bool TrySphereTraceInteract(const UObject* WorldContextObject,
        const FHitResult& HitResult,
        FGameplayTag InteractionTag);

    /** 
     * Tries to perform a sphere trace focus based on the provided trace parameters and interaction tag. Returns false if the trace did not hit any valid interactable target or if the focus request failed for any reason.
     * 
     * @param WorldContextObject The world context object for locating the world in which to perform the trace.
     * @param TraceParameters The parameters defining the sphere trace, such as start and end locations, radius, and collision channel.
     * @param InteractionTag The gameplay tag representing the type of interaction to focus on.
     * @param bAutoSendUnfocus Automatically calls Unfocus on the target actor with the interacting actor as parameter if the focus request is successful.
     * @param OutFocusData The output parameter that will contain the focus data provided by the hit actor if the function returns true.
     * @param OutHitResult The output parameter that will contain the hit result of the trace if the function returns true.
     * @param bOutWasRefreshRequest Indicates if the interaction attempt is a refresh request of the interaction data. Typically true after the first focus and false on the initial focus.
     * @param bOutWasUnfocus Indicates if the focus request resulted in an unfocus action after the last focused actor lost focus.

     * 
     * @return True if a valid interactable target was hit and the focus data was successfully requested, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Try Focus (Sphere Trace)"))
    static bool TrySphereTraceFocus(const UObject* WorldContextObject,
        const FGorgeousInteractionSphereTraceParameters& TraceParameters,
        FGameplayTag InteractionTag,
        const bool bAutoSendUnfocus,
        FInstancedStruct& OutFocusData,
        bool& bOutWasRefreshRequest,
        bool& bOutWasUnfocus,
        FHitResult& OutHitResult);
    
private:
    
    // Map of actors currently in the focus of another actor.
    static TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> InteractionActors;
};

using UGT_IF = UGorgeousInteractionFoundation;
