#include "FeedbackEffects/Effects/GorgeousCascadeFeedbackEffect.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

void UGorgeousCascadeFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !ParticleSystem)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform EffectTransform = ResolveEffectTransform(Context);
	const FVector SpawnLocation = EffectTransform.GetLocation() + Offset;
	const FRotator SpawnRotation = EffectTransform.GetRotation().Rotator() + RotationOffset;

	// When the effect should follow a target and a component is available, attach the emitter to it.
	if (bFollowTarget && Context.AttachComponent)
	{
		UGameplayStatics::SpawnEmitterAttached(
			ParticleSystem,
			Context.AttachComponent,
			AttachSocket,
			Offset,
			RotationOffset,
			Scale,
			EAttachLocation::SnapToTarget,
			bAutoDestroy);
		return;
	}

	UGameplayStatics::SpawnEmitterAtLocation(World, ParticleSystem, SpawnLocation, SpawnRotation, Scale, bAutoDestroy);
}

bool UGorgeousCascadeFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return bEnabled && ParticleSystem != nullptr;
}
