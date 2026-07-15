#include "FeedbackEffects/Effects/GorgeousSoundFeedbackEffect.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UGorgeousSoundFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !Sound)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// When the effect should follow a target and a component is available, attach the sound to it.
	if (bFollowTarget && Context.AttachComponent)
	{
		UGameplayStatics::SpawnSoundAttached(
			Sound,
			Context.AttachComponent,
			AttachSocket,
			RelativeTransform.GetLocation(),
			RelativeTransform.GetRotation().Rotator(),
			EAttachLocation::SnapToTarget,
			true,
			VolumeMultiplier,
			PitchMultiplier,
			StartTime);
		return;
	}

	const FTransform EffectTransform = ResolveEffectTransform(Context);
	UGameplayStatics::PlaySoundAtLocation(
		World,
		Sound,
		EffectTransform.GetLocation(),
		EffectTransform.GetRotation().Rotator(),
		VolumeMultiplier,
		PitchMultiplier,
		StartTime);
}

bool UGorgeousSoundFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return bEnabled && Sound != nullptr;
}

#if WITH_EDITOR
void UGorgeousSoundFeedbackEffect::Preview_Internal()
{
	// Previewing a one-shot sound through the editor is intentionally unsupported; the effect is
	// fully described by its editable properties and plays through normal execution.
}
#endif
