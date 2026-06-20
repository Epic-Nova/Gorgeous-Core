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

#include "InsightMatrix/Slate/SGorgeousNetworkTrafficInspectorWindow.h"

#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "InsightMatrix/GorgeousAutoReplicationStatsCollector.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniLineChart.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

// ════════════════════════════════════════════════════════════════════════════
//  Static state
// ════════════════════════════════════════════════════════════════════════════

TWeakPtr<SWindow> SGorgeousNetworkTrafficInspectorWindow::WindowWeak;

// ════════════════════════════════════════════════════════════════════════════
//  Open / Close / IsOpen
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousNetworkTrafficInspectorWindow::Open()
{
	if (TSharedPtr<SWindow> Existing = WindowWeak.Pin())
	{
		Existing->BringToFront();
		return;
	}

	// Enable the stats collector while the window is open
	FGorgeousAutoReplicationStatsCollector::SetEnabled(true);

	TSharedRef<SGorgeousNetworkTrafficInspectorWindow> Content = SNew(SGorgeousNetworkTrafficInspectorWindow);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("Network Traffic Inspector")))
		.ClientSize(FVector2D(1050, 700))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		[
			Content
		];

	WindowWeak = Window;
	FSlateApplication::Get().AddWindow(Window);
}

void SGorgeousNetworkTrafficInspectorWindow::Close()
{
	if (TSharedPtr<SWindow> Window = WindowWeak.Pin())
	{
		Window->RequestDestroyWindow();
	}
	WindowWeak.Reset();

	// Disable stats collection when the window closes
	FGorgeousAutoReplicationStatsCollector::SetEnabled(false);
}

bool SGorgeousNetworkTrafficInspectorWindow::IsOpen()
{
	return WindowWeak.IsValid();
}

// ════════════════════════════════════════════════════════════════════════════
//  Construct
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousNetworkTrafficInspectorWindow::Construct(const FArguments& InArgs)
{
	// Initialize rolling history with zeros
	BandwidthHistory.SetNumZeroed(GraphHistorySize);
	SyncCountHistory.SetNumZeroed(GraphHistorySize);
	RPCCountHistory.SetNumZeroed(GraphHistorySize);

	// Seed previous counters from current snapshot so the first delta is meaningful
	const FGorgeousAutoReplicationStatsSnapshot InitSnap = FGorgeousAutoReplicationStatsCollector::Get().GetCurrentStats();
	PrevBytesSent = InitSnap.TotalBytesSent;
	PrevBytesReceived = InitSnap.TotalBytesReceived;
	PrevPropertySyncs = InitSnap.TotalPropertySyncs;
	PrevRPCsSent = InitSnap.TotalRPCsSent;
	PrevRPCsReceived = InitSnap.TotalRPCsReceived;

	DetailsBox = SNew(SVerticalBox);

	ChildSlot
	[
		SNew(SVerticalBox)

		// ── Graph banner ───────────────────────────────────────────────────
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.035f, 0.035f, 0.04f, 0.95f))
			.Padding(FMargin(8.f, 6.f))
			[
				SNew(SVerticalBox)

				// Title row + capture interval control
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Bandwidth (KB/s)")))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
						.ColorAndOpacity(FLinearColor(0.3f, 0.75f, 0.5f, 1.f))
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					.Padding(12.f, 0.f, 0.f, 0.f)
					[
						SAssignNew(GraphStatusLabel, STextBlock)
						.Text(FText::FromString(TEXT("Waiting for data...")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(8.f, 0.f, 4.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Capture:")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(60.f)
						[
							SNew(SSpinBox<double>)
							.MinValue(0.25)
							.MaxValue(10.0)
							.Delta(0.25)
							.Value(CaptureIntervalSec)
							.OnValueChanged_Lambda([this](double NewValue) { CaptureIntervalSec = NewValue; })
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4.f, 0.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("s")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
				]

				// The line chart
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(120.f)
					[
						SAssignNew(BandwidthChart, SGorgeousInsightMiniLineChart)
					]
				]
			]
		]

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

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Replicating Variables")))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
					.ColorAndOpacity(FLinearColor(0.3f, 0.75f, 0.5f, 1.f))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNullWidget::NullWidget
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
					.ContentPadding(FMargin(10.f, 4.f))
					.OnClicked(this, &SGorgeousNetworkTrafficInspectorWindow::OnRefreshClicked)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Refresh")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]
			]
		]

		// ── Splitter: List + Details ───────────────────────────────────────
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// Left: stream list
			+ SSplitter::Slot()
			.Value(0.45f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.95f))
				.Padding(2)
				[
					SAssignNew(ListView, SListView<TSharedPtr<FStreamItem>>)
					.ListItemsSource(&StreamItems)
					.OnGenerateRow(this, &SGorgeousNetworkTrafficInspectorWindow::OnGenerateRow)
					.OnSelectionChanged(this, &SGorgeousNetworkTrafficInspectorWindow::OnSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
			]

			// Right: details
			+ SSplitter::Slot()
			.Value(0.55f)
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

	RefreshList();

	// Push initial flat-line into the chart
	if (BandwidthChart.IsValid())
	{
		BandwidthChart->SetValues(BandwidthHistory,
			FText::FromString(TEXT("Bandwidth")),
			FText::FromString(TEXT("KB/s over time")));
	}
}

