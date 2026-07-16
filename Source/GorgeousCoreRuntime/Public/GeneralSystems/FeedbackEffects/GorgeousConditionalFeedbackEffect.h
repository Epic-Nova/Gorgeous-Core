#pragma once

#include "CoreMinimal.h"
#include "GorgeousFeedbackEffect.h"
#include "GorgeousConditionalFeedbackEffect.generated.h"

/**
 * A feedback effect that wraps another effect and only runs it when tag conditions are met.
 *
 * Use this to gate a single effect behind gameplay tags carried by the feedback context, for
 * example playing an upgraded impact effect only when the source item is tagged as legendary.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousConditionalFeedbackEffect  : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The effect that is executed when the required tags are present. */
	UPROPERTY(EditAnywhere, Instanced, Category="Condition")
	TObjectPtr<UGorgeousFeedbackEffect> Effect;

public:

	/** Returns true only if the required tags are present and the wrapped effect can execute. */
	virtual bool CanExecute_Implementation(
		const FGorgeousFeedbackContext& Context
	) const override;

	/** Executes the wrapped effect when the conditions are satisfied. */
	virtual void Execute_Implementation(
		const FGorgeousFeedbackContext& Context
	) override;

};
