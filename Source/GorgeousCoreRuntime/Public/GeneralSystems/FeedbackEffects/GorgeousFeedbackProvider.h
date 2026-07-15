#pragma once

#include "GorgeousFeedbackEffectTypes.h"
#include "GorgeousFeedbackProvider.generated.h"

// @TODO: providers could theoretically also draw from the internet (if added then it would come in 2.0 of this system)

/**
 * Abstract base class for objects that provide feedback definitions.
 *
 * A feedback provider resolves a situation (described by an FGorgeousFeedbackContext) into a
 * concrete FGorgeousFeedbackDefinition. This allows feedback to be sourced dynamically, for
 * example from an item definition, a surface type or a gameplay tag, instead of being hardcoded.
 */
UCLASS(Abstract, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousFeedbackProvider : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Resolves the feedback that should play for the given context.
	 *
	 * @param Context    The runtime situation the feedback is requested for.
	 * @param OutFeedback The feedback definition to populate and return to the caller.
	 * @return True if a feedback definition was provided, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Feedback")
	bool ProvideFeedback(const FGorgeousFeedbackContext& Context,FGorgeousFeedbackDefinition& OutFeedback) const;
};
