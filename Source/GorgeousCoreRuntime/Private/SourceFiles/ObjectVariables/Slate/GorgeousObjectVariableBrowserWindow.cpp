// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Object Variable Browser (Runtime)          |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "ObjectVariables/Slate/GorgeousObjectVariableBrowserWindow.h"

#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

// ════════════════════════════════════════════════════════════════════════════
//  Static state
// ════════════════════════════════════════════════════════════════════════════

TWeakPtr<SWindow> SGorgeousObjectVariableBrowserWindow::BrowserWindowWeak;

// ════════════════════════════════════════════════════════════════════════════
//  Open / Close / IsOpen
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousObjectVariableBrowserWindow::Open()
{
	if (TSharedPtr<SWindow> Existing = BrowserWindowWeak.Pin())
	{
		Existing->BringToFront();
		return;
	}

	TSharedRef<SGorgeousObjectVariableBrowserWindow> Content = SNew(SGorgeousObjectVariableBrowserWindow);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("Object Variable Browser")))
		.ClientSize(FVector2D(960, 640))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		[
			Content
		];

	BrowserWindowWeak = Window;
	FSlateApplication::Get().AddWindow(Window);
}

void SGorgeousObjectVariableBrowserWindow::Close()
{
	if (TSharedPtr<SWindow> Window = BrowserWindowWeak.Pin())
	{
		Window->RequestDestroyWindow();
	}
	BrowserWindowWeak.Reset();
}

bool SGorgeousObjectVariableBrowserWindow::IsOpen()
{
	return BrowserWindowWeak.IsValid();
}

// ════════════════════════════════════════════════════════════════════════════
//  Destructor / Auto-refresh
// ════════════════════════════════════════════════════════════════════════════

SGorgeousObjectVariableBrowserWindow::~SGorgeousObjectVariableBrowserWindow()
{
	if (TreeChangedDelegateHandle.IsValid())
	{
		UGorgeousObjectVariable::OnVariableTreeChanged.Remove(TreeChangedDelegateHandle);
		TreeChangedDelegateHandle.Reset();
	}
}

void SGorgeousObjectVariableBrowserWindow::OnVariableTreeChangedHandler()
{
	RefreshTree();
	RefreshDetailsPanel();
}

// ════════════════════════════════════════════════════════════════════════════
//  Construct
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousObjectVariableBrowserWindow::Construct(const FArguments& InArgs)
{
	// Bind to variable-tree changes so we auto-refresh when new OVs are created
	TreeChangedDelegateHandle = UGorgeousObjectVariable::OnVariableTreeChanged.AddSP(
		this, &SGorgeousObjectVariableBrowserWindow::OnVariableTreeChangedHandler);

	// Build root filter options
	RootFilterOptions.Reset();
	RootFilterOptions.Add(MakeShared<FName>(NAME_None)); // "All Roots"
	for (const FName& RootName : UGorgeousRootObjectVariable::GetRegisteredRootNames())
	{
		RootFilterOptions.Add(MakeShared<FName>(RootName));
	}

	// Build world/connection filter options
	RefreshWorldConnections();
	WorldFilterOptions.Reset();
	WorldFilterOptions.Add(MakeShared<FString>(TEXT(""))); // "All Worlds"
	for (const FWorldConnection& Conn : AvailableConnections)
	{
		WorldFilterOptions.Add(MakeShared<FString>(Conn.Label));
	}

	// Details panel placeholder
	DetailsBox = SNew(SVerticalBox);

	ChildSlot
	[
		SNew(SVerticalBox)

		// ── Toolbar ────────────────────────────────────────────────────────
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.95f))
			.Padding(FMargin(8.f, 6.f))
			[
				SNew(SHorizontalBox)

				// Root filter
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SNew(SComboBox<TSharedPtr<FName>>)
					.OptionsSource(&RootFilterOptions)
					.OnSelectionChanged(this, &SGorgeousObjectVariableBrowserWindow::OnRootFilterChanged)
					.OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
					{
						const FString Label = Item->IsNone() ? TEXT("All Roots") : Item->ToString();
						return SNew(STextBlock)
							.Text(FText::FromString(Label))
							.Font(FAppStyle::GetFontStyle("SmallFont"));
					})
					.Content()
					[
						SNew(STextBlock)
						.Text_Lambda([this]() -> FText
						{
							return FText::FromString(RootFilter.IsNone() ? TEXT("All Roots") : RootFilter.ToString());
						})
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]

				// World/Connection filter
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SAssignNew(WorldComboBox, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&WorldFilterOptions)
					.OnSelectionChanged(this, &SGorgeousObjectVariableBrowserWindow::OnWorldFilterChanged)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
					{
						const FString Label = Item->IsEmpty() ? TEXT("All Worlds") : *Item;
						return SNew(STextBlock)
							.Text(FText::FromString(Label))
							.Font(FAppStyle::GetFontStyle("SmallFont"));
					})
					.Content()
					[
						SNew(STextBlock)
						.Text_Lambda([this]() -> FText
						{
							return FText::FromString(WorldFilter.IsEmpty() ? TEXT("All Worlds") : WorldFilter);
						})
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]

				// Search filter
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SNew(SSearchBox)
					.HintText(FText::FromString(TEXT("Filter variables...")))
					.OnTextChanged(this, &SGorgeousObjectVariableBrowserWindow::OnFilterTextChanged)
				]

				// Refresh
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 4, 0)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
					.ContentPadding(FMargin(10.f, 4.f))
					.OnClicked(this, &SGorgeousObjectVariableBrowserWindow::OnRefreshClicked)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Refresh")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]

				// Expand all
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 4, 0)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
					.ContentPadding(FMargin(10.f, 4.f))
					.OnClicked(this, &SGorgeousObjectVariableBrowserWindow::OnExpandAllClicked)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Expand All")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]

				// Collapse all
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
					.ContentPadding(FMargin(10.f, 4.f))
					.OnClicked(this, &SGorgeousObjectVariableBrowserWindow::OnCollapseAllClicked)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Collapse All")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]
			]
		]

		// ── Splitter: Tree + Details ───────────────────────────────────────
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// Left: tree
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.95f))
				.Padding(2)
				[
					SAssignNew(TreeView, STreeView<TSharedPtr<FTreeItem>>)
					.TreeItemsSource(&RootItems)
					.OnGenerateRow(this, &SGorgeousObjectVariableBrowserWindow::OnGenerateRow)
					.OnGetChildren(this, &SGorgeousObjectVariableBrowserWindow::OnGetChildren)
					.OnSelectionChanged(this, &SGorgeousObjectVariableBrowserWindow::OnSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
			]

			// Right: details
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.95f))
				.Padding(6)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						DetailsBox.ToSharedRef()
					]
				]
			]
		]
	];

	RefreshTree();
}