// ════════════════════════════════════════════════════════════════════════════
//  Tick, capture rolling graph samples
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousNetworkTrafficInspectorWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if ((InCurrentTime - LastCaptureTime) >= CaptureIntervalSec)
	{
		LastCaptureTime = InCurrentTime;
		CaptureGraphSample();
	}
}

void SGorgeousNetworkTrafficInspectorWindow::CaptureGraphSample()
{
	const FGorgeousAutoReplicationStatsSnapshot Snap = FGorgeousAutoReplicationStatsCollector::Get().GetCurrentStats();

	// Compute deltas since last capture
	const int64 DeltaBytesSent = Snap.TotalBytesSent - PrevBytesSent;
	const int64 DeltaBytesReceived = Snap.TotalBytesReceived - PrevBytesReceived;
	const int32 DeltaSyncs = Snap.TotalPropertySyncs - PrevPropertySyncs;
	const int32 DeltaRPCs = (Snap.TotalRPCsSent - PrevRPCsSent) + (Snap.TotalRPCsReceived - PrevRPCsReceived);

	// Store current as previous for next delta
	PrevBytesSent = Snap.TotalBytesSent;
	PrevBytesReceived = Snap.TotalBytesReceived;
	PrevPropertySyncs = Snap.TotalPropertySyncs;
	PrevRPCsSent = Snap.TotalRPCsSent;
	PrevRPCsReceived = Snap.TotalRPCsReceived;

	// Bandwidth in KB/s for this interval
	const double BandwidthKBps = static_cast<double>(DeltaBytesSent + DeltaBytesReceived) / (1024.0 * CaptureIntervalSec);

	// Shift history left (promote current as the one N seconds ago)
	if (BandwidthHistory.Num() >= GraphHistorySize)
	{
		BandwidthHistory.RemoveAt(0, 1, EAllowShrinking::No);
		SyncCountHistory.RemoveAt(0, 1, EAllowShrinking::No);
		RPCCountHistory.RemoveAt(0, 1, EAllowShrinking::No);
	}

	BandwidthHistory.Add(BandwidthKBps);
	SyncCountHistory.Add(static_cast<double>(DeltaSyncs));
	RPCCountHistory.Add(static_cast<double>(DeltaRPCs));

	// Update the line chart
	if (BandwidthChart.IsValid())
	{
		BandwidthChart->SetValues(BandwidthHistory,
			FText::FromString(TEXT("Bandwidth")),
			FText::FromString(FString::Printf(TEXT("%.2f KB/s | %d syncs | %d RPCs this interval"),
				BandwidthKBps, DeltaSyncs, DeltaRPCs)));
	}

	// Update status label
	if (GraphStatusLabel.IsValid())
	{
		const int32 SampleCount = BandwidthHistory.Num();
		const double WindowSec = SampleCount * CaptureIntervalSec;
		GraphStatusLabel->SetText(FText::FromString(FString::Printf(
			TEXT("%.2f KB/s | %d samples (%.0fs window)"),
			BandwidthKBps, SampleCount, WindowSec)));
	}
}

