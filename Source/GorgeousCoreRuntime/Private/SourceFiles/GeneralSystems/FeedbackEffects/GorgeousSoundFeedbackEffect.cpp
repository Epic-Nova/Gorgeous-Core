#include "FeedbackEffects/Effects/GorgeousSoundFeedbackEffect.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

UAudioComponent* UGorgeousSoundFeedbackEffect::SpawnAudio(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !Sound) { return nullptr; }
	UWorld* World = GetWorld();
	if (!World && Context.OwnerActor) { World = Context.OwnerActor->GetWorld(); }
	if (!World) { return nullptr; }
	if (USceneComponent* Attach = ResolveAttachComponent(Context); bFollowTarget && Attach)
	{
		return UGameplayStatics::SpawnSoundAttached(Sound, Attach, AttachSocket, RelativeTransform.GetLocation(),
			RelativeTransform.Rotator(), EAttachLocation::KeepRelativeOffset, true, VolumeMultiplier, PitchMultiplier, StartTime);
	}
	const FTransform Transform = ResolveEffectTransform(Context);
	return UGameplayStatics::SpawnSoundAtLocation(World, Sound, Transform.GetLocation(), Transform.Rotator(),
		VolumeMultiplier, PitchMultiplier, StartTime);
}

void UGorgeousSoundFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	PerformExecuteWithCompletion(Context, FSimpleDelegate());
}

void UGorgeousSoundFeedbackEffect::PerformExecuteWithCompletion(const FGorgeousFeedbackContext& Context, FSimpleDelegate OnCompleted)
{
	UAudioComponent* Audio = SpawnAudio(Context);
	if (!Audio) { OnCompleted.ExecuteIfBound(); return; }
	ActiveAudio.RemoveAll([](const auto& Existing) { return !Existing.IsValid(); });
	ActiveAudio.Add(Audio);
	const TWeakObjectPtr<UAudioComponent> WeakAudio(Audio);
	WaitForPlayback(Audio->GetWorld(), [WeakAudio] { return !WeakAudio.IsValid() || !WeakAudio->IsPlaying(); }, OnCompleted);
}

void UGorgeousSoundFeedbackEffect::StopActiveEffects()
{
	for (const auto& Audio : ActiveAudio) { if (Audio.IsValid()) { Audio->Stop(); } }
	ActiveAudio.Reset();
}

bool UGorgeousSoundFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && Sound != nullptr;
}

#if WITH_EDITOR
void UGorgeousSoundFeedbackEffect::Preview_Internal()
{
	Super::Preview_Internal();
}
#endif
