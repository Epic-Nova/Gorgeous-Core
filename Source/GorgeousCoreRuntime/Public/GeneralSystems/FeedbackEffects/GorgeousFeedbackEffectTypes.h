#pragma once

#include "GorgeousFeedbackEffectTypes.generated.h"

// Policy objects?


/** Defines how a feedback definition chooses which of its effects to play. */
UENUM(BlueprintType)
enum class EGorgeousFeedbackSelectionMode : uint8
{
	EAll            UMETA(DisplayName="Play All"),          /** Play every effect in the definition. */
	ERandom         UMETA(DisplayName="Random"),            /** Play a single randomly selected effect. */
	ESequential     UMETA(DisplayName="Sequential"),        /** Play effects one after another in order. */
	EWeightedRandom UMETA(DisplayName="Weighted Random"),   /** Play a single effect chosen by its weight. */
	ECustom         UMETA(DisplayName="Custom")             /** Defer the selection to custom logic. */
};

/** Defines where a feedback effect is anchored when it executes. */
UENUM(BlueprintType)
enum class EGorgeousFeedbackAttachmentMode : uint8
{
	None            UMETA(DisplayName="None"),              /** No attachment; the effect is not spatially anchored. */
	WorldLocation   UMETA(DisplayName="World Location"),    /** Anchor the effect at an explicit world transform. */
	TargetActor     UMETA(DisplayName="Target Actor"),      /** Anchor the effect to the context's target actor. */
	TargetSocket    UMETA(DisplayName="Target Socket"),     /** Anchor the effect to a named socket on the target. */
	Component       UMETA(DisplayName="Component")          /** Anchor the effect to an explicit scene component. */
};

/**
 * A reusable container of feedback effects.
 * Wraps an array of UGorgeousFeedbackEffect instances that should play together for a given event.
 */
USTRUCT(BlueprintType, meta = (ShowOnlyInnerProperties))
struct FGorgeousFeedbackDefinition
{
	GENERATED_BODY()

	/** The feedback effects that make up this definition. Each effect runs according to its own rules. */
	UPROPERTY(EditAnywhere, Instanced, Category="Feedback")
	TArray<TObjectPtr<class UGorgeousFeedbackEffect>> Effects;

};

/**
 * Carries the runtime information an effect needs in order to execute.
 * Populated by the system that triggers the feedback and passed to every effect's Execute.
 */
USTRUCT(BlueprintType)
struct FGorgeousFeedbackContext
{
	GENERATED_BODY()

	/** The object that initiated the feedback (e.g. the attacking actor or ability). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	TObjectPtr<UObject> Instigator;

	/** The object the feedback is directed at (e.g. the victim or impacted actor). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	TObjectPtr<UObject> Target;

	/** The actor that owns the feedback context, used as a world/fallback reference. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	TObjectPtr<AActor> OwnerActor;
	
	/** The scene component the effect should attach to when an attachment mode requires it. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	TObjectPtr<USceneComponent> AttachComponent;
	
	/** The world-space location the effect should spawn at when not attached to a component. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	FVector WorldLocation;

	/** The world-space rotation the effect should spawn with when not attached to a component. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	FRotator WorldRotation;
	
	/** Gameplay tags describing the situation, used by effects for tag-gated execution. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	FGameplayTagContainer ContextTags;

	/** Optional arbitrary user data forwarded alongside the feedback for custom effects. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Core|Feedback Effects") 
	TObjectPtr<UObject> UserData;
};
