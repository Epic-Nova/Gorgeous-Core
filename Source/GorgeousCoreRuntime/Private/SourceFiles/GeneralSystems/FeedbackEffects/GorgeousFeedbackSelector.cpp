// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/FeedbackEffects/GorgeousFeedbackSelector.h"

void UGorgeousFeedbackSelector::SelectEffects_Implementation(
	const TArray<UGorgeousFeedbackEffect*>& Candidates,
	const FGorgeousFeedbackContext& Context,
	TArray<UGorgeousFeedbackEffect*>& OutSelected) const
{
	// Default behaviour: no customisation, pass every candidate through unchanged.
	OutSelected = Candidates;
}
