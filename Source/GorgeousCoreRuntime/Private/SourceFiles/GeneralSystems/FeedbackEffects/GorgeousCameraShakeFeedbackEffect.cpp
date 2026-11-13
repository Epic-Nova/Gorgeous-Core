#include "FeedbackEffects/Effects/GorgeousCameraShakeFeedbackEffect.h"

#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"

void UGorgeousCameraShakeFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !CameraShake)
	{
		return;
	}

	if (APlayerController* PlayerController = ResolvePlayerController(Context))
	{
		ActiveControllers.AddUnique(PlayerController);
		PlayerController->ClientStartCameraShake(CameraShake, Scale, PlaySpace, UserPlaySpaceRotation);
	}
}

bool UGorgeousCameraShakeFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && CameraShake != nullptr;
}

float UGorgeousCameraShakeFeedbackEffect::GetPlaybackDuration(const FGorgeousFeedbackContext& Context) const
{
	FCameraShakeDuration Duration;
	if (!UCameraShakeBase::GetCameraShakeDuration(CameraShake, Duration)) { return 0.f; }
	return Duration.IsInfinite() ? -1.f : Duration.Get();
}

void UGorgeousCameraShakeFeedbackEffect::StopActiveEffects()
{
	for (const auto& WeakController : ActiveControllers) { if (APlayerController* Controller = WeakController.Get()) { Controller->ClientStopCameraShake(CameraShake, true); } }
	ActiveControllers.Reset();
}
