// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Views/SListView.h"
#include "AutoReplication/Tracking/GorgeousRPCDebugTracker.h"
#include "Widgets/Input/SComboBox.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

// ════════════════════════════════════════════════════════════════════════════
//  SGorgeousRPCResponseTimeline
//
//  Self-contained, persistent vertical response timeline widget.
//  Displayed in the right sub-pane of the RPC Inspector detail area.
//  Anti-flicker: UpdateEntry() rebuilds internal children only when visible
//  data has actually changed, so live ticks cost nothing when idle.
// ════════════════════════════════════════════════════════════════════════════
class GORGEOUSCORERUNTIME_API SGorgeousRPCResponseTimeline : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousRPCResponseTimeline) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Feed a fresh entry snapshot; rebuilds content only when visible data changed. */
	void UpdateEntry(const FGorgeousRPCDebugEntry& NewEntry);

	/** Clear the timeline (no selection). */
	void Clear();

private:
	/** Rebuild the full internal Slate tree from CachedEntry. */
	void RebuildContent();

	/** Returns true when any visually-significant field on NewEntry differs from cache. */
	static bool HasVisiblyChanged(const FGorgeousRPCDebugEntry& Old, const FGorgeousRPCDebugEntry& New);

	// ── Helpers for building timeline rows ──────────────────────────────
	/** Build one timeline node row (initialised event or per-responder). */
	TSharedRef<SWidget> MakeNodeRow(
		const FString& TimeLabel,
		bool bIsFirst,
		bool bIsLast,
		bool bIsPending,
		bool bIsServer,
		const FString& ResponderLabel,
		double LatencyMs,
		bool bHasReturnValue,
		const FString& ReturnValuePreview,
		EGorgeousRPCReadyState ReadyState) const;

	/** Build the coloured latency chip.  Returns null slot when LatencyMs <= 0. */
	TSharedRef<SWidget> MakeLatencyChip(double LatencyMs) const;

	/** Build the ready-state badge.  Returns Collapsed box for Ready state. */
	TSharedRef<SWidget> MakeReadyStateBadge(EGorgeousRPCReadyState ReadyState, bool bIsPending) const;

	// ── State ────────────────────────────────────────────────────────────
	FGorgeousRPCDebugEntry CachedEntry;
	bool bHasEntry = false;
};

// ════════════════════════════════════════════════════════════════════════════
//  SGorgeousRPCInspectorWindow
// ════════════════════════════════════════════════════════════════════════════

/**
 * Live RPC Inspector companion window.
 *
 * Layout (three panes):
 *   ┌─────────────┬────────────────────┬─────────────────────┐
 *   │  RPC Feed   │  Request Info +    │  Response Timeline  │
 *   │  (list)     │  Arguments         │  (custom widget)    │
 *   └─────────────┴────────────────────┴─────────────────────┘
 *
 * Anti-flicker strategy:
 *   - Left details pane is rebuilt ONLY on actual selection change (GUID diff).
 *   - Timeline widget updates in-place, internally dirty-checking each tick.
 *   - List refresh restores selection by GUID without triggering a full rebuild.
 */
class GORGEOUSCORERUNTIME_API SGorgeousRPCInspectorWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousRPCInspectorWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	static void Open();
	static void Close();
	static bool IsOpen();

	// ── List item wrapping a debug entry snapshot ────────────────────────
	struct FRPCListItem
	{
		FGorgeousRPCDebugEntry Entry;		bool bIsMulticastChild = false;  // True if this is a secondary dispatch of a grouped multicast
		FGuid ParentGuid;                 // GUID of the primary entry if this is a child
		TArray<FGuid> ChildGuids;         // GUIDs of related dispatches if this is primary		bool IsValid() const { return Entry.RequestGuid.IsValid(); }
	};

private:
	// ── List ─────────────────────────────────────────────────────────────
	void RefreshList();
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FRPCListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnSelectionChanged(TSharedPtr<FRPCListItem> Item, ESelectInfo::Type SelectInfo);

	// ── Detail panel ─────────────────────────────────────────────────────
	/** Full rebuild of the left details box (Request Info + Arguments). */
	void RebuildLeftDetails(const FGorgeousRPCDebugEntry& Entry);
	/** Clear both sub-panels (no selection). */
	void ClearDetailPanels();

	// ── Helpers ──────────────────────────────────────────────────────────
	bool PassesFilter(const FGorgeousRPCDebugEntry& Entry) const;	/** Detects if two entries are related multicasts (same Key+Handler+Type within 1 sec). */
	bool AreMulticastsRelated(const FGorgeousRPCDebugEntry& A, const FGorgeousRPCDebugEntry& B) const;
	/** Collects all responses from an entry and its related multicast children. */
	TArray<FGorgeousRPCResponseSnapshot> GetCombinedResponses(const FGorgeousRPCDebugEntry& Entry) const;
	/** Collects all responses from an item and its related multicast children (more efficient). */
	TArray<FGorgeousRPCResponseSnapshot> GetCombinedResponses(const TSharedPtr<FRPCListItem>& Item) const;
	FReply OnLivePauseToggled();
	FReply OnClearClicked();

	// Reusable row builder used by RebuildLeftDetails
	void AddDetailRow(const FString& Label, const FString& Value,
		const FLinearColor& ValueColor = FLinearColor::White);
	void AddDetailSectionHeader(const FString& Title, const FLinearColor& AccentColor);

	// ── Widgets ──────────────────────────────────────────────────────────
	TSharedPtr<SListView<TSharedPtr<FRPCListItem>>> ListView;
	TSharedPtr<STextBlock>                          LivePauseLabel;
	TSharedPtr<SVerticalBox>                        LeftDetailsBox;
	TSharedPtr<SGorgeousRPCResponseTimeline>        TimelineWidget;

	// Type filter combo
	TArray<TSharedPtr<FString>>                     TypeFilterOptions;
	TSharedPtr<SComboBox<TSharedPtr<FString>>>      TypeFilterCombo;
	TSharedPtr<FString>                             CurrentTypeFilter;

	// ── Data ─────────────────────────────────────────────────────────────
	TArray<TSharedPtr<FRPCListItem>> RPCItems;
	TArray<TSharedPtr<FRPCListItem>> FilteredRPCItems; // RPCItems without multicast children (for list display)
	TSharedPtr<FRPCListItem>         SelectedItem;

	/** GUID of the entry that LeftDetailsBox was last built for. */
	FGuid LastBuiltDetailGuid;

	bool bIsLive = true;
	FString FilterText;
	int32 TypeFilterIndex = 0; // 0=All, 1=Server, 2=Client, 3=Multicast
	double LastRefreshTime = 0.0;
	static constexpr double RefreshIntervalSec = 0.25;

	static TWeakPtr<SWindow> WindowWeak;
};