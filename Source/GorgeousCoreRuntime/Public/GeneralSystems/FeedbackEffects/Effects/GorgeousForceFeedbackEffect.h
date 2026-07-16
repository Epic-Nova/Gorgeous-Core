#pragma once

#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "GorgeousForceFeedbackEffect.generated.h"

/**
 * Feedback effect that triggers controller force feedback using Unreal's built-in force feedback.
 *
 * The effect relies entirely on the engine's force feedback orchestration (player controller /
 * world-spawned force feedback component) and does not touch any low-level platform API.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousForceFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The force feedback curve asset that drives the controller motors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UForceFeedbackEffect> ForceFeedback;

	/** Whether the force feedback effect should loop until stopped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = false;

	/** Whether time dilation should be ignored while the effect plays. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreTimeDilation = false;

	/** Whether the effect should continue playing while the game is paused. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayWhilePaused = false;

public:

	/** Plays the force feedback effect on the player controller or at the world location. */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/** Returns true when the effect is enabled and a force feedback asset is set. */
	virtual bool CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const override;
};
