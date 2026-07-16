#pragma once

#include "CoreMinimal.h"
#include "GorgeousFeedbackEffect.h"
#include "GorgeousCompositeFeedbackEffect.generated.h"

/**
 * A feedback effect that groups several child effects and plays them together.
 *
 * Example: a sword hit could be expressed as a composite that fires a Niagara fire burst,
 * a hit sound, a controller rumble and a camera shake all at once. The composite forwards
 * the same feedback context to every child and only runs children whose own CanExecute passes.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousCompositeFeedbackEffect  : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

public:

	/** The child effects that are executed together when this composite runs. */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="Composite")
	TArray<TObjectPtr<UGorgeousFeedbackEffect>> Children;

public:

	/** Executes every child effect whose own conditions are satisfied. */
	virtual void Execute_Implementation(
		const FGorgeousFeedbackContext& Context
	) override;

	/** Returns true if at least one child effect is currently able to execute. */
	virtual bool CanExecute_Implementation(
		const FGorgeousFeedbackContext& Context
	) const override;

#if WITH_EDITOR

	/** Previews every child effect in the editor. */
	virtual void Preview_Internal() override;

	/** Stops the preview of every child effect in the editor. */
	virtual void StopPreview_Internal() override;

#endif

};
