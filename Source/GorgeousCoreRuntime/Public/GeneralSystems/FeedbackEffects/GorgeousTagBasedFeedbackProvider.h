#pragma once

#include "GorgeousFeedbackProvider.h"
#include "GorgeousTagBasedFeedbackProvider.generated.h"

/**
 * A tag-driven feedback provider.
 *
 * Maps gameplay tags to feedback definitions. When queried it returns the definition whose tag
 * is present in the feedback context, allowing designers to author feedback per situation
 * (e.g. surface type, damage type, item rarity) entirely through data without writing code.
 */
UCLASS(BlueprintType, EditInlineNew, DisplayName="Tag Based Feedback Provider")
class GORGEOUSCORERUNTIME_API UGorgeousTagBasedFeedbackProvider : public UGorgeousFeedbackProvider
{
	GENERATED_BODY()

public:

	/** Tag-to-feedback mapping. The first entry whose tag is present in the context is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feedback")
	TMap<FGameplayTag, FGorgeousFeedbackDefinition> TagToFeedback;

	/** Optional fallback used when no mapped tag is present in the context. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feedback")
	FGorgeousFeedbackDefinition DefaultFeedback;

	/**
	 * Resolves the feedback for the given context.
	 *
	 * Iterates the context tags and returns the definition mapped to the first matching tag.
	 * Falls back to DefaultFeedback when no mapped tag is present. Returns false only if neither
	 * a mapped tag nor a default definition produced any effects.
	 *
	 * @param Context     The runtime situation the feedback is requested for.
	 * @param OutFeedback The resolved feedback definition to populate.
	 * @return True if a non-empty feedback definition was resolved, false otherwise.
	 */
	virtual bool ProvideFeedback_Implementation(
		const FGorgeousFeedbackContext& Context,
		FGorgeousFeedbackDefinition& OutFeedback
	) const override;
};
