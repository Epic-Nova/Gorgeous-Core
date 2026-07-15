#include "FeedbackEffects/Effects/GorgeousCameraShakeFeedbackEffect.h"

#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"

void UGorgeousCameraShakeFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !CameraShake)
	{
		return;
	}

	if (APlayerController* PlayerController = ResolvePlayerController(Context))
	{
		PlayerController->ClientStartCameraShake(CameraShake, Scale, PlaySpace, UserPlaySpaceRotation);
	}
}

bool UGorgeousCameraShakeFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return bEnabled && CameraShake != nullptr;
}
