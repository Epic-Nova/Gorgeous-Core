#include "FeedbackEffects/Effects/GorgeousForceFeedbackEffect.h"

#include "Engine/World.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ForceFeedbackComponent.h"

void UGorgeousForceFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
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
		if (UForceFeedbackComponent* Component = UGameplayStatics::SpawnForceFeedbackAtLocation(
			World,
			ForceFeedback,
			EffectTransform.GetLocation(),
			EffectTransform.Rotator(),
			bLooping,
			1.f,
			0.f,
			nullptr,
			true)) { ActiveWorldEffects.Add(Component); }
		return;
	}

	if (APlayerController* PlayerController = ResolvePlayerController(Context))
	{
		ActiveControllers.AddUnique(PlayerController);
		FForceFeedbackParameters Params;
		Params.bLooping = bLooping;
		Params.bIgnoreTimeDilation = bIgnoreTimeDilation;
		Params.bPlayWhilePaused = bPlayWhilePaused;
		PlayerController->ClientPlayForceFeedback(ForceFeedback, Params);
	}
}

bool UGorgeousForceFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && ForceFeedback != nullptr;
}

float UGorgeousForceFeedbackEffect::GetPlaybackDuration(const FGorgeousFeedbackContext& Context) const
{
	if (!ForceFeedback) return 0.f;
	if (bLooping) return -1.f;
	// GetDuration is not exported by the engine's MinimalAPI class. Read the
	// cooked duration, or calculate a conservative duration for an edited asset.
	float Duration = FMath::Max(0.f, ForceFeedback->Duration);
	auto IncludeChannels = [&Duration](const TArray<FForceFeedbackChannelDetails>& Channels)
	{
		for (const auto& Channel : Channels)
		{
			float Start = 0.f, End = 0.f;
			Channel.Curve.GetRichCurveConst()->GetTimeRange(Start, End);
			Duration = FMath::Max(Duration, End);
		}
	};
	IncludeChannels(ForceFeedback->ChannelDetails);
	for (const auto& Override : ForceFeedback->PerDeviceOverrides) IncludeChannels(Override.Value.ChannelDetails);
	return Duration;
}

void UGorgeousForceFeedbackEffect::StopActiveEffects()
{
	for (const auto& WeakController : ActiveControllers) { if (APlayerController* Controller = WeakController.Get()) { Controller->ClientStopForceFeedback(ForceFeedback, NAME_None); } }
	ActiveControllers.Reset();
	for (const auto& Component : ActiveWorldEffects) { if (Component.IsValid()) { Component->Stop(); Component->DestroyComponent(); } }
	ActiveWorldEffects.Reset();
}