// ════════════════════════════════════════════════════════════════════════════
//  Tree population
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousObjectVariableBrowserWindow::RefreshTree()
{
	RootItems.Reset();

	// Update root filter options
	RootFilterOptions.Reset();
	RootFilterOptions.Add(MakeShared<FName>(NAME_None));
	for (const FName& RootName : UGorgeousRootObjectVariable::GetRegisteredRootNames())
	{
		RootFilterOptions.Add(MakeShared<FName>(RootName));
	}

	// Update world/connection list
	RefreshWorldConnections();
	WorldFilterOptions.Reset();
	WorldFilterOptions.Add(MakeShared<FString>(TEXT("")));
	for (const FWorldConnection& Conn : AvailableConnections)
	{
		WorldFilterOptions.Add(MakeShared<FString>(Conn.Label));
	}

	// Restore the combo box selection after rebuilding options
	if (WorldComboBox.IsValid())
	{
		TSharedPtr<FString> MatchingOption;
		for (const TSharedPtr<FString>& Opt : WorldFilterOptions)
		{
			if (*Opt == WorldFilter)
			{
				MatchingOption = Opt;
				break;
			}
		}
		if (MatchingOption.IsValid())
		{
			bIsRefreshing = true;
			WorldComboBox->SetSelectedItem(MatchingOption);
			bIsRefreshing = false;
		}
		else
		{
			// Selected world no longer exists, reset to All Worlds
			WorldFilter.Empty();
			bIsRefreshing = true;
			WorldComboBox->SetSelectedItem(WorldFilterOptions[0]);
			bIsRefreshing = false;
		}
	}

	const TArray<FName> RootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();

	for (const FName& RootName : RootNames)
	{
		// Skip if filtering by root and this isn't the one
		if (!RootFilter.IsNone() && RootFilter != RootName)
		{
			continue;
		}

		const TArray<UGorgeousObjectVariable*> RootVars = UGorgeousRootObjectVariable::GetRootVariableRegistry(RootName);

		for (UGorgeousObjectVariable* RootVar : RootVars)
		{
			if (!RootVar)
			{
				continue;
			}

			// Annotate with world/connection info, use the OV's own GetVariableWorld()
			// which walks Outer -> FallbackOwner -> Outer chain via UGorgeousBaseWorldContextUObject.
			// Root variables are created with GetTransientPackage() as outer, so
			// GetVariableWorld() returns nullptr until FallbackOwner is injected by
			// the networking ownership path (ClaimRootRegistryOwnership).  OVs
			// without a world are shown under every world filter (they're global).
			UWorld* VarWorld = RootVar->GetVariableWorld();
			FWorldConnection Conn = FindConnectionForWorld(VarWorld);

			// Apply world filter, pass through OVs that have no world context
			const bool bHasWorldContext = VarWorld != nullptr && Conn.IsValid();
			if (!WorldFilter.IsEmpty() && bHasWorldContext && Conn.Label != WorldFilter)
			{
				continue;
			}

			TSharedPtr<FTreeItem> RootItem = MakeShared<FTreeItem>();
			RootItem->Variable = RootVar;
			RootItem->RootName = RootName;
			RootItem->bIsRoot  = true;
			RootItem->OwningWorld = VarWorld;
			RootItem->WorldLabel = bHasWorldContext ? Conn.Label : TEXT("Global");
			RootItem->ConnectionAccent = bHasWorldContext ? Conn.AccentColor : FLinearColor(0.5f, 0.5f, 0.5f, 0.6f);
			RootItem->DisplayLabel = FString::Printf(TEXT("[%s] %s"),
				*RootName.ToString(),
				*(!RootVar->GetDisplayName().IsEmpty() ? RootVar->GetDisplayName() : RootVar->GetName()));

			PopulateChildrenRecursive(RootItem, RootVar, RootName);

			if (!FilterText.IsEmpty() && !PassesFilter(*RootItem) && RootItem->Children.Num() == 0)
			{
				continue;
			}

			RootItems.Add(RootItem);
		}
	}

	if (TreeView.IsValid())
	{
		TreeView->RequestTreeRefresh();
	}
}

