// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#pragma once

#include "GorgeousInsightMatrixTypes.h"

/**
 * Runtime provider interface for the unified debug panel.
 * Each plugin should implement a single provider instance and register it.
 */
class GORGEOUSCORERUNTIME_API IGorgeousInsightMatrixProvider : public IModularFeature
{
public:
	virtual ~IGorgeousInsightMatrixProvider() = default;

	/** Returns the feature name used for modular feature registration. */
	static FName GetFeatureName() { return FName(TEXT("GorgeousInsightMatrixProvider")); }

	/** Stable provider key (typically plugin name). */
	virtual FName GetProviderName() const = 0;

	/** Describes the provider for display. */
	virtual FText GetProviderDisplayName() const { return FText::FromName(GetProviderName()); }

	/** Gather current stat snapshots. */
	virtual void GatherStats(TArray<FGorgeousInsightStat>& OutStats) const {}

	/** Gather provider-defined charts. */
	virtual void GatherCharts(TArray<FGorgeousInsightChartDefinition>& OutCharts) const {}

	/** Declare available actions. */
	virtual void GetActions(TArray<FGorgeousInsightAction>& OutActions) const {}

	/** Execute a specific action. */
	virtual void ExecuteAction(FName ActionId, const FGorgeousInsightActionContext& Context) {}

	/** Declare available tests. */
	virtual void GetTests(TArray<FGorgeousInsightTest>& OutTests) const {}

	/** Run a specific test. */
	virtual FGorgeousInsightTestResult RunTest(FName TestId, const FGorgeousInsightTestContext& Context) { return FGorgeousInsightTestResult(); }
};
