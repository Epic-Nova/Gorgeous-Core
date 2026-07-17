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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousFeedbackEffectTypes.generated.h"
//<-------------------------------------------------------------------------->

// Policy objects?


/**
 * Defines how a feedback definition chooses which of its effects to play.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousFeedbackSelectionMode : uint8
{
	// Plays every effect in the definition.
	EAll UMETA(DisplayName="Play All"),
	// Plays a single randomly selected effect.
	ERandom UMETA(DisplayName="Random"),
	// Plays effects one after another in order.
	ESequential UMETA(DisplayName="Sequential"),
	// Plays a single effect chosen by its weight.
	EWeightedRandom UMETA(DisplayName="Weighted Random"),
	// Defers selection to custom logic.
	ECustom UMETA(DisplayName="Custom")
};

/**
 * Defines where a feedback effect is anchored when it executes.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousFeedbackAttachmentMode : uint8
{
	// Does not spatially anchor the effect.
	None UMETA(DisplayName="None"),
	// Anchors the effect at an explicit world transform.
	WorldLocation UMETA(DisplayName="World Location"),
	// Anchors the effect to the context target actor.
	TargetActor UMETA(DisplayName="Target Actor"),
	// Anchors the effect to a named target socket.
	TargetSocket UMETA(DisplayName="Target Socket"),
	// Anchors the effect to an explicit scene component.
	Component UMETA(DisplayName="Component")
};

/**
 * A reusable container of feedback effects.
 * Wraps an array of UGorgeousFeedbackEffect instances that should play together for a given event.
 * The SelectionMode controls how the contained effects are chosen at play time, consuming the
 * per-effect Weight and Tags properties; FilterTags further narrows the candidate set.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType, meta = (ShowOnlyInnerProperties))
struct FGorgeousFeedbackDefinition
{
	GENERATED_BODY()

	// The feedback effects that make up this definition. Each effect runs according to its own rules.
	UPROPERTY(EditAnywhere, Instanced, Category="Feedback")
	TArray<TObjectPtr<class UGorgeousFeedbackEffect>> Effects;

	// How the definition chooses which of its effects to play. Consumes the per-effect Weight and Tags.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feedback")
	EGorgeousFeedbackSelectionMode SelectionMode = EGorgeousFeedbackSelectionMode::EAll;

	// Optional tag filter. Effects whose Tags do not contain any of these are excluded before selection.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feedback")
	FGameplayTagContainer FilterTags;

	/**
	 * Custom selection policy, only consulted when SelectionMode is ECustom. Receives the
	 * filter-passed candidates and returns the final, ordered set to play (absolute freedom:
	 * drop, reorder, duplicate or synthesize effects). Ignored for all other modes.
	 */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="Feedback", meta=(EditCondition="SelectionMode == EGorgeousFeedbackSelectionMode::ECustom"))
	TObjectPtr<class UGorgeousFeedbackSelector> Selector;

};

/**
 * Carries the runtime information an effect needs in order to execute.
 * Populated by the system that triggers the feedback and passed to every effect's Execute.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousFeedbackContext
{
	GENERATED_BODY()

	// The object that initiated the feedback (e.g. the attacking actor or ability).
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	TObjectPtr<UObject> Instigator;

	// The object the feedback is directed at (e.g. the victim or impacted actor).
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	TObjectPtr<UObject> Target;

	// The actor that owns the feedback context, used as a world/fallback reference.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	TObjectPtr<AActor> OwnerActor;

	// The scene component the effect should attach to when an attachment mode requires it.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	TObjectPtr<USceneComponent> AttachComponent;

	// The world-space location the effect should spawn at when not attached to a component.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	FVector WorldLocation;

	// The world-space rotation the effect should spawn with when not attached to a component.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	FRotator WorldRotation;

	// Gameplay tags describing the situation, used by effects for tag-gated execution.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	FGameplayTagContainer ContextTags;

	// Optional arbitrary user data forwarded alongside the feedback for custom effects.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects")
	TObjectPtr<UObject> UserData;

	/**
	 * Casts the context user data to the requested type.
	 *
	 * @return The user data cast to T, or nullptr when it is unset or incompatible.
	 */
	template<typename T>
	T* GetUserData() const
	{
		return Cast<T>(UserData);
	}
};