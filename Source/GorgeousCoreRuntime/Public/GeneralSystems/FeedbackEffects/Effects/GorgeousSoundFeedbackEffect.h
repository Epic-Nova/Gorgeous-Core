#pragma once

#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "GorgeousSoundFeedbackEffect.generated.h"

/**
 * Feedback effect that plays a sound cue at the feedback location or attached to the target.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousSoundFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The sound asset to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<USoundBase> Sound;

	/** Multiplier applied to the sound's volume. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	float VolumeMultiplier = 1.f;

	/** Multiplier applied to the sound's pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	float PitchMultiplier = 1.f;

	/** Time in seconds into the sound at which playback begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	float StartTime = 0.f;

	/** Whether the sound should follow the target when an attach component is available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	bool bFollowTarget = true;

public:

	/** Plays the sound at the feedback location or attached to the target. */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/** Returns true when the effect is enabled and a sound asset is set. */
	virtual bool CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const override;

protected:

#if WITH_EDITOR
	/** Editor preview hook; intentionally a no-op as the effect is described by its properties. */
	virtual void Preview_Internal() override;
#endif
};