// ════════════════════════════════════════════════════════════════════════════
//  List population
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousNetworkTrafficInspectorWindow::RefreshList()
{
	StreamItems.Reset();

	const TArray<UGorgeousObjectVariable*> AllVariables = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(NAME_None);

	for (UGorgeousObjectVariable* Var : AllVariables)
	{
		if (!IsValid(Var) || !Var->bSupportsNetworking)
		{
			continue;
		}

		TSharedPtr<FStreamItem> Item = MakeShared<FStreamItem>();
		Item->Variable = Var;
		Item->DisplayLabel = !Var->GetDisplayName().IsEmpty() ? Var->GetDisplayName() : Var->GetName();

		const FGorgeousAutoReplicationStreamConfig& Config = Var->AutoReplicationConfig;

		switch (Config.Backend)
		{
		case EGorgeousAutoReplicationBackend::Native:           Item->BackendLabel = TEXT("Native UE"); break;
		case EGorgeousAutoReplicationBackend::Iris:             Item->BackendLabel = TEXT("Iris"); break;
		case EGorgeousAutoReplicationBackend::ReplicationGraph: Item->BackendLabel = TEXT("Rep Graph"); break;
		default:                                                 Item->BackendLabel = TEXT("Unknown"); break;
		}

		Item->UpdateFrequency = Config.GetEffectiveUpdateFrequency();
		Item->BandwidthBudgetKB = Config.BandwidthBudgetKB;
		Item->Priority = Config.Priority;
		Item->bSupportsMulticast = Config.bSupportsMulticast;
		Item->bRespectsAccessPolicy = Config.bRespectAccessPolicy;

		Item->ChannelName = Var->RootNetworkConfig.ReplicationChannel.IsNone()
			? TEXT("(default)")
			: Var->RootNetworkConfig.ReplicationChannel.ToString();

		switch (Var->RootNetworkConfig.AccessPolicy)
		{
		case EGorgeousObjectVariableAccessPolicy::Everyone:              Item->AccessPolicyLabel = TEXT("Everyone"); break;
		case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly: Item->AccessPolicyLabel = TEXT("Owner Only"); break;
		case EGorgeousObjectVariableAccessPolicy::Custom:               Item->AccessPolicyLabel = TEXT("Custom"); break;
		default:                                                         Item->AccessPolicyLabel = TEXT("Unknown"); break;
		}

		Item->bHasARBinding = Var->HasAutoReplicationBinding();
		if (Item->bHasARBinding)
		{
			Item->AREntryKey = Var->GetAutoReplicationEntryKey().ToString();
			Item->AROwnerLabel = Var->GetAutoReplicationOwner()
				? Var->GetAutoReplicationOwner()->GetName()
				: TEXT("(null)");
		}

		StreamItems.Add(Item);
	}

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

// ════════════════════════════════════════════════════════════════════════════
//  Row generation
// ════════════════════════════════════════════════════════════════════════════

TSharedRef<ITableRow> SGorgeousNetworkTrafficInspectorWindow::OnGenerateRow(TSharedPtr<FStreamItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const bool bReplicating = Item->Variable.IsValid() && Item->Variable->IsReplicationActive();

	// Accent: replicating = vibrant green, network-capable-only = amber
	const FLinearColor AccentColor = bReplicating
		? FLinearColor(0.25f, 0.80f, 0.45f, 1.f)
		: FLinearColor(0.80f, 0.60f, 0.20f, 1.f);

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

	TSharedRef<SHorizontalBox> ChipRow = SNew(SHorizontalBox);

	// Backend chip
	ChipRow->AddSlot()
		.AutoWidth()
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			MakeChip(FText::FromString(Item->BackendLabel),
				FLinearColor(0.25f, 0.50f, 0.80f, 1.f))
		];

	// Update frequency chip
	ChipRow->AddSlot()
		.AutoWidth()
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			MakeChip(FText::FromString(FString::Printf(TEXT("%.0fHz"), Item->UpdateFrequency)),
				FLinearColor(0.50f, 0.35f, 0.70f, 1.f))
		];

	// Bandwidth chip
	ChipRow->AddSlot()
		.AutoWidth()
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			MakeChip(FText::FromString(FString::Printf(TEXT("%.1fKB"), Item->BandwidthBudgetKB)),
				FLinearColor(0.70f, 0.40f, 0.25f, 1.f))
		];

	// State chip
	ChipRow->AddSlot()
		.AutoWidth()
		[
			MakeChip(
				FText::FromString(bReplicating ? TEXT("Active") : TEXT("Idle")),
				bReplicating
					? FLinearColor(0.20f, 0.60f, 0.35f, 1.f)
					: FLinearColor(0.45f, 0.45f, 0.45f, 1.f))
		];

	return SNew(STableRow<TSharedPtr<FStreamItem>>, OwnerTable)
		[
			SNew(SBorder)
			.Padding(FMargin(6.f))
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor_Lambda([this, Item]()
			{
				const bool bSelected = ListView.IsValid() && Item.IsValid() && ListView->IsItemSelected(Item);
				return bSelected
					? FLinearColor(0.10f, 0.45f, 0.85f, 0.35f)
					: FLinearColor(0.06f, 0.06f, 0.06f, 0.35f);
			})
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
						.WidthOverride(4.f)
						.HeightOverride(44.f)
					]
				]

				// Content
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(FMargin(8.f, 2.f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Item->DisplayLabel))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
					]

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

