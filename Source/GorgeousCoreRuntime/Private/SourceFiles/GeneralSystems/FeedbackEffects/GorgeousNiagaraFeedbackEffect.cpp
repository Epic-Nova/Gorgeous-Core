#include "FeedbackEffects/Effects/GorgeousNiagaraFeedbackEffect.h"

#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"

void UGorgeousNiagaraFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !System)
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

	// When the effect should follow a target and a component is available, attach the system to it.
	if (bFollowTarget && Context.AttachComponent)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			System,
			Context.AttachComponent,
			AttachSocket,
			Offset,
			RotationOffset,
			EAttachLocation::SnapToTarget,
			bAutoDestroy);
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, System, SpawnLocation, SpawnRotation, Scale, bAutoDestroy);
}

bool UGorgeousNiagaraFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && System != nullptr;
}
