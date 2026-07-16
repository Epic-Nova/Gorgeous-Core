#pragma once

#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "NiagaraSystem.h"
#include "GorgeousNiagaraFeedbackEffect.generated.h"

/**
 * Feedback effect that spawns a Niagara VFX system at the feedback location or attached to the target.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousNiagaraFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The Niagara system template to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> System;

	/** World-space offset added to the spawn location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset = FVector::ZeroVector;

	/** Rotation offset added to the spawn rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** Scale applied to the spawned Niagara system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector(1.f);

	/** Whether the system follows the target when an attach component is available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFollowTarget = true;

	/** Whether the spawned system is automatically destroyed when it completes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDestroy = true;

public:

	/** Spawns the Niagara system at the feedback location or attached to the target. */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/** Returns true when the effect is enabled and a Niagara system is set. */
	virtual bool CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const override;
};