void SGorgeousNetworkTrafficInspectorWindow::OnSelectionChanged(TSharedPtr<FStreamItem> Item, ESelectInfo::Type SelectInfo)
{
	SelectedItem = Item;
	RefreshDetailsPanel();
}

// ════════════════════════════════════════════════════════════════════════════
//  Details panel
// ════════════════════════════════════════════════════════════════════════════

void SGorgeousNetworkTrafficInspectorWindow::RefreshDetailsPanel()
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
			.Text(FText::FromString(TEXT("Select a variable to view stream details.")))
			.Font(FAppStyle::GetFontStyle("SmallFont"))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
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
				.WidthOverride(160.f)
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

	const FStreamItem& Item = *SelectedItem;
	const UGorgeousObjectVariable* Var = Item.Variable.Get();

	// ── Stream Configuration ─────────────────────────────────────────────
	AddSectionHeader(TEXT("Stream Configuration"), FLinearColor(0.30f, 0.75f, 0.50f, 1.f));
	AddRow(TEXT("Variable"), Item.DisplayLabel);
	AddRow(TEXT("Backend"), Item.BackendLabel);
	AddRow(TEXT("Update Frequency"), FString::Printf(TEXT("%.1f Hz"), Item.UpdateFrequency));
	AddRow(TEXT("Bandwidth Budget"), FString::Printf(TEXT("%.2f KB"), Item.BandwidthBudgetKB));
	AddRow(TEXT("Priority"), FString::Printf(TEXT("%d"), Item.Priority));
	AddRow(TEXT("Supports Multicast"), Item.bSupportsMulticast ? TEXT("Yes") : TEXT("No"));
	AddRow(TEXT("Respects Access Policy"), Item.bRespectsAccessPolicy ? TEXT("Yes") : TEXT("No"));

	if (Var)
	{
		const TCHAR* ModeStr = TEXT("Unknown");
		switch (Var->ReplicationMode)
		{
		case EGorgeousObjectVariableReplicationMode::EFullAutoReplication: ModeStr = TEXT("Full AutoReplication"); break;
		case EGorgeousObjectVariableReplicationMode::EHybrid:             ModeStr = TEXT("Hybrid"); break;
		case EGorgeousObjectVariableReplicationMode::EManual:             ModeStr = TEXT("Manual"); break;
		default: break;
		}
		AddRow(TEXT("Replication Mode"), ModeStr);
		AddRow(TEXT("Replicating"), Var->IsReplicationActive() ? TEXT("Yes") : TEXT("No"));
	}

	// ── Root Network Stack ───────────────────────────────────────────────
	AddSectionHeader(TEXT("Root Network Stack"), FLinearColor(0.50f, 0.35f, 0.80f, 1.f));
	AddRow(TEXT("Channel"), Item.ChannelName);
	AddRow(TEXT("Access Policy"), Item.AccessPolicyLabel);

	if (Var)
	{
		AddRow(TEXT("Exposed via Stack"), Var->RootNetworkConfig.bExposeThroughRootNetworkStack ? TEXT("Yes") : TEXT("No"));
		AddRow(TEXT("Shared Net Stack"), Var->bUseSharedNetworkStack ? TEXT("Yes") : TEXT("No"));
	}

	// ── AR Binding ───────────────────────────────────────────────────────
	if (Item.bHasARBinding)
	{
		AddSectionHeader(TEXT("AutoReplication Binding"), FLinearColor(0.80f, 0.55f, 0.20f, 1.f));
		AddRow(TEXT("Entry Key"), Item.AREntryKey);
		AddRow(TEXT("Owner"), Item.AROwnerLabel);
		if (Var)
		{
			AddRow(TEXT("Replication Index"), FString::Printf(TEXT("%d"), Var->GetAutoReplicationReplicationIndex()));
		}
	}

	// ── Connection Channel Subscriptions ─────────────────────────────────
	if (Var)
	{
		// Find best game world for connection queries
		UWorld* World = nullptr;
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* W = Context.World();
				if (W && (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game))
				{
					World = W;
					break;
				}
			}
		}

		if (World)
		{
			UGorgeousRootNetworkStackSubsystem* StackSub = UGorgeousRootNetworkStackSubsystem::Get(World);
			if (StackSub)
			{
				AddSectionHeader(TEXT("Connection Access"), FLinearColor(0.20f, 0.60f, 0.90f, 1.f));

				int32 ConnectionIndex = 0;
				for (auto It = World->GetPlayerControllerIterator(); It; ++It)
				{
					APlayerController* PC = It->Get();
					if (!PC)
					{
						continue;
					}

					FGorgeousAutoReplicationRPCResponderHandle Handle = FGorgeousAutoReplicationRPCResponderHandle::FromController(PC);
					const FString ConnLabel = Handle.bIsServer
						? TEXT("Server")
						: FString::Printf(TEXT("Client %d"), ConnectionIndex);

					const bool bCanAccess = StackSub->CanControllerAccess(Var, PC);
					const TArray<FName> Subs = StackSub->GetControllerSubscriptions(PC);

					AddRow(ConnLabel, FString::Printf(TEXT("Access: %s | Subs: %d"),
						bCanAccess ? TEXT("Yes") : TEXT("No"), Subs.Num()));

					++ConnectionIndex;
				}

				if (ConnectionIndex == 0)
				{
					AddRow(TEXT("(no controllers)"), TEXT("No player controllers in world"));
				}
			}
		}
	}
}

FReply SGorgeousNetworkTrafficInspectorWindow::OnRefreshClicked()
{
	RefreshList();
	RefreshDetailsPanel();
	return FReply::Handled();
}