void SGorgeousObjectVariableBrowserWindow::PopulateChildrenRecursive(const TSharedPtr<FTreeItem>& ParentItem, UGorgeousObjectVariable* Variable, FName RootName)
{
	if (!Variable)
	{
		return;
	}

	for (const auto& [ChildKey, ChildPtr] : Variable->VariableRegistry)
	{
		UGorgeousObjectVariable* Child = ChildPtr.Get();
		if (!Child)
		{
			continue;
		}

		TSharedPtr<FTreeItem> ChildItem = MakeShared<FTreeItem>();
		ChildItem->Variable = Child;
		ChildItem->RootName = RootName;
		ChildItem->bIsRoot  = false;
		ChildItem->DisplayLabel = !Child->GetDisplayName().IsEmpty() ? Child->GetDisplayName() : Child->GetName();

		// Annotate with world/connection info
		UWorld* ChildWorld = Child->GetVariableWorld();
		FWorldConnection ChildConn = FindConnectionForWorld(ChildWorld);
		const bool bChildHasWorld = ChildWorld != nullptr && ChildConn.IsValid();
		ChildItem->OwningWorld = ChildWorld;
		ChildItem->WorldLabel = bChildHasWorld ? ChildConn.Label : TEXT("Global");
		ChildItem->ConnectionAccent = bChildHasWorld ? ChildConn.AccentColor : FLinearColor(0.5f, 0.5f, 0.5f, 0.6f);

		PopulateChildrenRecursive(ChildItem, Child, RootName);

		if (!FilterText.IsEmpty() && !PassesFilter(*ChildItem) && ChildItem->Children.Num() == 0)
		{
			continue;
		}

		ParentItem->Children.Add(ChildItem);
	}
}

// ════════════════════════════════════════════════════════════════════════════
//  STreeView callbacks
// ════════════════════════════════════════════════════════════════════════════

TSharedRef<ITableRow> SGorgeousObjectVariableBrowserWindow::OnGenerateRow(TSharedPtr<FTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const UGorgeousObjectVariable* Var = Item->Variable.Get();
	const FString ClassLabel = Var ? BuildClassLabel(Var) : TEXT("(null)");
	const FString ReplicLabel = Var ? BuildReplicationLabel(Var) : TEXT("");
	const int32 ChildCount = Var ? Var->VariableRegistry.Num() : 0;
	const bool bNetworked = Var && Var->bSupportsNetworking;
	const bool bIsRoot = Item->bIsRoot;
	const bool bIsValid = Item->IsValid();

	// Accent color by variable kind, root: blue, networked: green, local: subtle gray
	const FLinearColor AccentColor = !bIsValid
		? FLinearColor(0.35f, 0.35f, 0.35f, 0.85f)
		: bIsRoot
			? FLinearColor(0.20f, 0.60f, 0.90f, 1.f)
			: bNetworked
				? FLinearColor(0.25f, 0.75f, 0.45f, 1.f)
				: FLinearColor(0.45f, 0.45f, 0.50f, 0.85f);

	// Label texts
	const FText TitleText = FText::FromString(Item->DisplayLabel);
	const FText SubtitleText = FText::FromString(ClassLabel);

	// Badge chip helper lambda
	auto MakeChip = [](const FText& Text, const FLinearColor& Color) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.Padding(FMargin(6.f, 2.f))
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Color)
			[
				SNew(STextBlock)
				.Text(Text)
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FLinearColor::White)
			];
	};

	// Build chip row
	TSharedRef<SHorizontalBox> ChipRow = SNew(SHorizontalBox);

	if (ChildCount > 0)
	{
		ChipRow->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				MakeChip(FText::FromString(FString::Printf(TEXT("Children: %d"), ChildCount)),
					FLinearColor(0.20f, 0.50f, 0.30f, 1.f))
			];
	}

	if (bNetworked)
	{
		const bool bReplicates = Var && Var->bReplicates;
		ChipRow->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				MakeChip(
					FText::FromString(bReplicates ? TEXT("Synced") : TEXT("Not synced")),
					bReplicates
						? FLinearColor(0.20f, 0.55f, 0.35f, 1.f)
						: FLinearColor(0.65f, 0.40f, 0.10f, 1.f))
			];
	}

	if (bIsRoot)
	{
		ChipRow->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				MakeChip(FText::FromString(Item->RootName.ToString()),
					FLinearColor(0.25f, 0.30f, 0.55f, 1.f))
			];
	}

	// Connection/world badge, shows which PIE world this variable belongs to
	if (!Item->WorldLabel.IsEmpty())
	{
		ChipRow->AddSlot()
			.AutoWidth()
			[
				MakeChip(FText::FromString(Item->WorldLabel),
					Item->ConnectionAccent)
			];
	}
	return SNew(STableRow<TSharedPtr<FTreeItem>>, OwnerTable)
		[
			SNew(SBorder)
			.Padding(FMargin(6.f))
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor_Lambda([this, Item]()
			{
				const bool bSelected = TreeView.IsValid() && Item.IsValid() && TreeView->IsItemSelected(Item);
				return bSelected
					? FLinearColor(0.10f, 0.45f, 0.85f, 0.35f)
					: FLinearColor(0.06f, 0.06f, 0.06f, 0.35f);
			})
			[
				SNew(SHorizontalBox)

				// Left accent bar
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
					.Padding(0.f)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(AccentColor)
					[
						SNew(SBox)
						.WidthOverride(4.f)
						.HeightOverride(40.f)
					]
				]

				// Content column
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(FMargin(8.f, 2.f))
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FAppStyle::GetFontStyle("BoldFont"))
						.ColorAndOpacity_Lambda([bIsValid]()
						{
							return bIsValid ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f));
						})
					]

					// Subtitle (class name)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(SubtitleText)
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]

					// Badge chips
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						ChipRow
					]
				]
			]
		];
}

