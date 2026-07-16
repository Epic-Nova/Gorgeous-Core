#pragma once

#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "GorgeousCameraShakeFeedbackEffect.generated.h"

/**
 * Feedback effect that plays a camera shake on the player's camera.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousCameraShakeFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The camera shake class to start on the player's camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> CameraShake;

	/** Intensity multiplier applied to the camera shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.f;

	/** Coordinate space in which the camera shake is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal;

	/** Custom rotation used when PlaySpace is set to UserDefined. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator UserPlaySpaceRotation;

public:

	/** Starts the camera shake on the resolved player controller's camera. */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/** Returns true when the effect is enabled and a camera shake class is set. */
	virtual bool CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const override;
};
