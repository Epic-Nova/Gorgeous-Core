#include "FeedbackEffects/GorgeousFeedbackEffect.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "FeedbackEffects/GorgeousFeedbackProvider.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

void UGorgeousFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
	if (bPerformingScheduledExecution)
	{
		bNativeCompletionStarted = true;
		PerformExecuteWithCompletion(Context, FSimpleDelegate::CreateWeakLambda(this, [this, Id = CurrentExecutionId] { CompleteFeedbackExecution(Id); }));
	}
	else { PlayScheduled(Context); }
}

void UGorgeousFeedbackEffect::PlayFeedback(const FGorgeousFeedbackContext& Context)
{
	PlayScheduled(Context);
}

void UGorgeousFeedbackEffect::PlayScheduled(const FGorgeousFeedbackContext& Context, FSimpleDelegate OnCompleted)
{
	if (!CanExecute(Context) || Chance <= 0.f || (Chance < 1.f && FMath::FRand() >= Chance))
	{
		OnCompleted.ExecuteIfBound();
		return;
	}
	const float InitialDelay = FMath::Max(0.f, Delay) + (RandomDelay > 0.f ? FMath::FRandRange(0.f, RandomDelay) : 0.f);
	const int32 Plays = FMath::Max(1, RepeatCount);
	UWorld* World = GetWorld();
	if (!World && IsValid(Context.OwnerActor)) { World = Context.OwnerActor->GetWorld(); }
	if (!World && IsValid(Context.AttachComponent)) { World = Context.AttachComponent->GetWorld(); }
	const TWeakObjectPtr<UGorgeousFeedbackEffect> WeakThis(this);
	const TWeakObjectPtr<UObject> Instigator(Context.Instigator), Target(Context.Target), UserData(Context.UserData);
	const TWeakObjectPtr<AActor> Owner(Context.OwnerActor);
	const TWeakObjectPtr<USceneComponent> Attach(Context.AttachComponent);
	const bool bHadOwner = IsValid(Context.OwnerActor);
	const TSharedRef<int32> Remaining = MakeShared<int32>(Plays);
	for (int32 Index = 0; Index < Plays; ++Index)
	{
		const float PlayDelay = InitialDelay + Index * FMath::Max(0.f, RepeatInterval);
		const FGuid Id = FGuid::NewGuid();
		auto Play = [WeakThis, Context, Instigator, Target, UserData, Owner, Attach, bHadOwner, Remaining, OnCompleted, Id]() mutable
		{
			if (!WeakThis.IsValid()) { return; }
			UGorgeousFeedbackEffect* Self = WeakThis.Get();
			Self->PendingTimers.Remove(Id);
			if (bHadOwner && !Owner.IsValid()) { return; }
			FGorgeousFeedbackContext SafeContext = Context;
			SafeContext.Instigator = Instigator.Get(); SafeContext.Target = Target.Get(); SafeContext.UserData = UserData.Get();
			SafeContext.OwnerActor = Owner.Get(); SafeContext.AttachComponent = Attach.Get();
			const FSimpleDelegate Finished = FSimpleDelegate::CreateLambda([Remaining, OnCompleted]
			{ if (--*Remaining == 0) { OnCompleted.ExecuteIfBound(); } });
			Self->PendingCompletions.Add(Id, Finished);
			if (Self->GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UGorgeousFeedbackEffect, OnExecuteAsync)))
			{
				Self->OnExecuteAsync(SafeContext, Id);
			}
			else
			{
				TGuardValue<bool> Executing(Self->bPerformingScheduledExecution, true);
				TGuardValue<bool> NativeCompletion(Self->bNativeCompletionStarted, false);
				TGuardValue<FGuid> Execution(Self->CurrentExecutionId, Id);
				Self->Execute(SafeContext);
				// The legacy Blueprint Execute hook is synchronous unless it calls its native parent.
				if (!Self->bNativeCompletionStarted) { Self->CompleteFeedbackExecution(Id); }
			}
		};
		if (PlayDelay <= 0.f) { Play(); }
		else if (World)
		{
			FTimerHandle Handle;
			World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda(MoveTemp(Play)), PlayDelay, false);
			PendingTimers.Add(Id, TPair<TWeakObjectPtr<UWorld>, FTimerHandle>(World, Handle));
		}
		else { if (--*Remaining == 0) { OnCompleted.ExecuteIfBound(); } }
	}
}

