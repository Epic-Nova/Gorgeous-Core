// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Object Variable Browser (Runtime)          |
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
#include "Widgets/Views/STreeView.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SComboBox.h"

class UGorgeousObjectVariable;
class UGorgeousRootObjectVariable;
class SEditableTextBox;
class SCheckBox;

/**
 * General-purpose tree browser for all UGorgeousObjectVariable instances
 * registered across every root.
 *
 * The browser is opened from the Insight Matrix debug panel via an action
 * on the GorgeousCore provider.  It provides:
 *
 *  - A navigable tree that mirrors the root / parent / child hierarchy
 *  - Per-variable details panel (identity, value preview, child count)
 *  - Optional replication column showing replication status when networking
 *    is enabled on the variable
 *  - Real-time refresh on a configurable timer
 *  - Root filter and text search
 *
 * This widget is intentionally a general-purpose Object Variable inspector.
 * AutoReplication metadata is displayed as bonus info when present but is
 * NOT a prerequisite.
 */
class GORGEOUSCORERUNTIME_API SGorgeousObjectVariableBrowserWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousObjectVariableBrowserWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SGorgeousObjectVariableBrowserWindow();

	/** Opens the browser as a standalone Slate window (singleton). */
	static void Open();

	/** Closes the browser window if open. */
	static void Close();

	/** Returns true if the browser window is currently open. */
	static bool IsOpen();

	// ── World/Connection descriptor ────────────────────────────────────────
	struct FWorldConnection
	{
		TWeakObjectPtr<UWorld> World;
		FString Label;			// "Server", "Client 0", "Standalone", etc.
		FLinearColor AccentColor;
		bool bIsServer = false;
		int32 ClientIndex = INDEX_NONE;

		bool IsValid() const { return World.IsValid(); }
	};

	// ── Tree item ──────────────────────────────────────────────────────────
	struct FTreeItem
	{
		TWeakObjectPtr<UGorgeousObjectVariable> Variable;

		/** Root name this variable belongs to (cached for display). */
		FName RootName;

		/** Children of this node in the tree. */
		TArray<TSharedPtr<FTreeItem>> Children;

		/** Cached display text. */
		FString DisplayLabel;

		/** True when this node represents a root-level object variable. */
		bool bIsRoot = false;

		/** World this variable belongs to (for multi-PIE differentiation). */
		TWeakObjectPtr<UWorld> OwningWorld;

		/** Cached label for the owning world/connection. */
		FString WorldLabel;

		/** Accent color derived from the connection. */
		FLinearColor ConnectionAccent = FLinearColor(0.45f, 0.45f, 0.50f, 0.85f);

		bool IsValid() const { return Variable.IsValid(); }
	};

private:
	// ── Tree population ────────────────────────────────────────────────────
	void RefreshTree();
	void PopulateChildrenRecursive(const TSharedPtr<FTreeItem>& ParentItem, UGorgeousObjectVariable* Variable, FName RootName);

	// ── STreeView callbacks ────────────────────────────────────────────────
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedPtr<FTreeItem> Item, TArray<TSharedPtr<FTreeItem>>& OutChildren);
	void OnSelectionChanged(TSharedPtr<FTreeItem> Item, ESelectInfo::Type SelectInfo);

	// ── Detail panel ───────────────────────────────────────────────────────
	TSharedRef<SWidget> BuildDetailsPanel();
	void RefreshDetailsPanel();

	// ── Property manipulator ───────────────────────────────────────────────
	void BuildManipulatorSection(UGorgeousObjectVariable* Variable);
	void OnDisplayNameCommitted(const FText& NewText, ETextCommit::Type CommitType);
	void OnPersistentChanged(ECheckBoxState NewState);
	void OnReplicatesChanged(ECheckBoxState NewState);
	void OnSharedNetworkStackChanged(ECheckBoxState NewState);
	void OnValueTextCommitted(const FText& NewText, ETextCommit::Type CommitType);

	// ── Auto-refresh ───────────────────────────────────────────────────────
	void OnVariableTreeChangedHandler();

	// ── Helpers ─────────────────────────────────────────────────────────────
	FText GetFilterText() const;
	void OnFilterTextChanged(const FText& InFilterText);
	void OnRootFilterChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);
	void OnWorldFilterChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	FReply OnRefreshClicked();
	FReply OnExpandAllClicked();
	FReply OnCollapseAllClicked();
	bool PassesFilter(const FTreeItem& Item) const;

	static FString BuildValuePreview(const UGorgeousObjectVariable* Variable);
	static FString BuildClassLabel(const UGorgeousObjectVariable* Variable);
	static FString BuildReplicationLabel(const UGorgeousObjectVariable* Variable);

	void RefreshWorldConnections();
	FWorldConnection FindConnectionForWorld(UWorld* World) const;
	static FLinearColor GetConnectionColor(int32 Index, bool bIsServer);

	// ── Data ───────────────────────────────────────────────────────────────
	TArray<TSharedPtr<FTreeItem>> RootItems;
	TSharedPtr<STreeView<TSharedPtr<FTreeItem>>> TreeView;

	TSharedPtr<FTreeItem> SelectedItem;
	TSharedPtr<SVerticalBox> DetailsBox;

	FString FilterText;
	FName RootFilter; // NAME_None = all roots
	TArray<TSharedPtr<FName>> RootFilterOptions;

	// ── World/Connection filtering ─────────────────────────────────────────
	TArray<FWorldConnection> AvailableConnections;
	TArray<TSharedPtr<FString>> WorldFilterOptions;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> WorldComboBox;
	FString WorldFilter; // empty = "All Worlds"
	bool bIsRefreshing = false; // Guard against SetSelectedItem → OnWorldFilterChanged → RefreshTree recursion

	// ── Delegate handle ────────────────────────────────────────────────────
	FDelegateHandle TreeChangedDelegateHandle;

	// ── Window management ──────────────────────────────────────────────────
	static TWeakPtr<SWindow> BrowserWindowWeak;
};
