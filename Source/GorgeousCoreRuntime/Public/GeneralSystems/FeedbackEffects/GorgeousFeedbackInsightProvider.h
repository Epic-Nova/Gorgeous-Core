// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "InsightMatrix/GorgeousInsightMatrixProvider.h"

/**
 * Insight Matrix provider for the Feedback Effects system.
 *
 * Exposes runtime counters (registered providers, triggered/resolved feedback and total effects
 * executed) to the unified debug panel. The dispatcher pushes live values into this provider; the
 * provider itself is a plain modular feature registered once by the module (it is not a UObject,
 * matching the other Gorgeous Insight providers).
 */
class GORGEOUSCORERUNTIME_API FGorgeousFeedbackInsightProvider : public IGorgeousInsightMatrixProvider
{
public:
	static FName ProviderName();

	virtual FName GetProviderName() const override { return ProviderName(); }
	virtual FString GetPluginName() const override { return TEXT("Gorgeous-Core"); }
	virtual FText GetProviderDisplayName() const override;
	virtual void GatherStats(const FGorgeousInsightGatherContext& Context, TArray<FGorgeousInsightStat>& OutStats) const override;
	virtual void GetActions(TArray<FGorgeousInsightAction>& OutActions) const override;
	virtual void ExecuteAction(FName ActionId, const FGorgeousInsightActionContext& Context) override;

	/** Live counters pushed in by the feedback dispatcher. */
	struct FFeedbackCounters
	{
		int32 RegisteredProviders = 0;
		int32 TriggeredCount = 0;
		int32 ResolvedCount = 0;
		int32 ExecutedEffects = 0;
	} Counters;
};
