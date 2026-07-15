#pragma once

#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "GorgeousHapticFeedbackEffect.generated.h"

/**
 * Feedback effect that triggers haptic feedback on a controller or mobile device.
 *
 * Uses Unreal's built-in haptics orchestration; no low-level platform API is accessed.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousHapticFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The haptic curve asset describing the vibration pattern. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UHapticFeedbackEffect_Base> HapticEffect;

	/** Which hand/controller the haptic effect is played on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EControllerHand Hand = EControllerHand::Left;

	/** Multiplier applied to the intensity of the haptic effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.f;

	/** Whether the haptic effect should follow the player's hand. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFollowTarget = false;

public:

	/** Plays the haptic effect on the resolved player controller. */
	virtual void Execute_Implementation(const FGorgeousFeedbackContext& Context) override;

	/** Returns true when the effect is enabled and a haptic asset is set. */
	virtual bool CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const override;
};