void SGorgeousObjectVariableBrowserWindow::OnGetChildren(TSharedPtr<FTreeItem> Item, TArray<TSharedPtr<FTreeItem>>& OutChildren)
{
	OutChildren = Item->Children;
}

void SGorgeousObjectVariableBrowserWindow::OnSelectionChanged(TSharedPtr<FTreeItem> Item, ESelectInfo::Type SelectInfo)
{
	SelectedItem = Item;
	RefreshDetailsPanel();
}

// ════════════════════════════════════════════════════════════════════════════
//  Details panel
// ════════════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SGorgeousObjectVariableBrowserWindow::BuildDetailsPanel()
{
	return DetailsBox.ToSharedRef();
}

void SGorgeousObjectVariableBrowserWindow::RefreshDetailsPanel()
{
	if (!DetailsBox.IsValid())
	{
		return;
	}

	DetailsBox->ClearChildren();

	if (!SelectedItem.IsValid() || !SelectedItem->IsValid())
	{
		DetailsBox->AddSlot()
		.AutoHeight()
		.Padding(8.f, 16.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Select a variable from the tree to view details.")))
			.Font(FAppStyle::GetFontStyle("SmallFont"))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
		return;
	}

	const UGorgeousObjectVariable* Var = SelectedItem->Variable.Get();
	if (!Var)
	{
		return;
	}

	auto AddRow = [this](const FString& Label, const FString& Value)
	{
		DetailsBox->AddSlot()
		.AutoHeight()
		.Padding(12.f, 2.f, 4.f, 2.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 12, 0)
			[
				SNew(SBox)
				.WidthOverride(130.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Label))
					.Font(FAppStyle::GetFontStyle("SmallFont"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.AutoWrapText(true)
			]
		];
	};

	auto AddSectionHeader = [this](const FString& Title, const FLinearColor& AccentColor)
	{
		DetailsBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 10.f, 0.f, 4.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.06f, 0.5f))
			.Padding(FMargin(0.f))
			[
				SNew(SHorizontalBox)

				// Accent bar
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
					.Padding(0.f)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(AccentColor)
					[
						SNew(SBox)
						.WidthOverride(3.f)
						.HeightOverride(22.f)
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(8.f, 4.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Title))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
					.ColorAndOpacity(FSlateColor(AccentColor))
				]
			]
		];
	};

	// ── Identity ───────────────────────────────────────────────────────────
	AddSectionHeader(TEXT("Identity"), FLinearColor(0.20f, 0.60f, 0.90f, 1.f));
	AddRow(TEXT("Display Name"), Var->GetDisplayName().IsEmpty() ? TEXT("(unnamed)") : Var->GetDisplayName());
	AddRow(TEXT("Class"), BuildClassLabel(Var));
	AddRow(TEXT("Identifier"), Var->UniqueIdentifier.ToString());
	AddRow(TEXT("Root"), SelectedItem->RootName.ToString());
	AddRow(TEXT("Parent"), Var->GetParent() ? Var->GetParent()->GetDisplayNameOrFallback() : TEXT("(root)"));
	AddRow(TEXT("Children"), FString::FromInt(Var->VariableRegistry.Num()));
	AddRow(TEXT("Persistent"), Var->bPersistent ? TEXT("Yes") : TEXT("No"));
	AddRow(TEXT("Unique"), Var->bUnique ? TEXT("Yes") : TEXT("No"));

	// ── World / Connection ─────────────────────────────────────────────────
	if (!SelectedItem->WorldLabel.IsEmpty())
	{
		AddSectionHeader(TEXT("World / Connection"), SelectedItem->ConnectionAccent);
		AddRow(TEXT("World"), SelectedItem->WorldLabel);

		if (UWorld* VarWorld = SelectedItem->OwningWorld.Get())
		{
			const ENetMode NetMode = VarWorld->GetNetMode();
			const TCHAR* NetModeStr = TEXT("Standalone");
			switch (NetMode)
			{
			case NM_DedicatedServer: NetModeStr = TEXT("Dedicated Server"); break;
			case NM_ListenServer:   NetModeStr = TEXT("Listen Server"); break;
			case NM_Client:         NetModeStr = TEXT("Client"); break;
			default: break;
			}
			AddRow(TEXT("Net Mode"), NetModeStr);
			AddRow(TEXT("Net Role"), VarWorld->GetAuthGameMode() ? TEXT("Authority") : TEXT("Remote"));
		}
	}

	// ── Value Preview ──────────────────────────────────────────────────────
	const FString ValuePreview = BuildValuePreview(Var);
	if (!ValuePreview.IsEmpty())
	{
		AddSectionHeader(TEXT("Value Preview"), FLinearColor(0.65f, 0.35f, 0.85f, 1.f));
		AddRow(TEXT("Value"), ValuePreview);
	}

	// ── Networking ─────────────────────────────────────────────────────────
	if (Var->bSupportsNetworking)
	{
		AddSectionHeader(TEXT("Networking"), FLinearColor(0.25f, 0.75f, 0.45f, 1.f));

		const TCHAR* ModeStr = TEXT("Unknown");
		switch (Var->ReplicationMode)
		{
		case EGorgeousObjectVariableReplicationMode::EFullAutoReplication: ModeStr = TEXT("Full AutoReplication"); break;
		case EGorgeousObjectVariableReplicationMode::EHybrid:             ModeStr = TEXT("Hybrid"); break;
		case EGorgeousObjectVariableReplicationMode::EManual:             ModeStr = TEXT("Manual"); break;
		default: break;
		}
		AddRow(TEXT("Replication Mode"), ModeStr);
		AddRow(TEXT("Replicates"), Var->bReplicates ? TEXT("Yes") : TEXT("No"));
		AddRow(TEXT("Root Network Stack"), Var->RootNetworkConfig.bExposeThroughRootNetworkStack ? TEXT("Enabled") : TEXT("Disabled"));
		AddRow(TEXT("Shared Network Stack"), Var->bUseSharedNetworkStack ? TEXT("Yes") : TEXT("No"));

		const TCHAR* PolicyStr = TEXT("Unknown");
		switch (Var->RootNetworkConfig.AccessPolicy)
		{
		case EGorgeousObjectVariableAccessPolicy::Everyone:              PolicyStr = TEXT("Everyone"); break;
		case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly: PolicyStr = TEXT("Owning Controller Only"); break;
		case EGorgeousObjectVariableAccessPolicy::Custom:               PolicyStr = TEXT("Custom"); break;
		default: break;
		}
		AddRow(TEXT("Access Policy"), PolicyStr);
		AddRow(TEXT("Replication Channel"), Var->RootNetworkConfig.ReplicationChannel.IsNone() ? TEXT("(default)") : Var->RootNetworkConfig.ReplicationChannel.ToString());

		if (Var->HasAutoReplicationBinding())
		{
			AddRow(TEXT("AR Owner"), Var->GetAutoReplicationOwner() ? Var->GetAutoReplicationOwner()->GetName() : TEXT("(null)"));
			AddRow(TEXT("AR Entry Key"), Var->GetAutoReplicationEntryKey().ToString());
			AddRow(TEXT("AR Replication Index"), FString::FromInt(Var->GetAutoReplicationReplicationIndex()));
		}

		// ── Connection Access Matrix ───────────────────────────────────────
		if (UWorld* VarWorld = SelectedItem->OwningWorld.Get())
		{
			UGorgeousRootNetworkStackSubsystem* StackSub = UGorgeousRootNetworkStackSubsystem::Get(VarWorld);
			if (StackSub)
			{
				AddSectionHeader(TEXT("Connection Access"), FLinearColor(0.20f, 0.60f, 0.90f, 1.f));

				int32 ConnIndex = 0;
				for (auto It = VarWorld->GetPlayerControllerIterator(); It; ++It)
				{
					APlayerController* PC = It->Get();
					if (!PC)
					{
						continue;
					}

					FGorgeousAutoReplicationRPCResponderHandle Handle = FGorgeousAutoReplicationRPCResponderHandle::FromController(PC);
					const FString ConnLabel = Handle.bIsServer
						? TEXT("Server")
						: (!Handle.PlayerDisplayName.IsEmpty()
							? Handle.PlayerDisplayName
							: FString::Printf(TEXT("Client %d"), ConnIndex));

					const bool bCanAccess = StackSub->CanControllerAccess(Var, PC);
					const TArray<FName> Subs = StackSub->GetControllerSubscriptions(PC);

					FString SubsStr;
					for (const FName& Sub : Subs)
					{
						if (!SubsStr.IsEmpty())
						{
							SubsStr += TEXT(", ");
						}
						SubsStr += Sub.ToString();
					}
					if (SubsStr.IsEmpty())
					{
						SubsStr = TEXT("(none)");
					}

					AddRow(ConnLabel, FString::Printf(TEXT("%s | Channels: %s"),
						bCanAccess ? TEXT("Can Access") : TEXT("No Access"), *SubsStr));

					++ConnIndex;
				}

				if (ConnIndex == 0)
				{
					AddRow(TEXT("Connections"), TEXT("No player controllers in world"));
				}
			}
		}
	}
	else
	{
		AddSectionHeader(TEXT("Networking"), FLinearColor(0.35f, 0.35f, 0.35f, 0.85f));
		AddRow(TEXT("Status"), TEXT("Not networking-enabled"));
	}

	// ── Registry Children ──────────────────────────────────────────────────
	if (Var->VariableRegistry.Num() > 0)
	{
		AddSectionHeader(FString::Printf(TEXT("Children (%d)"), Var->VariableRegistry.Num()), FLinearColor(0.85f, 0.55f, 0.15f, 1.f));
		const int32 MaxDisplay = FMath::Min(Var->VariableRegistry.Num(), 20);
		int32 Shown = 0;
		for (const auto& [ChildKey, ChildPtr] : Var->VariableRegistry)
		{
			if (Shown >= MaxDisplay)
			{
				break;
			}
			const UGorgeousObjectVariable* Child = ChildPtr.Get();
			if (Child)
			{
				AddRow(ChildKey.ToString(),
					FString::Printf(TEXT("%s (%s)"),
						*(!Child->GetDisplayName().IsEmpty() ? Child->GetDisplayName() : Child->GetName()),
						*BuildClassLabel(Child)));
			}
			++Shown;
		}
		if (Var->VariableRegistry.Num() > MaxDisplay)
		{
			AddRow(TEXT("..."), FString::Printf(TEXT("and %d more"), Var->VariableRegistry.Num() - MaxDisplay));
		}
	}

	// ── Property Manipulator ───────────────────────────────────────────────
	BuildManipulatorSection(const_cast<UGorgeousObjectVariable*>(Var));
}