void UGorgeousFeedbackEffect::CancelPendingExecutions()
{
	for (auto& Entry : PendingTimers)
	{
		if (UWorld* World = Entry.Value.Key.Get()) { World->GetTimerManager().ClearTimer(Entry.Value.Value); }
	}
	PendingTimers.Reset();
	PendingCompletions.Reset();
}

void UGorgeousFeedbackEffect::StopFeedback()
{
	CancelPendingExecutions();
	StopActiveEffects();
}

void UGorgeousFeedbackEffect::BeginDestroy()
{
	CancelPendingExecutions();
	StopActiveEffects();
	Super::BeginDestroy();
}

void UGorgeousFeedbackEffect::CompleteFeedbackExecution(FGuid ExecutionId)
{
	FSimpleDelegate Finished;
	if (PendingCompletions.RemoveAndCopyValue(ExecutionId, Finished)) { Finished.ExecuteIfBound(); }
}

void UGorgeousFeedbackEffect::PerformExecuteWithCompletion(const FGorgeousFeedbackContext& Context, FSimpleDelegate OnCompleted)
{
	PerformExecute(Context);
	const float Duration = GetPlaybackDuration(Context);
	if (Duration < 0.f) { return; } // Authored looping effects require explicit cancellation/completion.
	UWorld* World = GetWorld();
	if (!World && Context.OwnerActor) { World = Context.OwnerActor->GetWorld(); }
	if (Duration <= 0.f || !World) { OnCompleted.ExecuteIfBound(); return; }
	const FGuid TimerId = FGuid::NewGuid();
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, TimerId, OnCompleted]
	{ PendingTimers.Remove(TimerId); OnCompleted.ExecuteIfBound(); }), Duration, false);
	PendingTimers.Add(TimerId, TPair<TWeakObjectPtr<UWorld>, FTimerHandle>(World, Handle));
}

void UGorgeousFeedbackEffect::WaitForPlayback(UWorld* World, TFunction<bool()> IsFinished, FSimpleDelegate OnCompleted)
{
	if (!World || IsFinished()) { OnCompleted.ExecuteIfBound(); return; }
	const FGuid Id = FGuid::NewGuid();
	const TWeakObjectPtr<UGorgeousFeedbackEffect> WeakThis(this);
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakThis, Id, IsFinished = MoveTemp(IsFinished), OnCompleted]() mutable
	{
		if (!WeakThis.IsValid() || !IsFinished()) { return; }
		auto* Entry = WeakThis->PendingTimers.Find(Id);
		if (!Entry) { return; }
		if (UWorld* TimerWorld = Entry->Key.Get()) { TimerWorld->GetTimerManager().ClearTimer(Entry->Value); }
		WeakThis->PendingTimers.Remove(Id);
		OnCompleted.ExecuteIfBound();
	}), .02f, true);
	PendingTimers.Add(Id, TPair<TWeakObjectPtr<UWorld>, FTimerHandle>(World, Handle));
}

bool UGorgeousFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	// Universal gating shared by every effect:
	// disabled effects, missing required tags and present blocked tags never run.
	const FString EffectName = DisplayName.IsEmpty() ? GetName() : DisplayName.ToString();

	if (!bEnabled)
	{
		GT_W_LOG("GT.FeedbackEffects.CanExecute", TEXT("Effect '%s' skipped: disabled."), *EffectName);
		return false;
	}

	if (!Context.ContextTags.HasAll(RequiredTags))
	{
		GT_W_LOG("GT.FeedbackEffects.CanExecute",
			TEXT("Effect '%s' skipped: missing required tags. Has [%s], needs [%s]."),
			*EffectName, *Context.ContextTags.ToString(), *RequiredTags.ToString());
		return false;
	}

	if (Context.ContextTags.HasAny(BlockedTags))
	{
		GT_W_LOG("GT.FeedbackEffects.CanExecute",
			TEXT("Effect '%s' skipped: blocked by tag. Context tags [%s], blocked by [%s]."),
			*EffectName, *Context.ContextTags.ToString(), *BlockedTags.ToString());
		return false;
	}

	GT_I_LOG("GT.FeedbackEffects.CanExecute", TEXT("Effect '%s' passed gating."), *EffectName);
	return true;
}

