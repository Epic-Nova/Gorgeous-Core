#pragma once

#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "GorgeousCascadeFeedbackEffect.generated.h"

/**
 * Feedback effect that spawns a Cascade (legacy particle) emitter at the feedback location or attached to the target.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousCascadeFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The particle system template to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UParticleSystem> ParticleSystem;

	/** World-space offset added to the spawn location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset = FVector::ZeroVector;

	/** Rotation offset added to the spawn rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** Scale applied to the spawned emitter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector(1.f);

	/** Whether the emitter follows the target when an attach component is available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFollowTarget = true;

	/** Whether the spawned emitter is automatically destroyed when it completes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDestroy = true;
	
public:

	/** Spawns the Cascade emitter at the feedback location or attached to the target. */
	virtual void Execute_Implementation(const FGorgeousFeedbackContext& Context) override;

	/** Returns true when the effect is enabled and a particle system is set. */
	virtual bool CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const override;
};
