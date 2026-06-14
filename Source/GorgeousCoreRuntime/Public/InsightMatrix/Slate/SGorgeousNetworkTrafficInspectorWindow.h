// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|            Gorgeous Core - Network Traffic Inspector Window               |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class UGorgeousObjectVariable;
class SGorgeousInsightMiniLineChart;

/**
 * Companion window that displays per-variable replication stream configuration,
 * bandwidth budgets, update frequencies, and Root Network Stack channel
 * subscriptions.
 *
 * Features a rolling line graph at the top that captures a new data point
 * every CaptureIntervalSec (default 1s, configurable) showing aggregate
 * bandwidth (KB/s sent + received) over time.
 *
 * Opened from the Core Insight Provider via the "Network Traffic Inspector"
 * action.  Follows the singleton-window + Flow Tree design language.
 */
class GORGEOUSCORERUNTIME_API SGorgeousNetworkTrafficInspectorWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousNetworkTrafficInspectorWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	static void Open();
	static void Close();
	static bool IsOpen();

	// ── List item ────────────────────────────────────────────────────────
	struct FStreamItem
	{
		TWeakObjectPtr<UGorgeousObjectVariable> Variable;
		FString DisplayLabel;
		FString BackendLabel;
		float UpdateFrequency = 0.f;
		float BandwidthBudgetKB = 0.f;
		int32 Priority = 0;
		bool bSupportsMulticast = false;
		bool bRespectsAccessPolicy = false;
		FString ChannelName;
		FString AccessPolicyLabel;
		bool bHasARBinding = false;
		FString AREntryKey;
		FString AROwnerLabel;

		bool IsValid() const { return Variable.IsValid(); }
	};

private:
	void RefreshList();
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FStreamItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnSelectionChanged(TSharedPtr<FStreamItem> Item, ESelectInfo::Type SelectInfo);
	void RefreshDetailsPanel();
	FReply OnRefreshClicked();

	/** Captures a new graph sample from current snapshot and shifts the rolling window. */
	void CaptureGraphSample();

	// ── Data ─────────────────────────────────────────────────────────────
	TArray<TSharedPtr<FStreamItem>> StreamItems;
	TSharedPtr<SListView<TSharedPtr<FStreamItem>>> ListView;
	TSharedPtr<FStreamItem> SelectedItem;
	TSharedPtr<SVerticalBox> DetailsBox;

	// ── Rolling graph ────────────────────────────────────────────────────
	TSharedPtr<SGorgeousInsightMiniLineChart> BandwidthChart;
	TSharedPtr<STextBlock> GraphStatusLabel;

	/** Rolling bandwidth samples (KB/s). Most recent at the end. */
	TArray<double> BandwidthHistory;

	/** Rolling property sync count samples. */
	TArray<double> SyncCountHistory;

	/** Rolling RPC count samples (sent + received). */
	TArray<double> RPCCountHistory;

	/** Previous snapshot for delta computation. */
	int64 PrevBytesSent = 0;
	int64 PrevBytesReceived = 0;
	int32 PrevPropertySyncs = 0;
	int32 PrevRPCsSent = 0;
	int32 PrevRPCsReceived = 0;

	/** Number of data points to display in the graph. */
	static constexpr int32 GraphHistorySize = 60;

	/** Capture interval in seconds (configurable). */
	double CaptureIntervalSec = 1.0;

	/** Time of last graph capture. */
	double LastCaptureTime = 0.0;

	static TWeakPtr<SWindow> WindowWeak;
};
