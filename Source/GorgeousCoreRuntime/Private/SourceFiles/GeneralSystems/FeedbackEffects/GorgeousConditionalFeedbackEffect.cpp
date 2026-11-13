#include "FeedbackEffects/GorgeousConditionalFeedbackEffect.h"

void UGorgeousConditionalFeedbackEffect::PerformExecute(const FGorgeousFeedbackContext& Context)
{
	// Only run the wrapped effect if both the tag requirement and the wrapped effect itself agree.
	if (CanExecute(Context) && Effect)
	{
		Effect->PlayFeedback(Context);
	}
}

void UGorgeousConditionalFeedbackEffect::PerformExecuteWithCompletion(const FGorgeousFeedbackContext& Context, FSimpleDelegate OnCompleted)
{
	if (Effect && CanExecute(Context)) { Effect->PlayScheduled(Context, OnCompleted); }
	else { OnCompleted.ExecuteIfBound(); }
}

bool UGorgeousConditionalFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	if (!Super::CanExecute_Implementation(Context) || !Effect)
	{
		return false;
	}

	// The context must carry every required tag before the wrapped effect is allowed to run.
	if (!Context.ContextTags.HasAll(RequiredTags))
	{
		return false;
	}

	return Effect->CanExecute(Context);
}

void UGorgeousConditionalFeedbackEffect::StopActiveEffects()
{
	if (Effect) { Effect->StopFeedback(); }
}
