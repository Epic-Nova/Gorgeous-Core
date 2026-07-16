#include "FeedbackEffects/GorgeousTagBasedFeedbackProvider.h"

bool UGorgeousTagBasedFeedbackProvider::ProvideFeedback_Implementation(
	const FGorgeousFeedbackContext& Context,
	FGorgeousFeedbackDefinition& OutFeedback
) const
{
	// Prefer an explicit tag mapping: the first context tag that has an entry wins.
	for (const FGameplayTag& ContextTag : Context.ContextTags)
	{
		if (const FGorgeousFeedbackDefinition* Mapped = TagToFeedback.Find(ContextTag))
		{
			OutFeedback = *Mapped;
			return Mapped->Effects.Num() > 0;
		}
	}

	// Fall back to the default definition when no mapped tag matched the context.
	OutFeedback = DefaultFeedback;
	return DefaultFeedback.Effects.Num() > 0;
}
