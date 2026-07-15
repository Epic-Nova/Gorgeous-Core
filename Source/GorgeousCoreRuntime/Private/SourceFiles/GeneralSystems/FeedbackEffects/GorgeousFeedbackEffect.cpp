#include "FeedbackEffects/GorgeousFeedbackEffect.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "FeedbackEffects/GorgeousFeedbackProvider.h"

void UGorgeousFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
}

bool UGorgeousFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	return false;
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