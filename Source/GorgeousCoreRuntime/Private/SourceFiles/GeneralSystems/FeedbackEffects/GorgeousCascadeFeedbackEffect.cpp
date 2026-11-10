#include "FeedbackEffects/Effects/GorgeousCascadeFeedbackEffect.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

UParticleSystemComponent* UGorgeousCascadeFeedbackEffect::SpawnFeedbackEmitter(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !ParticleSystem) { return nullptr; }
	UWorld* World = GetWorld();
	if (!World && Context.OwnerActor) { World = Context.OwnerActor->GetWorld(); }
	if (!World) { return nullptr; }
	if (USceneComponent* Attach = ResolveAttachComponent(Context); bFollowTarget && Attach)
	{
		return UGameplayStatics::SpawnEmitterAttached(ParticleSystem, Attach, AttachSocket, Offset + RelativeTransform.GetLocation(), RotationOffset + RelativeTransform.Rotator(), Scale * RelativeTransform.GetScale3D(), EAttachLocation::KeepRelativeOffset, bAutoDestroy);
	}
	const FTransform Transform = ResolveEffectTransform(Context);
	return UGameplayStatics::SpawnEmitterAtLocation(World, ParticleSystem, Transform.GetLocation() + Offset, Transform.Rotator() + RotationOffset, Scale * Transform.GetScale3D(), bAutoDestroy);
}

void UGorgeousCascadeFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	PerformExecuteWithCompletion(Context, FSimpleDelegate());
}

void UGorgeousCascadeFeedbackEffect::PerformExecuteWithCompletion(const FGorgeousFeedbackContext& Context, FSimpleDelegate OnCompleted)
{
	UParticleSystemComponent* Component = SpawnFeedbackEmitter(Context);
	if (!Component) { OnCompleted.ExecuteIfBound(); return; }
	ActiveEmitters.RemoveAll([](const auto& Existing) { return !Existing.IsValid(); });
	ActiveEmitters.Add(Component);
	const TWeakObjectPtr<UParticleSystemComponent> WeakComponent(Component);
	WaitForPlayback(Component->GetWorld(), [WeakComponent] { return !WeakComponent.IsValid() || WeakComponent->HasCompleted(); }, OnCompleted);
}

void UGorgeousCascadeFeedbackEffect::StopActiveEffects()
{
	for (const auto& Component : ActiveEmitters) { if (Component.IsValid()) { Component->DeactivateSystem(); Component->DestroyComponent(); } }
	ActiveEmitters.Reset();
}

bool UGorgeousCascadeFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && ParticleSystem != nullptr;
}