// ════════════════════════════════════════════════════════════════════════════
//  Property Manipulator
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousObjectVariableBrowserWindow::BuildManipulatorSection(UGorgeousObjectVariable* Variable)
{
	if (!Variable || !DetailsBox.IsValid())
	{
		return;
	}

	const FLinearColor ManipulatorAccent(0.85f, 0.35f, 0.20f, 1.f);

	// Section header with accent bar
	DetailsBox->AddSlot()
	.AutoHeight()
	.Padding(0.f, 14.f, 0.f, 6.f)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.06f, 0.5f))
		.Padding(FMargin(0.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBorder)
				.Padding(0.f)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(ManipulatorAccent)
				[
					SNew(SBox)
					.WidthOverride(3.f)
					.HeightOverride(22.f)
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(8.f, 4.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Property Manipulator")))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
				.ColorAndOpacity(FSlateColor(ManipulatorAccent))
			]
		]
	];

	auto MakeManipulatorRow = [this](const FString& Label, TSharedRef<SWidget> Control)
	{
		DetailsBox->AddSlot()
		.AutoHeight()
		.Padding(12.f, 3.f, 4.f, 3.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 12, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(130.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Label))
					.Font(FAppStyle::GetFontStyle("SmallFont"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				Control
			]
		];
	};

	// Display Name
	MakeManipulatorRow(TEXT("Display Name"),
		SNew(SEditableTextBox)
		.Text(FText::FromString(Variable->DisplayName))
		.Font(FAppStyle::GetFontStyle("SmallFont"))
		.OnTextCommitted(this, &SGorgeousObjectVariableBrowserWindow::OnDisplayNameCommitted)
	);

	// Persistent
	MakeManipulatorRow(TEXT("Persistent"),
		SNew(SCheckBox)
		.IsChecked(Variable->bPersistent ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		.OnCheckStateChanged(this, &SGorgeousObjectVariableBrowserWindow::OnPersistentChanged)
	);

	// Replicates (only if networking is supported)
	if (Variable->bSupportsNetworking)
	{
		MakeManipulatorRow(TEXT("Replicates"),
			SNew(SCheckBox)
			.IsChecked(Variable->bReplicates ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged(this, &SGorgeousObjectVariableBrowserWindow::OnReplicatesChanged)
		);

		MakeManipulatorRow(TEXT("Shared Net Stack"),
			SNew(SCheckBox)
			.IsChecked(Variable->bUseSharedNetworkStack ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged(this, &SGorgeousObjectVariableBrowserWindow::OnSharedNetworkStackChanged)
		);
	}

	// Value (generic text import/export for any subclass)
	if (const FProperty* ValueProp = FindFProperty<FProperty>(Variable->GetClass(), TEXT("Value")))
	{
		FString ValueStr;
		ValueProp->ExportTextItem_Direct(ValueStr, ValueProp->ContainerPtrToValuePtr<void>(Variable), nullptr, Variable, PPF_None);

		MakeManipulatorRow(TEXT("Value"),
			SNew(SEditableTextBox)
			.Text(FText::FromString(ValueStr))
			.Font(FAppStyle::GetFontStyle("SmallFont"))
			.OnTextCommitted(this, &SGorgeousObjectVariableBrowserWindow::OnValueTextCommitted)
		);
	}
	
	// Variable registry display (comma-separated list of child keys)
	if (Variable->VariableRegistry.Num() > 0)
	{
		FString RegistryStr;	
		for (const auto& [ChildKey, ChildPtr] : Variable->VariableRegistry)
		{
			if (!RegistryStr.IsEmpty())
			{
				RegistryStr += TEXT(", ");
			}
			RegistryStr += ChildKey.ToString();
		}

		MakeManipulatorRow(TEXT("Variable Registry"),
			SNew(STextBlock)
			.Text(FText::FromString(RegistryStr))
			.Font(FAppStyle::GetFontStyle("SmallFont"))
			.AutoWrapText(true)
		);
	}
	
	// Manipulator for every FProperty that has the BlueprintVisible flag, so that blueprint added variables can be edited here
	for (TFieldIterator<FProperty> PropIt(Variable->GetClass()); PropIt; ++PropIt)
	{
		// A list of properties to exclude since we already have dedicated manipulators for them, and some that are not safe to edit via text import/export
		TArray<FString> PropExcludeList = {
			TEXT("DisplayName"),
			TEXT("bPersistent"),
			TEXT("bReplicates"),
			TEXT("bUseSharedNetworkStack"),
			TEXT("Value"),
			TEXT("UniqueIdentifier"),
			TEXT("VariableRegistry"),
			TEXT("Parent"),
			TEXT("PinConfiguration"),
			TEXT("bSupportsNetworking"),
			TEXT("ReplicationMode")
		};
		
		if (PropExcludeList.Contains(PropIt->GetName())) { continue; }

		if (FProperty* Prop = *PropIt; Prop->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			FString PropValueStr;
			Prop->ExportTextItem_Direct(PropValueStr, Prop->ContainerPtrToValuePtr<void>(Variable), nullptr, Variable, PPF_None);

			MakeManipulatorRow(Prop->GetName(),
				SNew(SEditableTextBox)
				.Text(FText::FromString(PropValueStr))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.OnTextCommitted_Lambda([this, Variable, Prop](const FText& NewText, ETextCommit::Type CommitType)
				{
					if (!Variable || !Prop)
					{
						return;
					}

					void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Variable);
					const FString NewValueStr = NewText.ToString();

					// Use ImportText to parse the string back into the property
					Prop->ImportText_Direct(*NewValueStr, ValuePtr, Variable, PPF_None);

					RefreshDetailsPanel();
				})
			);
		}
	}
}

void SGorgeousObjectVariableBrowserWindow::OnDisplayNameCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (!SelectedItem.IsValid() || !SelectedItem->IsValid())
	{
		return;
	}

	UGorgeousObjectVariable* Var = SelectedItem->Variable.Get();
	if (Var)
	{
		Var->DisplayName = NewText.ToString();
		RefreshTree();
		RefreshDetailsPanel();
	}
}

