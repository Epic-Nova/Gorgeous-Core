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

#include "InsightMatrix/GorgeousInsightMatrixProvider.h"

/** Core Insight Matrix provider for harness + Gauntlet orchestration status. */
class GORGEOUSCORERUNTIME_API FGorgeousCoreInsightMatrixProvider : public IGorgeousInsightMatrixProvider
{
public:
	static FName ProviderName();

	virtual FName GetProviderName() const override { return ProviderName(); }
	virtual FString GetPluginName() const override { return TEXT("Gorgeous-Core"); }
	virtual FText GetProviderDisplayName() const override;
	virtual void GatherStats(const FGorgeousInsightGatherContext& Context, TArray<FGorgeousInsightStat>& OutStats) const override;
	virtual void GatherCharts(TArray<FGorgeousInsightChartDefinition>& OutCharts) const override;
	virtual void GetActions(TArray<FGorgeousInsightAction>& OutActions) const override;
	virtual void ExecuteAction(FName ActionId, const FGorgeousInsightActionContext& Context) override;
	virtual void GetTests(TArray<FGorgeousInsightTest>& OutTests) const override;
	virtual FGorgeousInsightTestResult RunTest(FName TestId, const FGorgeousInsightTestContext& Context) override;
};
