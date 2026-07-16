// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/FeedbackEffects/GorgeousFeedbackDispatcher.h"

#include "FeedbackEffects/GorgeousFeedbackEffectTypes.h"
#include "FeedbackEffects/GorgeousFeedbackProvider.h"
#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "FeedbackEffects/GorgeousFeedbackSelector.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

namespace
{
	/** Filters a definition's effects down to those whose Tags match the definition's FilterTags. */
	TArray<UGorgeousFeedbackEffect*> FilterDefinitionEffects(const FGorgeousFeedbackDefinition& Definition)
	{
		TArray<UGorgeousFeedbackEffect*> Result;
		Result.Reserve(Definition.Effects.Num());

		for (UGorgeousFeedbackEffect* Effect : Definition.Effects)
		{
			// The per-effect Tags property is consumed here: when the definition carries filter
			// tags, only effects that own at least one of them are kept. With no filter the full
			// set is retained.
			if (Effect && (Definition.FilterTags.IsEmpty() || Effect->Tags.HasAny(Definition.FilterTags)))
			{
				Result.Add(Effect);
			}
		}

		return Result;
	}

	/** Applies the definition's selection mode, consuming each effect's Weight where relevant. */
	TArray<UGorgeousFeedbackEffect*> SelectDefinitionEffects(
		const FGorgeousFeedbackDefinition& Definition,
		const FGorgeousFeedbackContext& Context)
	{
		TArray<UGorgeousFeedbackEffect*> Candidates = FilterDefinitionEffects(Definition);
		if (Candidates.IsEmpty())
		{
			return Candidates;
		}

		switch (Definition.SelectionMode)
		{
		case EGorgeousFeedbackSelectionMode::EAll:
			return Candidates;

		case EGorgeousFeedbackSelectionMode::ERandom:
			return { Candidates[FMath::RandRange(0, Candidates.Num() - 1)] };

		case EGorgeousFeedbackSelectionMode::EWeightedRandom:
		{
			// Weight (per-effect) drives the roll: each candidate's chance is proportional to its
			// Weight. Effects with Weight <= 0 are excluded from the weighted pool.
			double TotalWeight = 0.0;
			for (UGorgeousFeedbackEffect* Effect : Candidates)
			{
				TotalWeight += FMath::Max(0.0, static_cast<double>(Effect->Weight));
			}

			if (TotalWeight <= 0.0)
			{
				return { Candidates[FMath::RandRange(0, Candidates.Num() - 1)] };
			}

			double Roll = FMath::FRandRange(0.0, TotalWeight);
			double Accumulated = 0.0;
			for (UGorgeousFeedbackEffect* Effect : Candidates)
			{
				Accumulated += FMath::Max(0.0, static_cast<double>(Effect->Weight));
				if (Roll <= Accumulated)
				{
					return { Effect };
				}
			}

			return { Candidates.Last() };
		}

		case EGorgeousFeedbackSelectionMode::ESequential:
			// Sequential plays the candidates in order, each honoring its own scheduling. For a
			// single definition call this is equivalent to EAll at the top level, so we fall
			// through to returning every candidate.
			return Candidates;

		case EGorgeousFeedbackSelectionMode::ECustom:
		{
			// Absolute-freedom hook: the definition's Selector receives the filter-passed
			// candidates and returns the final, ordered set to play. With no selector assigned,
			// fall back to passing every candidate through unchanged.
			if (Definition.Selector)
			{
				TArray<UGorgeousFeedbackEffect*> Selected;
				Definition.Selector->SelectEffects(Candidates, Context, Selected);
				return Selected;
			}

			GT_W_LOG("GT.FeedbackEffects.Selection",
				TEXT("SelectionMode is Custom but no Selector is assigned; playing all candidates."));
			return Candidates;
		}

		default:
			return Candidates;
		}
	}
}

void UGorgeousFeedbackDispatcher::RegisterProvider(UGorgeousFeedbackProvider* Provider)
{
	if (Provider && !Providers.Contains(Provider))
	{
		Providers.Add(Provider);
	}
}

void UGorgeousFeedbackDispatcher::UnregisterProvider(UGorgeousFeedbackProvider* Provider)
{
	if (Provider)
	{
		Providers.Remove(Provider);
	}
}

int32 UGorgeousFeedbackDispatcher::GetRegisteredProviderCount() const
{
	return Providers.Num();
}

bool UGorgeousFeedbackDispatcher::ResolveDefinition(const FGorgeousFeedbackContext& Context, FGorgeousFeedbackDefinition& OutDefinition) const
{
	for (const TObjectPtr<UGorgeousFeedbackProvider>& Provider : Providers)
	{
		if (Provider && Provider->ProvideFeedback(Context, OutDefinition) && OutDefinition.Effects.Num() > 0)
		{
			return true;
		}
	}

	return false;
}

void UGorgeousFeedbackDispatcher::PlayDefinition(const FGorgeousFeedbackDefinition& Definition, const FGorgeousFeedbackContext& Context)
{
	const TArray<UGorgeousFeedbackEffect*> Effects = SelectDefinitionEffects(Definition, Context);

	// Surface the chosen selection mode + selection in the debug feed so designers can see
	// which effect actually fired for a given definition.
	const FString ModeName = StaticEnum<EGorgeousFeedbackSelectionMode>()->GetNameStringByValue(static_cast<int64>(Definition.SelectionMode));
	GT_I_LOG("GT.FeedbackEffects.PlayDefinition",
		TEXT("Playing definition with %d candidate(s) under selection mode %s."),
		Effects.Num(), *ModeName);

	for (UGorgeousFeedbackEffect* Effect : Effects)
	{
		if (Effect && Effect->CanExecute(Context))
		{
			const FString EffectName = Effect->DisplayName.IsEmpty() ? Effect->GetName() : Effect->DisplayName.ToString();

			if (!Effect->Description.IsEmpty())
			{
				GT_I_LOG("GT.FeedbackEffects.Execute", TEXT("Executing effect '%s' (%s)."), *EffectName, *Effect->Description.ToString());
			}
			else
			{
				GT_I_LOG("GT.FeedbackEffects.Execute", TEXT("Executing effect '%s'."), *EffectName);
			}

			Effect->Execute(Context);
			++TotalEffectsExecuted;

			FString RecentEntry = Effect->Description.IsEmpty()
				? EffectName
				: FString::Printf(TEXT("%s - %s"), *EffectName, *Effect->Description.ToString());
			RecentEffects.Add(RecentEntry);
			while (RecentEffects.Num() > RecentEffectsCapacity)
			{
				RecentEffects.RemoveAt(0);
			}
		}
	}
}

bool UGorgeousFeedbackDispatcher::TriggerFeedback(const FGorgeousFeedbackContext& Context)
{
	++TotalTriggers;

	FGorgeousFeedbackDefinition Definition;
	if (!ResolveDefinition(Context, Definition))
	{
		++TotalTriggersUnresolved;
		return false;
	}

	++TotalTriggersResolved;
	PlayDefinition(Definition, Context);
	return true;
}