void SGorgeousObjectVariableBrowserWindow::OnPersistentChanged(ECheckBoxState NewState)
{
	if (!SelectedItem.IsValid() || !SelectedItem->IsValid())
	{
		return;
	}

	UGorgeousObjectVariable* Var = SelectedItem->Variable.Get();
	if (Var)
	{
		Var->bPersistent = (NewState == ECheckBoxState::Checked);
	}
}

void SGorgeousObjectVariableBrowserWindow::OnReplicatesChanged(ECheckBoxState NewState)
{
	if (!SelectedItem.IsValid() || !SelectedItem->IsValid())
	{
		return;
	}

	UGorgeousObjectVariable* Var = SelectedItem->Variable.Get();
	if (Var)
	{
		Var->bReplicates = (NewState == ECheckBoxState::Checked);
		RefreshDetailsPanel();
	}
}

void SGorgeousObjectVariableBrowserWindow::OnSharedNetworkStackChanged(ECheckBoxState NewState)
{
	if (!SelectedItem.IsValid() || !SelectedItem->IsValid())
	{
		return;
	}

	UGorgeousObjectVariable* Var = SelectedItem->Variable.Get();
	if (Var)
	{
		Var->bUseSharedNetworkStack = (NewState == ECheckBoxState::Checked);
		RefreshDetailsPanel();
	}
}

