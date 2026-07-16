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
#include "GeneralSystems/InteractionFoundation/InteractionFoundationStructures.h"
#include "GeneralSystems/InteractionFoundation/InteractionFoundation_I.h"
//----------------=== Third Party & Miscellaneous Includes ===---------------
#include "GorgeousInteractionFoundation.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Interaction Foundation
| Functional Name: UGorgeousInteractionFoundation
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Blueprint function library exposing helper functions for
| interaction related functionality in Blueprints.
<--------------------------------------------------------------------------->
<===========================================================================>
*/

UCLASS(Blueprintable, BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/InteractionFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/InteractionFoundation/GorgeousInteractionFoundation",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/InteractionFoundation/Examples/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousInteractionFoundation : public UGorgeous
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
public:

	/** Total successful interactions executed through the foundation. */
	static int32 GetTotalSuccessfulInteractions();

	/** Total interactions rejected because the target did not support the requested tag. */
	static int32 GetTotalRejectedInteractions();

	/** Total interactions rejected because the interacting actor lacked permission. */
	static int32 GetTotalPermissionDeniedInteractions();

	/** Increments the successful interaction counter. */
	static void IncrementSuccessfulInteractions();

	/** Increments the rejected interaction counter. */
	static void IncrementRejectedInteractions();

	/** Increments the permission-denied interaction counter. */
	static void IncrementPermissionDeniedInteractions();

	/** Number of actors currently tracked for interaction focus. */
	static int32 GetActiveTrackedInteractionActors();

	/**
	 * Retrieves the interaction tags from a target actor that implements the Interaction Foundation interface.
	 *
	 * @param TargetActor The actor to request the interaction tags from.
	 * @param OutInteractionTags Receives the requested interaction tags when the call succeeds.
	 * @return True if the tags were requested successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryRequestInteractionTags(AActor* TargetActor, FGameplayTagContainer& OutInteractionTags);

	/**
	 * Checks whether the interacting actor can currently interact with the target actor.
	 *
	 * @param TargetActor The actor to check for interaction possibility.
	 * @param InteractingActor The actor attempting the interaction.
	 * @param bCanInteract Receives the result of the possibility check when the call succeeds.
	 * @return True if the check was performed successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryCanInteract(AActor* TargetActor, AActor* InteractingActor, bool& bCanInteract);

	/**
	 * Requests focus data from a target actor, managing the tracked focus relationship for the interacting actor.
	 *
	 * @param TargetActor The actor to request focus data from.
	 * @param InteractingActor The actor requesting focus.
	 * @param bAutoSendUnfocus Sends Unfocus to the previously focused actor when a new focus is established.
	 * @param OutFocusData Receives the requested focus data when the call succeeds.
	 * @param bOutWasRefreshRequest True when the focus was a refresh of an already focused actor.
	 * @param bOutWasUnfocus True when the previous focus was released as part of this request.
	 * @return True if the focus data was requested successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryFocus(AActor* TargetActor, AActor* InteractingActor, bool bAutoSendUnfocus, FInstancedStruct& OutFocusData, bool& bOutWasRefreshRequest, bool& bOutWasUnfocus);

	/**
	 * Executes the primary interaction with a target actor.
	 *
	 * @param TargetActor The actor to interact with.
	 * @param InteractingActor The actor performing the interaction.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 * @return True if the interaction was executed successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryInteract(AActor* TargetActor, AActor* InteractingActor, const FHitResult& HitResult);

	/**
	 * Executes a secondary-button interaction with a target actor.
	 *
	 * @param TargetActor The actor to interact with.
	 * @param InteractingActor The actor performing the interaction.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 * @return True if the interaction was executed successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryInteractSecondaryButton(AActor* TargetActor, AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Executes a hold interaction with a target actor.
	 *
	 * @param TargetActor The actor to interact with.
	 * @param InteractingActor The actor performing the interaction.
	 * @param HoldDuration Duration the interaction has been held so far.
	 * @param RemainingDuration Remaining duration until the hold completes.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button, empty for the primary button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 * @return True if the interaction was executed successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryInteractHold(AActor* TargetActor, AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Executes a release interaction with a target actor.
	 *
	 * @param TargetActor The actor to interact with.
	 * @param InteractingActor The actor performing the interaction.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button, empty for the primary button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 * @return True if the interaction was executed successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryInteractRelease(AActor* TargetActor, AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Executes a cancel interaction with a target actor, aborting an in-progress hold.
	 *
	 * @param TargetActor The actor to interact with.
	 * @param InteractingActor The actor performing the interaction.
	 * @param HoldDuration Duration the interaction had been held before cancellation.
	 * @param RemainingDuration Remaining duration until the hold would have completed.
	 * @param KeyTag Gameplay tag identifying the secondary interaction button, empty for the primary button.
	 * @param HitResult Trace or focus hit result that triggered the interaction.
	 * @return True if the interaction was executed successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation")
	static bool TryInteractCancel(AActor* TargetActor, AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	/**
	 * Performs a sphere trace and executes the matching interaction on the first valid target supporting the interaction tag.
	 *
	 * @param WorldContextObject World context used to locate the world for the trace.
	 * @param HitResult Prior trace or focus hit result that defined the interaction target.
	 * @param InteractionTag Gameplay tag representing the type of interaction to perform.
	 * @return True if a valid target was hit and the interaction executed, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Interaction Foundation", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Try Interact (Sphere Trace)"))
	static bool TrySphereTraceInteract(const UObject* WorldContextObject,
		const FHitResult& HitResult,
		FGameplayTag InteractionTag);

	/**
	 * Performs a sphere trace and requests focus data from the first valid target supporting the interaction tag.
	 *
	 * @param WorldContextObject World context used to locate the world for the trace.
	 * @param TraceParameters Sphere trace configuration (start, end, radius, channel, etc.).
	 * @param InteractionTag Gameplay tag representing the type of interaction to focus on.
	 * @param bAutoSendUnfocus Sends Unfocus to the previously focused actor when a new focus is established.
	 * @param OutFocusData Receives the focus data when the call succeeds.
	 * @param OutHitResult Receives the trace hit result when the call succeeds.
	 * @param bOutWasRefreshRequest True when the focus was a refresh of an already focused actor.
	 * @param bOutWasUnfocus True when the previous focus was released as part of this request.
	 * @return True if a valid target was hit and focus data requested, false otherwise.
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
	//<------------------------------------------------------------------------->

	//<============================--- C++ Only ---=============================>
private:

	// Maps each interacting actor to the actor it is currently focused on.
	static TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> InteractionActors;
	//<------------------------------------------------------------------------->
};

using UGT_IF = UGorgeousInteractionFoundation;
