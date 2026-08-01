#include "FeedbackEffects/GorgeousFeedbackEffect.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "FeedbackEffects/GorgeousFeedbackProvider.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

void UGorgeousFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
	ExecuteWithScheduling(Context);
}

void UGorgeousFeedbackEffect::ExecuteWithScheduling(const FGorgeousFeedbackContext& Context)
{
	// Probability gate: roll against Chance and bail out if it does not pass.
	if (Chance < 1.f && FMath::FRand() > Chance)
	{
		return;
	}

	// Resolve the total initial delay (fixed delay plus an optional random component).
	const float InitialDelay = Delay + (RandomDelay > 0.f ? FMath::FRandRange(0.f, RandomDelay) : 0.f);

	const int32 Plays = FMath::Max(1, RepeatCount);
	const bool bNeedsTimer = InitialDelay > 0.f || (Plays > 1 && RepeatInterval > 0.f);

	if (!bNeedsTimer)
	{
		// Simple immediate case: run all plays back-to-back without a timer.
		for (int32 PlayIndex = 0; PlayIndex < Plays; ++PlayIndex)
		{
			PerformExecute(Context);
		}
		return;
	}

	// Deferred path: schedule the first play, then chain repeats on an interval timer.
	FTimerDelegate FirstPlay;
	FirstPlay.BindLambda([this, Context, Plays]()
	{
		PerformExecute(Context);

		if (Plays > 1 && RepeatInterval > 0.f)
		{
			FTimerDelegate RepeatPlay;
			RepeatPlay.BindLambda([this, Context]()
			{
				PerformExecute(Context);
			});

			if (UWorld* World = GetWorld())
			{
				FTimerHandle RepeatHandle;
				World->GetTimerManager().SetTimer(RepeatHandle, RepeatPlay, RepeatInterval, true);
			}
		}
	});

	if (UWorld* World = GetWorld())
	{
		FTimerHandle FirstPlayHandle;
		if (InitialDelay > 0.f)
		{
			World->GetTimerManager().SetTimer(FirstPlayHandle, FirstPlay, InitialDelay, false);
		}
		else
		{
			World->GetTimerManager().SetTimerForNextTick(FirstPlay);
		}
	}
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
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
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

FTransform UGorgeousFeedbackEffect::ResolveEffectTransform(const FGorgeousFeedbackContext& Context)
{
	FTransform Transform;
	Transform.SetLocation(Context.WorldLocation);
	Transform.SetRotation(FQuat(Context.WorldRotation));

	// When an attach component is provided it defines the spawn location and rotation.
	if (Context.AttachComponent)
	{
		Transform.SetLocation(Context.AttachComponent->GetComponentLocation());
		Transform.SetRotation(FQuat(Context.AttachComponent->GetComponentRotation()));
	}
	// OwnerActor acts as an explicit, editor-authored fallback reference. If both the explicit
	// world location and an attach component are missing we still want a stable anchor, so we
	// derive it from the owner when present (GetWorld already covers the general case).
	else if (Context.OwnerActor)
	{
		Transform.SetLocation(Context.OwnerActor->GetActorLocation());
		Transform.SetRotation(FQuat(Context.OwnerActor->GetActorRotation()));
	}

	return Transform;
}

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
}

void UGorgeousFeedbackEffect::StopPreview_Internal()
{
}


// --- Feedback Provider

bool UGorgeousFeedbackProvider::ProvideFeedback_Implementation(const FGorgeousFeedbackContext& Context,
	FGorgeousFeedbackDefinition& OutFeedback) const
{
	return false;
}