void SGorgeousObjectVariableBrowserWindow::OnValueTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (!SelectedItem.IsValid() || !SelectedItem->IsValid())
	{
		return;
	}

	UGorgeousObjectVariable* Var = SelectedItem->Variable.Get();
	if (!Var)
	{
		return;
	}

	FProperty* ValueProp = FindFProperty<FProperty>(Var->GetClass(), TEXT("Value"));
	if (!ValueProp)
	{
		return;
	}

	void* ValuePtr = ValueProp->ContainerPtrToValuePtr<void>(Var);
	const FString NewValueStr = NewText.ToString();

	// Use ImportText to parse the string back into the property
	ValueProp->ImportText_Direct(*NewValueStr, ValuePtr, Var, PPF_None);

	RefreshDetailsPanel();
}

// ════════════════════════════════════════════════════════════════════════════
//  Helpers
// ════════════════════════════════════════════════════════════════════════════

FText SGorgeousObjectVariableBrowserWindow::GetFilterText() const
{
	return FText::FromString(FilterText);
}

void SGorgeousObjectVariableBrowserWindow::OnFilterTextChanged(const FText& InFilterText)
{
	FilterText = InFilterText.ToString();
	RefreshTree();
}

void SGorgeousObjectVariableBrowserWindow::OnRootFilterChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	RootFilter = NewSelection.IsValid() ? *NewSelection : NAME_None;
	RefreshTree();
}

FReply SGorgeousObjectVariableBrowserWindow::OnRefreshClicked()
{
	RefreshTree();
	return FReply::Handled();
}

FReply SGorgeousObjectVariableBrowserWindow::OnExpandAllClicked()
{
	if (TreeView.IsValid())
	{
		for (const TSharedPtr<FTreeItem>& Item : RootItems)
		{
			TreeView->SetItemExpansion(Item, true);
			for (const TSharedPtr<FTreeItem>& Child : Item->Children)
			{
				TreeView->SetItemExpansion(Child, true);
			}
		}
	}
	return FReply::Handled();
}

FReply SGorgeousObjectVariableBrowserWindow::OnCollapseAllClicked()
{
	if (TreeView.IsValid())
	{
		for (const TSharedPtr<FTreeItem>& Item : RootItems)
		{
			TreeView->SetItemExpansion(Item, false);
		}
	}
	return FReply::Handled();
}

