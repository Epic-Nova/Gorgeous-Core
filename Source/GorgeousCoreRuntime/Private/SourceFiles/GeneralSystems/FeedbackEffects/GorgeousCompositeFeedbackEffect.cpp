#include "FeedbackEffects/GorgeousCompositeFeedbackEffect.h"

void UGorgeousCompositeFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	// Execute every child effect whose own execution conditions are satisfied. Children are
	// responsible for honoring their enabled state, tags and scheduling through CanExecute.
	for (UGorgeousFeedbackEffect* Child : Children)
	{
		if (Child && Child->CanExecute(Context))
		{
			Child->PlayFeedback(Context);
		}
	}
}

void UGorgeousCompositeFeedbackEffect::PerformExecuteWithCompletion(const FGorgeousFeedbackContext& Context, FSimpleDelegate OnCompleted)
{
	TArray<UGorgeousFeedbackEffect*> Playable;
	for (UGorgeousFeedbackEffect* Child : Children) { if (Child && Child->CanExecute(Context)) { Playable.Add(Child); } }
	if (Playable.IsEmpty()) { OnCompleted.ExecuteIfBound(); return; }
	const TSharedRef<int32> Remaining = MakeShared<int32>(Playable.Num());
	for (auto* Child : Playable)
	{
		Child->PlayScheduled(Context, FSimpleDelegate::CreateLambda([Remaining, OnCompleted] { if (--*Remaining == 0) { OnCompleted.ExecuteIfBound(); } }));
	}
}

bool UGorgeousCompositeFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	if (!Super::CanExecute_Implementation(Context))
	{
		return false;
	}

	if (Children.Num() == 0)
	{
		return false;
	}

	// The composite can run if at least one of its children is currently able to execute.
	for (const UGorgeousFeedbackEffect* Child : Children)
	{
		if (Child && Child->CanExecute(Context))
		{
			return true;
		}
	}

	return false;
}

#if WITH_EDITOR
void UGorgeousCompositeFeedbackEffect::Preview_Internal()
{
	Super::Preview_Internal();
}

void UGorgeousCompositeFeedbackEffect::StopPreview_Internal()
{
	Super::StopPreview_Internal();
}

#endif

void UGorgeousCompositeFeedbackEffect::StopActiveEffects()
{
	for (UGorgeousFeedbackEffect* Child : Children) { if (Child) { Child->StopFeedback(); } }
}
