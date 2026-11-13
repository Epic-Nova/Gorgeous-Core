#include "FeedbackEffects/Effects/GorgeousHapticFeedbackEffect.h"

#include "Engine/World.h"
#include "Haptics/HapticFeedbackEffect_Base.h"

void UGorgeousHapticFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !HapticEffect)
	{
		return;
	}

	if (APlayerController* PlayerController = ResolvePlayerController(Context))
	{
		ActiveControllers.AddUnique(PlayerController);
		PlayerController->PlayHapticEffect(HapticEffect, Hand, Scale, bLooping);
	}
}

bool UGorgeousHapticFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && HapticEffect != nullptr;
}

float UGorgeousHapticFeedbackEffect::GetPlaybackDuration(const FGorgeousFeedbackContext& Context) const
{
	return HapticEffect ? (bLooping ? -1.f : HapticEffect->GetDuration()) : 0.f;
}

void UGorgeousHapticFeedbackEffect::StopActiveEffects()
{
	for (const auto& WeakController : ActiveControllers) { if (APlayerController* Controller = WeakController.Get()) { Controller->StopHapticEffect(Hand); } }
	ActiveControllers.Reset();
}