bool SGorgeousObjectVariableBrowserWindow::PassesFilter(const FTreeItem& Item) const
{
	if (FilterText.IsEmpty())
	{
		return true;
	}

	if (Item.DisplayLabel.Contains(FilterText, ESearchCase::IgnoreCase))
	{
		return true;
	}

	if (const UGorgeousObjectVariable* Var = Item.Variable.Get())
	{
		if (Var->GetName().Contains(FilterText, ESearchCase::IgnoreCase))
		{
			return true;
		}
		if (Var->GetClass()->GetName().Contains(FilterText, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	// Check children recursively
	for (const TSharedPtr<FTreeItem>& Child : Item.Children)
	{
		if (PassesFilter(*Child))
		{
			return true;
		}
	}

	return false;
}

FString SGorgeousObjectVariableBrowserWindow::BuildValuePreview(const UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return TEXT("");
	}

	// Try to extract the "Value" property as a preview
	if (const FProperty* ValueProp = FindFProperty<FProperty>(Variable->GetClass(), TEXT("Value")))
	{
		FString ValueStr;
		ValueProp->ExportTextItem_Direct(ValueStr, ValueProp->ContainerPtrToValuePtr<void>(Variable), nullptr, const_cast<UGorgeousObjectVariable*>(Variable), PPF_None);
		if (!ValueStr.IsEmpty())
		{
			// Truncate very long values
			if (ValueStr.Len() > 200)
			{
				ValueStr = ValueStr.Left(197) + TEXT("...");
			}
			return ValueStr;
		}
	}

	return TEXT("");
}

FString SGorgeousObjectVariableBrowserWindow::BuildClassLabel(const UGorgeousObjectVariable* Variable)
{
	if (!Variable || !Variable->GetClass())
	{
		return TEXT("Unknown");
	}

	FString ClassName = Variable->GetClass()->GetName();

	// Strip common prefix for readability
	if (ClassName.StartsWith(TEXT("Gorgeous")))
	{
		// Keep the full name for clarity
	}

	return ClassName;
}

FString SGorgeousObjectVariableBrowserWindow::BuildReplicationLabel(const UGorgeousObjectVariable* Variable)
{
	if (!Variable || !Variable->bSupportsNetworking)
	{
		return TEXT("");
	}

	if (Variable->bReplicates)
	{
		return TEXT("Synced");
	}

	return TEXT("Not synced");
}

// ── World / Connection helpers ───────────────────────────────────────────

void SGorgeousObjectVariableBrowserWindow::OnWorldFilterChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		WorldFilter = *NewSelection;

		// "All Worlds" resets the filter
		if (WorldFilter == TEXT("All Worlds"))
		{
			WorldFilter.Empty();
		}
	}
	else
	{
		WorldFilter.Empty();
	}

	if (!bIsRefreshing)
	{
		RefreshTree();
	}
}

void SGorgeousObjectVariableBrowserWindow::RefreshWorldConnections()
{
	AvailableConnections.Reset();

	if (!GEngine)
	{
		return;
	}

	int32 ClientIdx = 0;
	int32 StandaloneIdx = 0;
	int32 ServerIdx = 0;

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (!World)
		{
			continue;
		}

		// Only inspect PIE or Game worlds
		if (Context.WorldType != EWorldType::PIE && Context.WorldType != EWorldType::Game)
		{
			continue;
		}

		FWorldConnection Connection;
		Connection.World = World;

		// Build a unique suffix from the PIE instance when available
		const int32 PIEInst = Context.PIEInstance;
		const bool bHasPIE = (PIEInst != INDEX_NONE && PIEInst >= 0);

		const ENetMode NetMode = World->GetNetMode();

		switch (NetMode)
		{
		case NM_DedicatedServer:
			Connection.Label = bHasPIE
				? FString::Printf(TEXT("Dedicated Server (PIE %d)"), PIEInst)
				: TEXT("Dedicated Server");
			Connection.bIsServer = true;
			Connection.AccentColor = GetConnectionColor(ServerIdx, true);
			ServerIdx++;
			break;

		case NM_ListenServer:
			Connection.Label = bHasPIE
				? FString::Printf(TEXT("Listen Server (PIE %d)"), PIEInst)
				: TEXT("Listen Server");
			Connection.bIsServer = true;
			Connection.AccentColor = GetConnectionColor(ServerIdx, true);
			ServerIdx++;
			break;

		case NM_Client:
			Connection.Label = bHasPIE
				? FString::Printf(TEXT("Client %d (PIE %d)"), ClientIdx, PIEInst)
				: FString::Printf(TEXT("Client %d"), ClientIdx);
			Connection.bIsServer = false;
			Connection.ClientIndex = ClientIdx;
			Connection.AccentColor = GetConnectionColor(ClientIdx, false);
			ClientIdx++;
			break;

		case NM_Standalone:
		default:
			if (bHasPIE)
			{
				Connection.Label = FString::Printf(TEXT("Standalone (PIE %d)"), PIEInst);
			}
			else if (StandaloneIdx > 0)
			{
				Connection.Label = FString::Printf(TEXT("Standalone %d"), StandaloneIdx + 1);
			}
			else
			{
				Connection.Label = TEXT("Standalone");
			}
			Connection.bIsServer = false;
			Connection.AccentColor = GetConnectionColor(StandaloneIdx, false);
			StandaloneIdx++;
			break;
		}

		AvailableConnections.Add(MoveTemp(Connection));
	}
}

SGorgeousObjectVariableBrowserWindow::FWorldConnection SGorgeousObjectVariableBrowserWindow::FindConnectionForWorld(UWorld* World) const
{
	if (!World)
	{
		return FWorldConnection();
	}

	for (const FWorldConnection& Conn : AvailableConnections)
	{
		if (Conn.World.Get() == World)
		{
			return Conn;
		}
	}

	return FWorldConnection();
}

FLinearColor SGorgeousObjectVariableBrowserWindow::GetConnectionColor(int32 Index, bool bIsServer)
{
	if (bIsServer)
	{
		// Server palette, cool blue
		return FLinearColor(0.20f, 0.60f, 0.90f, 0.85f);
	}

	// Client palette, warm, cycling
	static const FLinearColor ClientColors[] =
	{
		FLinearColor(0.25f, 0.78f, 0.45f, 0.85f),  // green
		FLinearColor(0.90f, 0.55f, 0.15f, 0.85f),  // orange
		FLinearColor(0.65f, 0.35f, 0.85f, 0.85f),  // purple
		FLinearColor(0.85f, 0.25f, 0.35f, 0.85f),  // red
		FLinearColor(0.20f, 0.75f, 0.85f, 0.85f),  // cyan
		FLinearColor(0.85f, 0.75f, 0.20f, 0.85f),  // gold
	};

	constexpr int32 NumColors = UE_ARRAY_COUNT(ClientColors);
	return ClientColors[FMath::Abs(Index) % NumColors];
}
