#include "FeedbackEffects/Effects/GorgeousForceFeedbackEffect.h"

#include "Engine/World.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "Kismet/GameplayStatics.h"

void UGorgeousForceFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !ForceFeedback)
	{
		return;
	}

	UWorld* World = GetWorld();
	const FTransform EffectTransform = ResolveEffectTransform(Context);

	// World-anchored force feedback is spawned at a location; everything else is routed to the player controller.
	if (AttachmentMode == EGorgeousFeedbackAttachmentMode::WorldLocation && World)
	{
		UGameplayStatics::SpawnForceFeedbackAtLocation(
			World,
			ForceFeedback,
			EffectTransform.GetLocation(),
			EffectTransform.Rotator(),
			bLooping,
			1.f,
			0.f,
			nullptr,
			true);
		return;
	}

	if (APlayerController* PlayerController = ResolvePlayerController(Context))
	{
		FForceFeedbackParameters Params;
		Params.bLooping = bLooping;
		Params.bIgnoreTimeDilation = bIgnoreTimeDilation;
		Params.bPlayWhilePaused = bPlayWhilePaused;
		PlayerController->ClientPlayForceFeedback(ForceFeedback, Params);
	}
}

bool UGorgeousForceFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return bEnabled && ForceFeedback != nullptr;
}
