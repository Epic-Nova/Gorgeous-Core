// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"

enum class ECheckBoxState : uint8;

/**
 * Unified debug panel scaffold for the Insight Matrix.
 */
class GORGEOUSCORERUNTIME_API SGorgeousInsightDebugPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousInsightDebugPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	struct FInsightPanelState
	{
		FName SelectedProvider = NAME_None;
		FString ProviderFilter;
		bool bRunWithHarness = false;
		TMap<FName, TMap<FName, FString>> TestInputValues;
		TMap<FName, double> StatWarningThresholds;
		TMap<FName, double> StatCriticalThresholds;
	};

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGorgeousInsightPanelStateChanged, const FInsightPanelState&);

	FInsightPanelState ExportState() const;
	void ImportState(const FInsightPanelState& State);
	FOnGorgeousInsightPanelStateChanged& OnStateChanged() { return StateChanged; }

	/** Rebuild provider list and refresh stats/actions for the selected provider. */
	void RefreshFromSubsystem();

	struct FStatRow
	{
		FGorgeousInsightStat Stat;
	};
	

	struct FProviderEntry
	{
		FName ProviderName = NAME_None;
		FText DisplayName;
		int32 StatCount = 0;
		int32 ActionCount = 0;
		int32 TestCount = 0;
	};


	struct FTestRow
	{
		FGorgeousInsightTest Test;
		FGorgeousInsightTestResult LastResult;
		bool bHasRun = false;
	};

	FText FormatTestResultText(const FTestRow& Row) const;
	FText FormatStatValue(const FGorgeousInsightStat& Stat) const;
	FReply OnQueueTestClicked(TSharedPtr<FTestRow> RowData);
	FReply OnRunTestClicked(TSharedPtr<FTestRow> RowData);
	
	TSharedRef<SWidget> BuildTestInputsWidget(const TSharedPtr<FTestRow>& RowData);
	FSlateColor GetStatValueColor(const FGorgeousInsightStat& Stat) const;
	TSharedRef<SWidget> BuildStatThresholdWidget(const FGorgeousInsightStat& Stat);
	FText FormatBaselineText(const FTestRow& Row) const;
	FLinearColor GetTestStatusAccent(const FTestRow& Row) const;

private:
	
	void RefreshProviders();
	void RefreshProviderData();
	void RebuildActions();

	TSharedRef<SWidget> BuildProviderList();
	TSharedRef<SWidget> BuildStatsList();
	TSharedRef<SWidget> BuildChartsPanel();
	TSharedRef<SWidget> BuildActionsPanel();
	TSharedRef<SWidget> BuildTestsList();
	TSharedRef<SWidget> BuildToolbar();
	void RebuildCharts();

	void OnProviderSelectionChanged(TSharedPtr<FProviderEntry> Item, ESelectInfo::Type SelectInfo);
	FReply OnRefreshClicked();
	FReply OnActionClicked(FName ActionId);

	FReply OnRunQueuedTestsClicked();
	FReply OnExportStatsCsvClicked();
	FReply OnExportStatsJsonClicked();
	void OnProviderFilterChanged(const FText& InFilterText);
	void OnHarnessToggleChanged(ECheckBoxState NewState);
	ECheckBoxState GetHarnessToggleState() const;
	void ApplyHarnessParameters(TMap<FString, FString>& Parameters) const;
	void ApplyTestInputParameters(const FGorgeousInsightTest& Test, TMap<FString, FString>& Parameters) const;
	void SetTestInputValue(FName TestId, FName InputId, const FString& Value);
	FString GetTestInputValue(FName TestId, FName InputId, const FString& DefaultValue) const;
	void SetStatThresholdValue(const FGorgeousInsightStat& Stat, bool bCritical, const FString& Value);
	FString GetStatThresholdText(const FGorgeousInsightStat& Stat, bool bCritical) const;
	FName MakeStatThresholdKey(const FGorgeousInsightStat& Stat) const;
	const FGorgeousInsightTestResult* GetBaselineResult(FName ProviderName, FName TestId) const;
	void BroadcastStateChanged();

	FText GetSelectedProviderText() const;
	FText GetStatsCountText() const;
	FText GetActionsCountText() const;
	FText GetTestsCountText() const;
	FText FormatProviderBadgeText(const FProviderEntry& Entry) const;

	TArray<TSharedPtr<FProviderEntry>> ProviderItems;
	TSharedPtr<SListView<TSharedPtr<FProviderEntry>>> ProviderListView;

	TArray<TSharedPtr<FStatRow>> StatItems;
	TSharedPtr<SListView<TSharedPtr<FStatRow>>> StatsListView;
	TSharedPtr<SUniformGridPanel> ChartsGrid;

	TArray<FGorgeousInsightAction> ActionItems;
	TSharedPtr<SUniformGridPanel> ActionsGrid;

	TArray<TSharedPtr<FTestRow>> TestItems;
	TSharedPtr<SListView<TSharedPtr<FTestRow>>> TestsListView;

	FString ProviderFilter;

	FName SelectedProvider = NAME_None;
	bool bRunWithHarness = false;
	TMap<FName, TMap<FName, FString>> TestInputValues;
	TMap<FName, double> StatWarningThresholds;
	TMap<FName, double> StatCriticalThresholds;
	FOnGorgeousInsightPanelStateChanged StateChanged;
	bool bSuppressStateBroadcast = false;
	bool bRefreshingProviderSelection = false;
};
