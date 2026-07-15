#pragma once

#include "GorgeousBaseWorldContextUObject.h"
#include "GorgeousFeedbackEffectTypes.h"
#include "GorgeousFeedbackEffect.generated.h"

class APlayerController;



/**
 * Abstract base class for all feedback effects.
 *
 * A feedback effect is a self-contained, asset-configurable unit of audiovisual or haptic
 * response (sound, camera shake, particle burst, force feedback, etc.). Effects are executed
 * through Execute using an FGorgeousFeedbackContext that supplies the runtime situation, and
 * can be gated through tags, a probability and scheduling options. Concrete effects subclass
 * this and implement Execute_Implementation (and optionally CanExecute_Implementation).
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousFeedbackEffect : public UGorgeousBaseWorldContextUObject
{
    GENERATED_BODY()

public:

    /** Whether this effect is enabled and allowed to run. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General")
    bool bEnabled = true;

    /** Optional name shown in the editor for readability. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General")
    FText DisplayName;

    /** Developer notes describing the intent of this effect. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General", meta=(MultiLine))
    FText Description;

    /** Probability of execution, in the range 0 (never) to 1 (always). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Execution", meta=(ClampMin="0", ClampMax="1"))
    float Chance = 1.f;

    /** Delay in seconds before the effect is executed after being triggered. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Execution", meta=(Units="s"))
    float Delay = 0.f;

    /** Additional random delay in seconds added on top of Delay. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Execution", meta=(Units="s"))
    float RandomDelay = 0.f;

    /** Number of times the effect is repeated when executed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Execution", meta=(ClampMin="1"))
    int32 RepeatCount = 1;

    /** Time in seconds between repeats. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Execution", meta=(Units="s"))
    float RepeatInterval = 0.f;

    /** Weight used when this effect is selected through a weighted random rule. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Selection")
    float Weight = 1.f;

    /** Gameplay tags describing this effect, useful for filtering and analytics. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tags")
    FGameplayTagContainer Tags;

    /** Tags that must all be present in the execution context for this effect to run. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tags")
    FGameplayTagContainer RequiredTags;

    /** Tags that, if present in the execution context, block this effect from running. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tags")
    FGameplayTagContainer BlockedTags;

    /** How the effect is anchored in the world when it executes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attachment")
    EGorgeousFeedbackAttachmentMode AttachmentMode = EGorgeousFeedbackAttachmentMode::TargetActor;

    /** Socket on the target the effect attaches to. Only used when AttachmentMode is TargetSocket. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attachment", meta=(EditCondition="AttachmentMode == EGorgeousFeedbackAttachmentMode::TargetSocket"))
    FName AttachSocket;

    /** Transform applied relative to the attach point or world location when spawning the effect. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attachment")
    FTransform RelativeTransform;

protected:

    /**
     * Resolves the most appropriate player controller from the feedback context.
     * Player-centric effects (force feedback, haptics, camera shakes) are routed to the
     * instigator/target if they are a player, otherwise to the first local player controller.
     *
     * @param Context The feedback context describing the current situation.
     * @return The resolved player controller, or nullptr if none could be found.
     */
    APlayerController* ResolvePlayerController(const FGorgeousFeedbackContext& Context) const;

    /**
     * Resolves the world-space transform the effect should spawn at, preferring an attached
     * component's transform and falling back to the context's explicit world location/rotation.
     *
     * @param Context The feedback context describing the current situation.
     * @return The resolved spawn transform.
     */
    static FTransform ResolveEffectTransform(const FGorgeousFeedbackContext& Context);

public:

    /** Executes the effect using the supplied feedback context. Override Execute_Implementation for custom behavior. */
    UFUNCTION(BlueprintNativeEvent, Category="Feedback")
    void Execute(const FGorgeousFeedbackContext& Context);

    /** Returns whether this effect is allowed to execute in the given context (tags, assets, enabled state). */
    UFUNCTION(BlueprintNativeEvent, Category="Feedback")
    bool CanExecute(const FGorgeousFeedbackContext& Context) const;

#if WITH_EDITORONLY_DATA

    /** Color used to tint this effect in the editor feedback graph. */
    UPROPERTY(EditAnywhere, Category="Editor")
    FLinearColor EditorColor = FLinearColor::White;

    /** Category label used to group this effect in the editor. */
    UPROPERTY(EditAnywhere, Category="Editor")
    FText EditorCategory;

#endif

#if WITH_EDITOR //@TODO WYSIWYG editor to preview stuff

    /** Plays this effect inside the editor for preview purposes. */
    UFUNCTION(CallInEditor, Category="Preview")
    void Preview();

    /** Stops the preview if supported by this effect. */
    UFUNCTION(CallInEditor, Category="Preview")
    void StopPreview();

    /** Whether this effect supports previewing. Defaults to true. */
    virtual bool SupportsPreview() const
    {
        return true;
    }

protected:

    /** Editor-only hook for playing the effect preview. Override in subclasses that support it. */
    virtual void Preview_Internal();

    /** Editor-only hook for stopping the effect preview. Override in subclasses that support it. */
    virtual void StopPreview_Internal();

#endif
};

/**
 * Potential future feedback effects that could be added to the system:
 *
 * UGorgeousDiscordNotificationEffect
 * UGorgeousScreenDistortionEffect
 * UGorgeousDialogueEffect
 */
