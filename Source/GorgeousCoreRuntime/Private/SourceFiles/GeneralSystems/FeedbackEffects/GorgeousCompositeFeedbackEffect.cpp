#include "FeedbackEffects/GorgeousCompositeFeedbackEffect.h"

void UGorgeousCompositeFeedbackEffect::Execute_Implementation(const FGorgeousFeedbackContext& Context)
{
	// Execute every child effect whose own execution conditions are satisfied. Children are
	// responsible for honoring their enabled state, tags and scheduling through CanExecute.
	for (UGorgeousFeedbackEffect* Child : Children)
	{
		if (Child && Child->CanExecute(Context))
		{
			Child->Execute(Context);
		}
	}
}

bool UGorgeousCompositeFeedbackEffect::CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const
{
	if (!bEnabled || Children.Num() == 0)
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
	// Iterate the children so the editor preview reflects the full combination of effects.
	for (UGorgeousFeedbackEffect* Child : Children)
	{
		if (Child)
		{
			Child->Preview();
		}
	}
}

void UGorgeousCompositeFeedbackEffect::StopPreview_Internal()
{
	for (UGorgeousFeedbackEffect* Child : Children)
	{
		if (Child)
		{
			Child->StopPreview();
		}
	}
}
#endif
