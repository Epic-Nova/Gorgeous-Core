// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GorgeousFeedbackEffectTypes.h"
#include "GorgeousFeedbackSelector.generated.h"

class UGorgeousFeedbackEffect;

/**
 * Policy object that fully owns selection when a feedback definition uses
 * EGorgeousFeedbackSelectionMode::ECustom.
 *
 * The dispatcher hands the selector the already-filter-passed candidate effects plus the feedback
 * context, and uses whatever ordered list it returns as the final set to play. This gives absolute
 * freedom: a selector may keep, drop, reorder, duplicate or even synthesize brand-new effects
 * (e.g. spawn an instanced effect at runtime) — there is no constraint on the result.
 *
 * Subclass and override SelectEffects (BlueprintNativeEvent, so it can be implemented in either
 * C++ or Blueprint). The default implementation returns the candidates unchanged.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, DisplayName="Feedback Selector")
class GORGEOUSCORERUNTIME_API UGorgeousFeedbackSelector : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Decides which effects from the candidate set should actually play.
	 *
	 * @param Candidates The effects that survived the definition's FilterTags (already gated).
	 * @param Context    The feedback context the definition is executing against.
	 * @param OutSelected The ordered list of effects to play. May be any subset, in any order,
	 *                     and may include effects not present in Candidates (full creative freedom).
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Gorgeous|Feedback")
	void SelectEffects(
		const TArray<UGorgeousFeedbackEffect*>& Candidates,
		const FGorgeousFeedbackContext& Context,
		TArray<UGorgeousFeedbackEffect*>& OutSelected) const;
};
