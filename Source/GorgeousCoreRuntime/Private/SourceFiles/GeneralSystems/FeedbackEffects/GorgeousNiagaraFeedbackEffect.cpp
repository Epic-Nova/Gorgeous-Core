#include "FeedbackEffects/Effects/GorgeousNiagaraFeedbackEffect.h"

#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

UNiagaraComponent* UGorgeousNiagaraFeedbackEffect::SpawnFeedbackSystem(const FGorgeousFeedbackContext& Context)
{
	if (!bEnabled || !System) { return nullptr; }
	UWorld* World = GetWorld();
	if (!World && Context.OwnerActor) { World = Context.OwnerActor->GetWorld(); }
	if (!World) { return nullptr; }
	if (USceneComponent* Attach = ResolveAttachComponent(Context); bFollowTarget && Attach)
	{
		return UNiagaraFunctionLibrary::SpawnSystemAttached(System, Attach, AttachSocket, Offset + RelativeTransform.GetLocation(), RotationOffset + RelativeTransform.Rotator(), Scale * RelativeTransform.GetScale3D(), EAttachLocation::KeepRelativeOffset, bAutoDestroy, ENCPoolMethod::None);
	}
	const FTransform Transform = ResolveEffectTransform(Context);
	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, System, Transform.GetLocation() + Offset, Transform.Rotator() + RotationOffset, Scale * Transform.GetScale3D(), bAutoDestroy);
}

void UGorgeousNiagaraFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	PerformExecuteWithCompletion(Context, FSimpleDelegate());
}

void UGorgeousNiagaraFeedbackEffect::PerformExecuteWithCompletion(const FGorgeousFeedbackContext& Context, FSimpleDelegate OnCompleted)
{
	UNiagaraComponent* Component = SpawnFeedbackSystem(Context);
	if (!Component) { OnCompleted.ExecuteIfBound(); return; }
	ActiveSystems.RemoveAll([](const auto& Existing) { return !Existing.IsValid(); });
	ActiveSystems.Add(Component);
	const TWeakObjectPtr<UNiagaraComponent> WeakComponent(Component);
	WaitForPlayback(Component->GetWorld(), [WeakComponent] { return !WeakComponent.IsValid() || WeakComponent->IsComplete(); }, OnCompleted);
}

void UGorgeousNiagaraFeedbackEffect::StopActiveEffects()
{
	for (const auto& Component : ActiveSystems) { if (Component.IsValid()) { Component->DeactivateImmediate(); Component->DestroyComponent(); } }
	ActiveSystems.Reset();
}

bool UGorgeousNiagaraFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return Super::CanExecute_Implementation(Context) && System != nullptr;
}
