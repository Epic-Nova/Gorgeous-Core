#include "FeedbackEffects/Effects/GorgeousHapticFeedbackEffect.h"

#include "Engine/World.h"

void UGorgeousHapticFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !HapticEffect)
	{
		return;
	}

	if (APlayerController* PlayerController = ResolvePlayerController(Context))
	{
		PlayerController->PlayHapticEffect(HapticEffect, Hand, Scale, bFollowTarget);
	}
}

bool UGorgeousHapticFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && HapticEffect != nullptr;
}