APlayerController* UGorgeousFeedbackEffect::ResolvePlayerController(const FGorgeousFeedbackContext& Context) const
{
	// Prefer an explicit player controller carried by the context.
	if (APlayerController* PlayerController = Cast<APlayerController>(Context.Instigator))
	{
		return PlayerController;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Context.Target))
	{
		return PlayerController;
	}

	// Fall back to the controller of a pawn carried by the context.
	if (const APawn* Pawn = Cast<APawn>(Context.Instigator))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			return PlayerController;
		}
	}

	if (const APawn* Pawn = Cast<APawn>(Context.Target))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			return PlayerController;
		}
	}

	// Last resort: resolve the first local player controller from the world.
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController(); PlayerController && PlayerController->IsLocalController())
		{
			return PlayerController;
		}

		if (ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(World))
			{
				return PlayerController;
			}
		}
	}

	return nullptr;
}

USceneComponent* UGorgeousFeedbackEffect::ResolveAttachComponent(const FGorgeousFeedbackContext& Context) const
{
	if (AttachmentMode == EGorgeousFeedbackAttachmentMode::None || AttachmentMode == EGorgeousFeedbackAttachmentMode::WorldLocation) { return nullptr; }
	if (IsValid(Context.AttachComponent)) { return Context.AttachComponent; }
	if (const AActor* Target = Cast<AActor>(Context.Target)) { return Target->GetRootComponent(); }
	return IsValid(Context.OwnerActor) ? Context.OwnerActor->GetRootComponent() : nullptr;
}

FTransform UGorgeousFeedbackEffect::ResolveEffectTransform(const FGorgeousFeedbackContext& Context) const
{
	FTransform Transform;
	Transform.SetLocation(Context.WorldLocation);
	Transform.SetRotation(FQuat(Context.WorldRotation));

	// When an attach component is provided it defines the spawn location and rotation.
	if ((AttachmentMode == EGorgeousFeedbackAttachmentMode::Component || AttachmentMode == EGorgeousFeedbackAttachmentMode::TargetSocket) && Context.AttachComponent)
	{
		Transform = AttachmentMode == EGorgeousFeedbackAttachmentMode::TargetSocket
			? Context.AttachComponent->GetSocketTransform(AttachSocket) : Context.AttachComponent->GetComponentTransform();
	}
	// OwnerActor acts as an explicit, editor-authored fallback reference. If both the explicit
	// world location and an attach component are missing we still want a stable anchor, so we
	// derive it from the owner when present (GetWorld already covers the general case).
	else if (AttachmentMode == EGorgeousFeedbackAttachmentMode::TargetActor || AttachmentMode == EGorgeousFeedbackAttachmentMode::TargetSocket)
	{
		if (const AActor* TargetActor = Cast<AActor>(Context.Target)) { Transform = TargetActor->GetActorTransform(); }
		else if (Context.OwnerActor && Context.WorldLocation.IsZero() && Context.WorldRotation.IsZero()) { Transform = Context.OwnerActor->GetActorTransform(); }
	}
	Transform = RelativeTransform * Transform;

	return Transform;
}

#if WITH_EDITOR
void UGorgeousFeedbackEffect::Preview()
{
	Preview_Internal();
}

void UGorgeousFeedbackEffect::StopPreview()
{
	StopPreview_Internal();
}

void UGorgeousFeedbackEffect::Preview_Internal()
{
	if (!GetWorld() && GEditor) { SetFallbackOwner(GEditor->GetEditorWorldContext().World()); }
	PlayFeedback(FGorgeousFeedbackContext{});
}

void UGorgeousFeedbackEffect::StopPreview_Internal()
{
	StopFeedback();
}
#endif


// --- Feedback Provider

bool UGorgeousFeedbackProvider::ProvideFeedback_Implementation(const FGorgeousFeedbackContext& Context,
	FGorgeousFeedbackDefinition& OutFeedback) const
{
	return false;
}
