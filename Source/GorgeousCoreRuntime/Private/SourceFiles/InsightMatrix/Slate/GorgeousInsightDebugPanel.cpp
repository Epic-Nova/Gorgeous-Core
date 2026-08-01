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

#include "InsightMatrix/Slate/GorgeousInsightDebugPanel.h"
#include "InsightMatrix/GorgeousInsightChartRegistry.h"
#include "InsightMatrix/GorgeousInsightChartWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Slate/SObjectWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniBarChart.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniFlowChart.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniHeatmap.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniHistogram.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniLineChart.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniPieChart.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniScatterPlot.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniTable.h"
#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniTimeline.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"
#include "UObject/SoftObjectPath.h"
#if WITH_EDITOR
#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "IDesktopPlatform.h"
#include "Editor.h"
#endif

#if WITH_EDITOR
#include "PropertyCustomizationHelpers.h"
#endif

namespace
{
	const FName ColumnStatName(TEXT("StatName"));
	const FName ColumnStatValue(TEXT("StatValue"));
	const FName ColumnStatCategory(TEXT("StatCategory"));
	const FName ColumnStatThresholds(TEXT("StatThresholds"));
	const FName ColumnTestName(TEXT("TestName"));
	const FName ColumnTestCategory(TEXT("TestCategory"));
	const FName ColumnTestInputs(TEXT("TestInputs"));
	const FName ColumnTestResult(TEXT("TestResult"));
	const FName ColumnTestBaseline(TEXT("TestBaseline"));
	const FName ColumnTestRun(TEXT("TestRun"));

	FText FormatSeconds(double Seconds)
	{
		return FText::AsNumber(Seconds, &FNumberFormattingOptions()
			.SetMaximumFractionalDigits(6)
			.SetMinimumFractionalDigits(2));
	}

	FText FormatBytes(double Bytes)
	{
		const double KB = 1024.0;
		const double MB = KB * 1024.0;
		const double GB = MB * 1024.0;

		double Value = Bytes;
		FText Suffix = NSLOCTEXT("GorgeousInsightDebugPanel", "Bytes", "B");

		if (FMath::Abs(Value) >= GB)
		{
			Value /= GB;
			Suffix = NSLOCTEXT("GorgeousInsightDebugPanel", "Gigabytes", "GB");
		}
		else if (FMath::Abs(Value) >= MB)
		{
			Value /= MB;
			Suffix = NSLOCTEXT("GorgeousInsightDebugPanel", "Megabytes", "MB");
		}
		else if (FMath::Abs(Value) >= KB)
		{
			Value /= KB;
			Suffix = NSLOCTEXT("GorgeousInsightDebugPanel", "Kilobytes", "KB");
		}

		return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "ByteFormat", "{0} {1}"),
			FText::AsNumber(Value, &FNumberFormattingOptions()
				.SetMaximumFractionalDigits(2)
				.SetMinimumFractionalDigits(0)),
			Suffix);
	}

	FText FormatRelativeTime(const FDateTime& Timestamp)
	{
		const FTimespan Delta = FDateTime::UtcNow() - Timestamp;
		if (Delta.GetTotalSeconds() < 0.5)
		{
			return NSLOCTEXT("GorgeousInsightDebugPanel", "JustNow", "just now");
		}
		if (Delta.GetTotalMinutes() < 1.0)
		{
			return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "SecondsAgo", "{0}s ago"),
				FText::AsNumber(FMath::RoundToInt(Delta.GetTotalSeconds())));
		}
		if (Delta.GetTotalHours() < 1.0)
		{
			return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "MinutesAgo", "{0}m ago"),
				FText::AsNumber(FMath::RoundToInt(Delta.GetTotalMinutes())));
		}
		if (Delta.GetTotalDays() < 1.0)
		{
			return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "HoursAgo", "{0}h ago"),
				FText::AsNumber(FMath::RoundToInt(Delta.GetTotalHours())));
		}
		return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "DaysAgo", "{0}d ago"),
			FText::AsNumber(FMath::RoundToInt(Delta.GetTotalDays())));
	}

	FLinearColor GetActionCategoryColor(const FName Category)
	{
		if (Category == TEXT("Harness"))
		{
			return FLinearColor(0.35f, 0.6f, 1.0f, 1.f);
		}
		if (Category == TEXT("Stats"))
		{
			return FLinearColor(0.3f, 0.85f, 0.45f, 1.f);
		}
		static const TArray<FLinearColor> Palette = {
			FLinearColor(0.85f, 0.6f, 0.25f, 1.f),
			FLinearColor(0.9f, 0.35f, 0.45f, 1.f),
			FLinearColor(0.6f, 0.55f, 1.f, 1.f),
			FLinearColor(0.25f, 0.85f, 0.75f, 1.f),
			FLinearColor(0.85f, 0.75f, 0.35f, 1.f)
		};
		const int32 Index = Palette.Num() > 0 ? (GetTypeHash(Category) % Palette.Num()) : 0;
		return Palette.IsValidIndex(Index) ? Palette[Index] : FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	}
}

class SGorgeousInsightStatRow : public SMultiColumnTableRow<TSharedPtr<SGorgeousInsightDebugPanel::FStatRow>>
{
public:
	SLATE_BEGIN_ARGS(SGorgeousInsightStatRow) {}
		SLATE_ARGUMENT(TSharedPtr<SGorgeousInsightDebugPanel::FStatRow>, RowData)
		SLATE_ARGUMENT(SGorgeousInsightDebugPanel*, OwnerWidget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		RowData = InArgs._RowData;
		OwnerWidget = InArgs._OwnerWidget;
		SMultiColumnTableRow<TSharedPtr<SGorgeousInsightDebugPanel::FStatRow>>::Construct(
			FSuperRowType::FArguments().Padding(FMargin(6.f, 2.f)),
			InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!RowData.IsValid() || !OwnerWidget)
		{
			return SNew(STextBlock).Text(FText::GetEmpty());
		}

		const FGorgeousInsightStat& Stat = RowData->Stat;

		if (ColumnName == ColumnStatName)
		{
			return SNew(STextBlock)
				.Text(Stat.DisplayName)
				.ToolTipText(Stat.Description)
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"));
		}
		if (ColumnName == ColumnStatValue)
		{
			return SNew(STextBlock)
				.Text(OwnerWidget->FormatStatValue(Stat))
				.ColorAndOpacity(OwnerWidget->GetStatValueColor(Stat));
		}
		if (ColumnName == ColumnStatCategory)
		{
			return SNew(STextBlock)
				.Text(FText::FromName(Stat.Category));
		}
		if (ColumnName == ColumnStatThresholds)
		{
			return OwnerWidget ? OwnerWidget->BuildStatThresholdWidget(Stat) : SNew(STextBlock).Text(FText::GetEmpty());
		}

		return SNew(STextBlock).Text(FText::GetEmpty());
	}

private:
	TSharedPtr<SGorgeousInsightDebugPanel::FStatRow> RowData;
	SGorgeousInsightDebugPanel* OwnerWidget = nullptr;
};

class SGorgeousInsightTestRow : public SMultiColumnTableRow<TSharedPtr<SGorgeousInsightDebugPanel::FTestRow>>
{
public:
	SLATE_BEGIN_ARGS(SGorgeousInsightTestRow) {}
		SLATE_ARGUMENT(TSharedPtr<SGorgeousInsightDebugPanel::FTestRow>, RowData)
		SLATE_ARGUMENT(SGorgeousInsightDebugPanel*, OwnerWidget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		RowData = InArgs._RowData;
		OwnerWidget = InArgs._OwnerWidget;
		SMultiColumnTableRow<TSharedPtr<SGorgeousInsightDebugPanel::FTestRow>>::Construct(
			FSuperRowType::FArguments().Padding(FMargin(6.f, 2.f)),
			InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		const FGorgeousInsightTest& Test = RowData->Test;
		if (ColumnName == ColumnTestName)
		{
			const FLinearColor Accent = OwnerWidget ? OwnerWidget->GetTestStatusAccent(*RowData) : FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
			return SNew(SBorder)
				.Padding(FMargin(6.f, 4.f))
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this]() { return GetRowHoverBgColor(); }))
				[ 
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Fill)
					[
						SNew(SBorder)
						.Padding(0.f)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(Accent)
						[
							SNew(SBox)
							.WidthOverride(4.f)
							.HeightOverride(32.f)
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.Padding(FMargin(8.f, 0.f, 0.f, 0.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(Test.DisplayName)
							.ToolTipText(Test.Description)
							.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromName(Test.Id))
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
						]
					]
				];
		}
		if (ColumnName == ColumnTestCategory)
		{
			return SNew(SBorder)
				.Padding(FMargin(6.f, 4.f))
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this]() { return GetRowHoverBgColorFlat(); }))
				[
					SNew(STextBlock)
					.Text(FText::FromName(Test.Category))
				];
		}
		if (ColumnName == ColumnTestInputs)
		{
			return SNew(SBorder)
				.Padding(FMargin(6.f, 4.f))
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this]() { return GetRowHoverBgColorFlat(); }))
				[
					OwnerWidget ? OwnerWidget->BuildTestInputsWidget(RowData) : StaticCastSharedRef<SWidget>(SNew(STextBlock).Text(FText::GetEmpty()))
				];
		}
		if (ColumnName == ColumnTestResult)
		{
			return SNew(SBorder)
				.Padding(FMargin(6.f, 4.f))
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this]() { return GetRowHoverBgColorFlat(); }))
				[
					SNew(STextBlock)
					.Text(OwnerWidget ? OwnerWidget->FormatTestResultText(*RowData) : FText::GetEmpty())
				];
		}
		if (ColumnName == ColumnTestBaseline)
		{
			return SNew(SBorder)
				.Padding(FMargin(6.f, 4.f))
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this]() { return GetRowHoverBgColorFlat(); }))
				[
					SNew(STextBlock)
					.Text(OwnerWidget ? OwnerWidget->FormatBaselineText(*RowData) : FText::GetEmpty())
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				];
		}
		if (ColumnName == ColumnTestRun)
		{
			return SNew(SBorder)
				.Padding(FMargin(2.f, 0.f))
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this]() { return GetRowHoverBgColorFlat(); }))
				[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.f, 0.f, 4.f, 0.f)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ContentPadding(FMargin(2.f))
					.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "RunTest", "Run"))
					[
						SNew(SBox)
						.WidthOverride(16.f)
						.HeightOverride(16.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(FCoreStyle::Get().GetBrush("Icons.Play"))
							.DesiredSizeOverride(FVector2D(16.f, 16.f))
						]
					]
					.OnClicked(OwnerWidget, &SGorgeousInsightDebugPanel::OnRunTestClicked, RowData)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ContentPadding(FMargin(2.f))
					.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "QueueTest", "Queue"))
					[
						SNew(SBox)
						.WidthOverride(16.f)
						.HeightOverride(16.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(FCoreStyle::Get().GetBrush("Icons.Plus"))
							.DesiredSizeOverride(FVector2D(16.f, 16.f))
						]
					]
					.OnClicked(OwnerWidget, &SGorgeousInsightDebugPanel::OnQueueTestClicked, RowData)
				]
			];
		}

		return SNew(STextBlock).Text(FText::GetEmpty());
	}

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		bIsHovered_Custom = true;
		Invalidate(EInvalidateWidgetReason::Paint);
		SMultiColumnTableRow<TSharedPtr<SGorgeousInsightDebugPanel::FTestRow>>::OnMouseEnter(MyGeometry, MouseEvent);
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		bIsHovered_Custom = false;
		Invalidate(EInvalidateWidgetReason::Paint);
		SMultiColumnTableRow<TSharedPtr<SGorgeousInsightDebugPanel::FTestRow>>::OnMouseLeave(MouseEvent);
	}

private:
	// Used for ColumnTestName, keeps a dark base and transitions to a blue-ish highlight on hover.
	FSlateColor GetRowHoverBgColor() const
	{
		return bIsHovered_Custom
			? FLinearColor(0.13f, 0.22f, 0.35f, 1.f)
			: FLinearColor(0.06f, 0.06f, 0.06f, 0.9f);
	}

	// Used for all other columns, transparent at rest, same highlight on hover.
	FSlateColor GetRowHoverBgColorFlat() const
	{
		return bIsHovered_Custom
			? FLinearColor(0.13f, 0.22f, 0.35f, 1.f)
			: FLinearColor(0.f, 0.f, 0.f, 0.f);
	}

	TSharedPtr<SGorgeousInsightDebugPanel::FTestRow> RowData;
	SGorgeousInsightDebugPanel* OwnerWidget = nullptr;
	bool bIsHovered_Custom = false;
};

void SGorgeousInsightDebugPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Threshold Monitoring
	for (const TSharedPtr<FStatRow>& Row : StatItems)
	{
		if (Row->Stat.ValueType != EGorgeousInsightStatValueType::Number) continue;

		FName StatKey = MakeStatThresholdKey(Row->Stat);
		double CurrentValue = Row->Stat.NumericValue;
		
		bool bBreachedCritical = false;
		bool bBreachedWarning = false;
		double ThresholdVal = 0.0;

		if (double* Crit = StatCriticalThresholds.Find(StatKey))
		{
			if (CurrentValue > *Crit) { bBreachedCritical = true; ThresholdVal = *Crit; }
		}
		
		if (!bBreachedCritical)
		{
			if (double* Warn = StatWarningThresholds.Find(StatKey))
			{
				if (CurrentValue > *Warn) { bBreachedWarning = true; ThresholdVal = *Warn; }
			}
		}

		if (bBreachedCritical || bBreachedWarning)
		{
			double LastToast = LastToastTimes.FindRef(StatKey);
			if (InCurrentTime - LastToast > 5.0) // 5 second debounce
			{
				LastToastTimes.Add(StatKey, InCurrentTime);
				if (bBreachedCritical)
				{
					GT_E_LOG_FULL_EX(TEXT("MatrixThreshold"), TEXT("Critical Threshold Breached! %s is %.1f (Limit: %.1f)"), 4.0f, true, true, false, true, nullptr, nullptr, *Row->Stat.DisplayName.ToString(), CurrentValue, ThresholdVal);
				}
				else
				{
					GT_W_LOG_FULL_EX(TEXT("MatrixThreshold"), TEXT("Warning Threshold Exceeded! %s is %.1f (Limit: %.1f)"), 4.0f, true, true, false, true, nullptr, nullptr, *Row->Stat.DisplayName.ToString(), CurrentValue, ThresholdVal);
				}
			}
		}
	}
}

SGorgeousInsightDebugPanel::~SGorgeousInsightDebugPanel()
{
#if WITH_EDITOR
	if (EndPieDelegateHandle.IsValid() && FEditorDelegates::BeginPIE.IsBoundToObject(this))
	{
		FEditorDelegates::BeginPIE.Remove(EndPieDelegateHandle);
	}
#endif
}

void SGorgeousInsightDebugPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				BuildContextSwitcher()
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 6.f)
				[
					BuildToolbar()
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SNew(SSplitter)
					+ SSplitter::Slot()
					.Value(0.25f)
					[
						BuildProviderList()
					]
				+ SSplitter::Slot()
				.Value(0.75f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 10.f)
						[
							BuildStatsList()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 10.f)
						[
							BuildChartsPanel()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 10.f)
						[
							BuildActionsPanel()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							BuildTestsList()
						]
					]
				]
			]
			]
		]
	];

	RefreshFromSubsystem();
	
#if WITH_EDITOR
	EndPieDelegateHandle = FEditorDelegates::BeginPIE.AddLambda([this](bool bIsSimulating) {
		// User requested: "when we start a new pie we clear the live"
		// If we switch to Live automatically on BeginPIE, we can do that here.
		if (CurrentContext.Mode != EGorgeousInsightContextMode::Live) {
			CurrentContext.Mode = EGorgeousInsightContextMode::Live;
			CurrentContext.WorldContext = nullptr; // Will be set next tick
			CurrentContext.InstanceName.Empty();
			RefreshProviderData();
			BroadcastStateChanged();
		}
	});
#endif
}

SGorgeousInsightDebugPanel::FInsightPanelState SGorgeousInsightDebugPanel::ExportState() const
{
	FInsightPanelState State;
	State.SelectedProvider = SelectedProvider;
	State.Context = CurrentContext;
	State.SelectedStatCategory = SelectedStatCategory;
	State.ProviderFilter = ProviderFilter;
	State.bRunWithHarness = bRunWithHarness;
	State.TestInputValues = TestInputValues;
	State.StatWarningThresholds = StatWarningThresholds;
	State.StatCriticalThresholds = StatCriticalThresholds;
	return State;
}

void SGorgeousInsightDebugPanel::ImportState(const FInsightPanelState& State)
{
	TGuardValue<bool> Guard(bSuppressStateBroadcast, true);
	SelectedProvider = State.SelectedProvider;
	CurrentContext = State.Context;
	SelectedStatCategory = State.SelectedStatCategory;
	ProviderFilter = State.ProviderFilter;
	bRunWithHarness = State.bRunWithHarness;
	TestInputValues = State.TestInputValues;
	StatWarningThresholds = State.StatWarningThresholds;
	StatCriticalThresholds = State.StatCriticalThresholds;

	RefreshProviders();
	RefreshProviderData();
}

void SGorgeousInsightDebugPanel::RefreshFromSubsystem()
{
	RefreshProviders();
	RefreshProviderData();
}

void SGorgeousInsightDebugPanel::RefreshProviders()
{
	ProviderItems.Reset();

	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		const TArray<IGorgeousInsightMatrixProvider*> Providers = Subsystem->GetProviders();
		ProviderItems.Reserve(Providers.Num());
		for (IGorgeousInsightMatrixProvider* Provider : Providers)
		{
			if (!Provider)
			{
				continue;
			}
			TSharedPtr<FProviderEntry> Entry = MakeShared<FProviderEntry>();
			Entry->ProviderName = Provider->GetProviderName();
			Entry->DisplayName = Provider->GetProviderDisplayName();
			TArray<FGorgeousInsightStat> Stats;
			TArray<FGorgeousInsightAction> Actions;
			TArray<FGorgeousInsightTest> Tests;
			Subsystem->GatherProviderStats(CurrentContext, Entry->ProviderName, Stats);
			Subsystem->GatherProviderActions(Entry->ProviderName, Actions);
			Subsystem->GatherProviderTests(Entry->ProviderName, Tests);
			Entry->StatCount = Stats.Num();
			Entry->ActionCount = Actions.Num();
			Entry->TestCount = Tests.Num();
			ProviderItems.Add(Entry);
		}
	}

	ProviderItems.Sort([](const TSharedPtr<FProviderEntry>& A, const TSharedPtr<FProviderEntry>& B)
	{
		return A.IsValid() && B.IsValid() ? A->DisplayName.ToString() < B->DisplayName.ToString() : false;
	});

	if (!ProviderFilter.IsEmpty())
	{
		ProviderItems.RemoveAll([this](const TSharedPtr<FProviderEntry>& Entry)
		{
			if (!Entry.IsValid())
			{
				return true;
			}
			return !Entry->DisplayName.ToString().Contains(ProviderFilter, ESearchCase::IgnoreCase);
		});
	}

	if (ProviderListView.IsValid())
	{
		ProviderListView->RequestListRefresh();
		const bool bSelectionValid = ProviderItems.ContainsByPredicate([this](const TSharedPtr<FProviderEntry>& Entry)
		{
			return Entry.IsValid() && Entry->ProviderName == SelectedProvider;
		});
		if (!bSelectionValid)
		{
			SelectedProvider = NAME_None;
		}
		TSharedPtr<FProviderEntry> DesiredSelection = ProviderItems.Num() > 0 ? ProviderItems[0] : nullptr;
		if (!SelectedProvider.IsNone())
		{
			if (TSharedPtr<FProviderEntry>* Found = ProviderItems.FindByPredicate([this](const TSharedPtr<FProviderEntry>& Entry)
			{
				return Entry.IsValid() && Entry->ProviderName == SelectedProvider;
			}))
			{
				DesiredSelection = *Found;
			}
		}
		if (DesiredSelection.IsValid())
		{
			SelectedProvider = DesiredSelection->ProviderName;
			bRefreshingProviderSelection = true;
			ProviderListView->SetSelection(DesiredSelection, ESelectInfo::Direct);
			bRefreshingProviderSelection = false;
		}
	}
}

void SGorgeousInsightDebugPanel::RefreshProviderData()
{
	StatItems.Empty();
	TestItems.Empty();
	ActionItems.Empty();
	RawProviderStats.Empty();

	if (StatCategoriesBox.IsValid())
	{
		StatCategoriesBox->ClearChildren();
	}

	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		if (SelectedProvider != NAME_None)
		{
			Subsystem->GatherProviderStats(CurrentContext, SelectedProvider, RawProviderStats);
			Subsystem->GatherProviderActions(SelectedProvider, ActionItems);

			TArray<FGorgeousInsightTest> TempTests;
			Subsystem->GatherProviderTests(SelectedProvider, TempTests);

			for (const FGorgeousInsightTest& Test : TempTests)
			{
				TSharedPtr<FTestRow> Row = MakeShared<FTestRow>();
				Row->Test = Test;
				if (const FGorgeousInsightTestResult* Baseline = GetBaselineResult(SelectedProvider, Test.Id))
				{
					Row->LastResult = *Baseline;
					Row->bHasRun = true;
				}
				TestItems.Add(Row);
			}
		}
	}

	// Build Categories
	TSet<FName> UniqueCategories;
	for (const FGorgeousInsightStat& Stat : RawProviderStats)
	{
		if (!Stat.Category.IsNone())
		{
			UniqueCategories.Add(Stat.Category);
		}
	}

	TArray<FName> SortedCategories = UniqueCategories.Array();
	SortedCategories.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });

	if (StatCategoriesBox.IsValid())
	{
		// "All" Tab
		StatCategoriesBox->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([this]() { return SelectedStatCategory == NAME_None ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { if (State == ECheckBoxState::Checked) OnStatCategoryChanged(NAME_None); })
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("All")))
				]
			];

		for (FName Cat : SortedCategories)
		{
			StatCategoriesBox->AddSlot()
				.AutoWidth()
				.Padding(0.f, 0.f, 4.f, 0.f)
				[
					SNew(SCheckBox)
					.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
					.IsChecked_Lambda([this, Cat]() { return SelectedStatCategory == Cat ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this, Cat](ECheckBoxState State) { if (State == ECheckBoxState::Checked) OnStatCategoryChanged(Cat); })
					[
						SNew(STextBlock).Text(FText::FromName(Cat))
					]
				];
		}
	}

	// Filter Stats
	for (const FGorgeousInsightStat& Stat : RawProviderStats)
	{
		if (SelectedStatCategory == NAME_None || Stat.Category == SelectedStatCategory)
		{
			TSharedPtr<FStatRow> Row = MakeShared<FStatRow>();
			Row->Stat = Stat;
			StatItems.Add(Row);
		}
	}

	// Sort Stats Alphabetically
	StatItems.Sort([](const TSharedPtr<FStatRow>& A, const TSharedPtr<FStatRow>& B)
	{
		return A->Stat.DisplayName.ToString() < B->Stat.DisplayName.ToString();
	});

	if (StatsListView.IsValid())
	{
		StatsListView->RequestListRefresh();
	}
	if (TestsListView.IsValid())
	{
		TestsListView->RequestListRefresh();
	}

	RebuildActions();
	RebuildCharts();
}

void SGorgeousInsightDebugPanel::OnStatCategoryChanged(FName NewCategory)
{
	SelectedStatCategory = NewCategory;
	RefreshProviderData();
	BroadcastStateChanged();
}
void SGorgeousInsightDebugPanel::RebuildActions()
{
	if (!ActionsGrid.IsValid())
	{
		return;
	}

	ActionsGrid->ClearChildren();
	UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get();

	const int32 ColumnsPerRow = 3; // 3 columns grid
	int32 ActionIndex = 0;

	for (const FGorgeousInsightAction& Action : ActionItems)
	{
		const FLinearColor Accent = GetActionCategoryColor(Action.Category);
		UGorgeousInsightMatrixSubsystem::FGorgeousInsightLastActionStats LastActionStats;
		const bool bHasStats = Subsystem && Subsystem->GetLastActionStats(SelectedProvider, Action.Id, LastActionStats) && LastActionStats.bHasRun;
		const FText LastRunText = bHasStats
			? FormatRelativeTime(LastActionStats.Timestamp)
			: NSLOCTEXT("GorgeousInsightDebugPanel", "ActionNeverRun", "Never");
		const FText DurationText = bHasStats
			? FormatSeconds(LastActionStats.DurationSeconds)
			: NSLOCTEXT("GorgeousInsightDebugPanel", "ActionDurationNA", "--");
		const FText ResultText = bHasStats
			? (LastActionStats.bSucceeded
				? NSLOCTEXT("GorgeousInsightDebugPanel", "ActionResultOk", "OK")
				: NSLOCTEXT("GorgeousInsightDebugPanel", "ActionResultFail", "Failed"))
			: NSLOCTEXT("GorgeousInsightDebugPanel", "ActionResultNA", "--");

		const int32 Column = ActionIndex % ColumnsPerRow;
		const int32 Row = ActionIndex / ColumnsPerRow;

		ActionsGrid->AddSlot(Column, Row)
		[
			SNew(SBorder)
			.Padding(4.f)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.Padding(FMargin(8.f, 6.f))
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(Accent.CopyWithNewOpacity(0.2f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SBorder)
							.Padding(0.f)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(Accent)
							[
								SNew(SBox)
								.WidthOverride(4.f)
								.HeightOverride(24.f)
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.Padding(FMargin(8.f, 0.f, 0.f, 0.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(Action.DisplayName)
								.ToolTipText(Action.Description)
								.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(FText::FromName(Action.Category))
								.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
								.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(6.f, 6.f, 6.f, 4.f)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FMargin(4.f, 2.f))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(STextBlock)
						.Text(FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "ActionLastRun", "Last: {0}"), LastRunText))
						.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(STextBlock)
						.Text(FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "ActionDur", "Dur: {0}"), DurationText))
						.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
					+ SUniformGridPanel::Slot(0, 1)
					[
						SNew(STextBlock)
						.Text(FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "ActionResult", "Result: {0}"), ResultText))
						.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(SBorder)
					.Padding(FMargin(6.f, 4.f))
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.06f, 0.9f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.OnClicked(this, &SGorgeousInsightDebugPanel::OnActionClicked, Action.Id)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(14.f)
									.HeightOverride(14.f)
									[
										SNew(SImage)
										.Image(FCoreStyle::Get().GetBrush("Icons.Play"))
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ActionRun", "Run"))
								]
							]
						]
					]
				]
			]
		];

		++ActionIndex;
	}

	if (ActionItems.IsEmpty())
	{
		ActionsGrid->AddSlot(0, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "NoActions", "No actions available for this provider."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
	}
}

void SGorgeousInsightDebugPanel::RebuildCharts()
{
	if (!ChartsGrid.IsValid())
	{
		return;
	}

	ChartsGrid->ClearChildren();

	if (SelectedProvider.IsNone())
	{
		ChartsGrid->AddSlot(0, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ChartsNoProvider", "Select a provider to view charts."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
		return;
	}

	TArray<FGorgeousInsightChartDefinition> Charts;
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->GatherProviderCharts(SelectedProvider, Charts);
	}

	if (Charts.IsEmpty())
	{
		ChartsGrid->AddSlot(0, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ChartsEmpty", "No charts available for this provider."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
		return;
	}

	const int32 ColumnsPerRow = 3;
	int32 ChartIndex = 0;
	for (const FGorgeousInsightChartDefinition& Chart : Charts)
	{
		TSharedRef<SWidget> ChartWidget = SNew(STextBlock).Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ChartUnsupported", "Unsupported chart"));
		bool bIsCustomRendered = false;
		if (!Chart.CustomChartType.IsNone())
		{
			if (const UGorgeousInsightChartRegistry* Registry = GetDefault<UGorgeousInsightChartRegistry>())
			{
				if (const TSoftClassPtr<UUserWidget>* WidgetClassPtr = Registry->RegisteredUMGCharts.Find(Chart.CustomChartType))
				{
					if (UClass* WidgetClass = WidgetClassPtr->LoadSynchronous())
					{
						UWorld* World = nullptr;
						if (GEngine && GEngine->GetWorldContexts().Num() > 0)
						{
							World = GEngine->GetWorldContexts()[0].World();
						}
						
						if (World)
						{
							if (UUserWidget* Widget = CreateWidget<UUserWidget>(World, WidgetClass))
							{
								if (Widget->Implements<UGorgeousInsightChartWidgetInterface>())
								{
									if (Chart.InstancedPayload.IsValid())
									{
										IGorgeousInsightChartWidgetInterface::Execute_ReceiveInstancedChartData(Widget, Chart.Title, Chart.Subtitle, Chart.InstancedPayload);
									}
									else
									{
										IGorgeousInsightChartWidgetInterface::Execute_ReceiveChartData(Widget, Chart.Title, Chart.Subtitle, Chart.Payload);
									}
								}
								ChartWidget = SNew(SObjectWidget, Widget);
								bIsCustomRendered = true;
							}
						}
					}
				}
			}
		}

		if (!bIsCustomRendered)
		{
		switch (Chart.Type)
		{
		case EGorgeousInsightChartType::Bar:
		{
			TSharedRef<SGorgeousInsightMiniBarChart> Widget = SNew(SGorgeousInsightMiniBarChart);
			Widget->SetValues(Chart.Bars, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Line:
		{
			TSharedRef<SGorgeousInsightMiniLineChart> Widget = SNew(SGorgeousInsightMiniLineChart);
			Widget->SetValues(Chart.LineValues, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Histogram:
		{
			TSharedRef<SGorgeousInsightMiniHistogram> Widget = SNew(SGorgeousInsightMiniHistogram);
			Widget->SetValues(Chart.HistogramBins, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Pie:
		{
			TSharedRef<SGorgeousInsightMiniPieChart> Widget = SNew(SGorgeousInsightMiniPieChart);
			Widget->SetValues(Chart.PieSlices, Chart.Title, Chart.Subtitle, Chart.bPieDonut);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Scatter:
		{
			TSharedRef<SGorgeousInsightMiniScatterPlot> Widget = SNew(SGorgeousInsightMiniScatterPlot);
			Widget->SetValues(Chart.ScatterPoints, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Heatmap:
		{
			TSharedRef<SGorgeousInsightMiniHeatmap> Widget = SNew(SGorgeousInsightMiniHeatmap);
			Widget->SetValues(Chart.HeatmapValues, Chart.HeatmapRows, Chart.HeatmapColumns, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Timeline:
		{
			TSharedRef<SGorgeousInsightMiniTimeline> Widget = SNew(SGorgeousInsightMiniTimeline);
			Widget->SetSegments(Chart.TimelineSegments, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Flow:
		{
			TSharedRef<SGorgeousInsightMiniFlowChart> Widget = SNew(SGorgeousInsightMiniFlowChart);
			Widget->SetGraph(Chart.FlowNodes, Chart.FlowEdges, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		case EGorgeousInsightChartType::Table:
		{
			TSharedRef<SGorgeousInsightMiniTable> Widget = SNew(SGorgeousInsightMiniTable);
			Widget->SetRows(Chart.TableRows, Chart.Title, Chart.Subtitle);
			ChartWidget = Widget;
			break;
		}
		default:
			break;
		}

	}
	const int32 Column = ChartIndex % ColumnsPerRow;
	const int32 Row = ChartIndex / ColumnsPerRow;
	ChartsGrid->AddSlot(Column, Row)
	[
		ChartWidget
	];
	++ChartIndex;
	}
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildToolbar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "Title", "Gorgeous Insight Matrix"))
			.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			SNew(STextBlock)
			.Text(this, &SGorgeousInsightDebugPanel::GetSelectedProviderText)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			SNew(STextBlock)
			.Text(this, &SGorgeousInsightDebugPanel::GetStatsCountText)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			SNew(STextBlock)
			.Text(this, &SGorgeousInsightDebugPanel::GetActionsCountText)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			SNew(STextBlock)
			.Text(this, &SGorgeousInsightDebugPanel::GetTestsCountText)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SGorgeousInsightDebugPanel::GetHarnessToggleState)
			.OnCheckStateChanged(this, &SGorgeousInsightDebugPanel::OnHarnessToggleChanged)
			.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "HarnessToggleTooltip", "Toggle Gauntlet harness for multi-session tests."))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "HarnessToggle", "Use Gauntlet Harness"))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(FMargin(4.f, 2.f))
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.06f, 0.85f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.f, 0.f)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "Refresh", "Refresh"))
					.OnClicked(this, &SGorgeousInsightDebugPanel::OnRefreshClicked)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(16.f)
							.HeightOverride(16.f)
							[
								SNew(SImage)
								.Image(FCoreStyle::Get().GetBrush("Icons.Refresh"))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "RefreshLabel", "Refresh"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.f, 0.f)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "ExportCsv", "Export CSV"))
					.OnClicked(this, &SGorgeousInsightDebugPanel::OnExportStatsCsvClicked)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(16.f)
							.HeightOverride(16.f)
							[
								SNew(SImage)
								.Image(FCoreStyle::Get().GetBrush("Icons.Save"))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ExportCsvLabel", "CSV"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.f, 0.f)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "ExportJson", "Export JSON"))
					.OnClicked(this, &SGorgeousInsightDebugPanel::OnExportStatsJsonClicked)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(16.f)
							.HeightOverride(16.f)
							[
								SNew(SImage)
								.Image(FCoreStyle::Get().GetBrush("Icons.Save"))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ExportJsonLabel", "JSON"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
			]
		];
}


TSharedPtr<SWidget> SGorgeousInsightDebugPanel::OnStatContextMenuOpening()
{
	if (CurrentContext.Mode != EGorgeousInsightContextMode::Baseline)
	{
		return nullptr;
	}

	TArray<TSharedPtr<FStatRow>> SelectedItems = StatsListView->GetSelectedItems();
	if (SelectedItems.Num() == 0)
	{
		return nullptr;
	}

	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("GorgeousInsightDebugPanel", "RemoveBaselineStat", "Remove Baseline Stat"),
		NSLOCTEXT("GorgeousInsightDebugPanel", "RemoveBaselineStatTooltip", "Removes this stat from the saved baseline."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this, SelectedItems]() {
				if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
				{
					for (TSharedPtr<FStatRow> Item : SelectedItems)
					{
						Subsystem->RemoveBaselineStat(SelectedProvider, Item->Stat.Id);
					}
					RefreshProviderData();
				}
			})
		)
	);

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildProviderList()
{
	return SNew(SBorder)
		.Padding(6.f)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SSearchBox)
				.HintText(NSLOCTEXT("GorgeousInsightDebugPanel", "ProviderSearch", "Search providers..."))
				.OnTextChanged(this, &SGorgeousInsightDebugPanel::OnProviderFilterChanged)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(ProviderListView, SListView<TSharedPtr<FProviderEntry>>)
				.ListItemsSource(&ProviderItems)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow_Lambda([this](TSharedPtr<FProviderEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(STableRow<TSharedPtr<FProviderEntry>>, OwnerTable)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(Item.IsValid() ? Item->DisplayName : FText::GetEmpty())
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(Item.IsValid() ? FormatProviderBadgeText(*Item) : FText::GetEmpty())
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						]
					];
				})
				.OnSelectionChanged(this, &SGorgeousInsightDebugPanel::OnProviderSelectionChanged)
			]
		];
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildStatsList()
{
	return SNew(SBorder)
		.Padding(6.f)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			
			// Baseline Info Banner (Only visible in Baseline context)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.Visibility_Lambda([this]() { return CurrentContext.Mode == EGorgeousInsightContextMode::Baseline ? EVisibility::Visible : EVisibility::Collapsed; })
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.2f, 0.6f, 0.9f, 0.3f))
				.Padding(FMargin(8.f, 8.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("Icons.Info"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "BaselineInfoBanner", "Baseline view represents the reference stats stored on disk. You can view saved baselines here. Developers typically do not need to interact with this view unless explicitly committing new baselines."))
						.AutoWrapText(true)
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]
			
			// Timeline Scrubber (Only visible in Editor context)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.Visibility_Lambda([this]() { return CurrentContext.Mode == EGorgeousInsightContextMode::Editor ? EVisibility::Visible : EVisibility::Collapsed; })
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(FMargin(8.f, 4.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "TimelineHistory", "Editor Timeline History"))
							.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
							.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.HAlign(HAlign_Right)
						[
							SNew(STextBlock)
							.Text_Lambda([]() {
								if (auto Subsystem = UGorgeousInsightMatrixSubsystem::Get()) {
									int32 Idx = Subsystem->GetActiveHistoryIndex();
									if (Subsystem->GetEditorHistory().IsValidIndex(Idx)) {
										return FText::FromString(Subsystem->GetEditorHistory()[Idx].Timestamp.ToString());
									}
								}
								return FText::FromString(TEXT("Live"));
							})
							.ColorAndOpacity(FLinearColor(0.4f, 0.8f, 1.0f))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SSlider)
						.Value_Lambda([]() {
							if (auto Subsystem = UGorgeousInsightMatrixSubsystem::Get()) {
								int32 Count = Subsystem->GetEditorHistory().Num();
								if (Count <= 1) return 1.0f;
								return (float)Subsystem->GetActiveHistoryIndex() / (Count - 1);
							}
							return 1.0f;
						})
						.OnValueChanged_Lambda([this](float Val) {
							if (auto Subsystem = UGorgeousInsightMatrixSubsystem::Get()) {
								int32 Count = Subsystem->GetEditorHistory().Num();
								if (Count > 0) {
									int32 Idx = FMath::Clamp(FMath::RoundToInt(Val * (Count - 1)), 0, Count - 1);
									if (Idx != Subsystem->GetActiveHistoryIndex()) {
										Subsystem->ApplyHistorySnapshot(Idx);
										RefreshProviderData();
									}
								}
							}
						})
					]
				]
			]
			
			// Stats Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "StatsHeader", "Stats"))
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(220.f)
			[
				SAssignNew(StatCategoriesBox, SHorizontalBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(220.f)
			[
				SAssignNew(StatsListView, SListView<TSharedPtr<FStatRow>>)
				.ListItemsSource(&StatItems)
				.SelectionMode(ESelectionMode::Single)
				.OnContextMenuOpening(this, &SGorgeousInsightDebugPanel::OnStatContextMenuOpening)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(ColumnStatName)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "StatName", "Stat"))
						.FillWidth(0.4f)
					+ SHeaderRow::Column(ColumnStatValue)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "StatValue", "Value"))
						.FillWidth(0.2f)
					+ SHeaderRow::Column(ColumnStatCategory)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "StatCategory", "Category"))
						.FillWidth(0.2f)
					+ SHeaderRow::Column(ColumnStatThresholds)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "StatThresholds", "Thresholds"))
						.FillWidth(0.2f)
				)
				.OnGenerateRow_Lambda([this](TSharedPtr<FStatRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SGorgeousInsightStatRow, OwnerTable)
						.RowData(Item)
						.OwnerWidget(this);
				})
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "StatsEmpty", "No stats available."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.Visibility_Lambda([this]()
				{
					return StatItems.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
		];
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildStatThresholdWidget(const FGorgeousInsightStat& Stat)
{
	if (Stat.ValueType != EGorgeousInsightStatValueType::Number)
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 4.f, 0.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "WarnThresholdLabel", "W"))
			.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "WarnThresholdTooltip", "Warning Threshold - Stat value displays in orange when exceeded"))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(50.f)
			.Text_Lambda([this, Stat]()
			{
				return FText::FromString(GetStatThresholdText(Stat, false));
			})
			.OnTextCommitted_Lambda([this, Stat](const FText& NewText, ETextCommit::Type)
			{
				SetStatThresholdValue(Stat, false, NewText.ToString());
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 4.f, 0.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "CriticalThresholdLabel", "C"))
			.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "CriticalThresholdTooltip", "Critical Threshold - Stat value displays in red when exceeded"))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(50.f)
			.Text_Lambda([this, Stat]()
			{
				return FText::FromString(GetStatThresholdText(Stat, true));
			})
			.OnTextCommitted_Lambda([this, Stat](const FText& NewText, ETextCommit::Type)
			{
				SetStatThresholdValue(Stat, true, NewText.ToString());
			})
		]
		;
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildChartsPanel()
{
	return SNew(SBorder)
		.Padding(6.f)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ChartsHeader", "Charts"))
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ChartsGrid, SUniformGridPanel)
				.SlotPadding(FMargin(6.f, 4.f))
			]
		];
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildActionsPanel()
{
	return SNew(SBorder)
		.Padding(6.f)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "ActionsHeader", "Actions"))
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(ActionsGrid, SUniformGridPanel)
					.SlotPadding(FMargin(6.f, 4.f))
				]
			]
		];
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildTestsList()
{
	return SNew(SBorder)
		.Padding(6.f)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "TestsHeader", "Tests"))
					.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.OnClicked(this, &SGorgeousInsightDebugPanel::OnRunQueuedTestsClicked)
					.ToolTipText(NSLOCTEXT("GorgeousInsightDebugPanel", "RunQueued", "Run queued tests"))
					[
						SNew(SBorder)
						.Padding(FMargin(8.f, 4.f))
						.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
						.BorderBackgroundColor(FLinearColor(0.07f, 0.07f, 0.07f, 0.9f))
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(16.f)
								.HeightOverride(16.f)
								[
									SNew(SImage)
									.Image(FCoreStyle::Get().GetBrush("Icons.Play"))
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "RunQueuedLabel", "Run Queued"))
							]
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(TestsListView, SListView<TSharedPtr<FTestRow>>)
				.ListItemsSource(&TestItems)
				.SelectionMode(ESelectionMode::Single)
				.OnContextMenuOpening(this, &SGorgeousInsightDebugPanel::OnStatContextMenuOpening)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(ColumnTestName)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "TestName", "Test"))
						.FillWidth(0.28f)
					+ SHeaderRow::Column(ColumnTestCategory)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "TestCategory", "Category"))
						.FillWidth(0.14f)
					+ SHeaderRow::Column(ColumnTestInputs)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "TestInputs", "Inputs"))
						.FillWidth(0.2f)
					+ SHeaderRow::Column(ColumnTestResult)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "TestResult", "Result"))
						.FillWidth(0.14f)
					+ SHeaderRow::Column(ColumnTestBaseline)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "TestBaseline", "Baseline"))
						.FillWidth(0.14f)
					+ SHeaderRow::Column(ColumnTestRun)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "TestRun", ""))
						.FillWidth(0.1f)
				)
				.OnGenerateRow_Lambda([this](TSharedPtr<FTestRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SGorgeousInsightTestRow, OwnerTable)
						.RowData(Item)
						.OwnerWidget(this);
				})
			]
		];
}

#include "Widgets/Input/SComboBox.h"

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildTestInputsWidget(const TSharedPtr<FTestRow>& RowData)
{
	if (!RowData.IsValid() || RowData->Test.Inputs.IsEmpty())
	{
		return SNew(STextBlock).Text(FText::GetEmpty());
	}

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	for (const FGorgeousInsightTest::FGorgeousInsightTestInput& Input : RowData->Test.Inputs)
	{
		const FName TestId = RowData->Test.Id;
		const FName InputId = Input.Id;
		const FString CurrentValue = GetTestInputValue(TestId, InputId, Input.DefaultValue);

		TSharedRef<SWidget> InputWidget = SNew(STextBlock).Text(FText::GetEmpty());
		if (Input.Type == FGorgeousInsightTest::FGorgeousInsightTestInput::EGorgeousInsightTestInputType::Bool)
		{
			const bool bChecked = CurrentValue.Equals(TEXT("true"), ESearchCase::IgnoreCase) || CurrentValue == TEXT("1");
			InputWidget = SNew(SCheckBox)
				.IsChecked(bChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this, TestId, InputId](ECheckBoxState NewState)
				{
					SetTestInputValue(TestId, InputId, NewState == ECheckBoxState::Checked ? TEXT("true") : TEXT("false"));
				});
		}
		else if (Input.Type == FGorgeousInsightTest::FGorgeousInsightTestInput::EGorgeousInsightTestInputType::Class)
		{
#if WITH_EDITOR
			UClass* AllowedClass = nullptr;
			if (!Input.AllowedClassPath.IsEmpty())
			{
				FSoftClassPath AllowedPath(Input.AllowedClassPath);
				AllowedClass = AllowedPath.TryLoadClass<UObject>();
			}

			InputWidget = SNew(SClassPropertyEntryBox)
				.MetaClass(AllowedClass)
				.AllowNone(!Input.bRequired)
				.SelectedClass_Lambda([this, TestId, InputId]() -> const UClass*
				{
					const FString StoredValue = GetTestInputValue(TestId, InputId, FString());
					if (StoredValue.IsEmpty())
					{
						return nullptr;
					}
					FSoftClassPath ValuePath(StoredValue);
					return ValuePath.TryLoadClass<UObject>();
				})
				.OnSetClass_Lambda([this, TestId, InputId](const UClass* NewClass)
				{
					SetTestInputValue(TestId, InputId, NewClass ? NewClass->GetPathName() : FString());
				});
#else
			InputWidget = SNew(SEditableTextBox)
				.Text(FText::FromString(CurrentValue))
				.OnTextChanged_Lambda([this, TestId, InputId](const FText& NewText)
				{
					SetTestInputValue(TestId, InputId, NewText.ToString());
				});
#endif
		}
		else if (Input.Type == FGorgeousInsightTest::FGorgeousInsightTestInput::EGorgeousInsightTestInputType::Dropdown)
		{
			// Ensure pointers array is populated
			TArray<TSharedPtr<FString>>* PtrsArray = const_cast<TArray<TSharedPtr<FString>>*>(&Input.DropdownOptionPtrs);
			if (PtrsArray->Num() != Input.DropdownOptions.Num())
			{
				PtrsArray->Empty();
				for (const FString& Option : Input.DropdownOptions)
				{
					PtrsArray->Add(MakeShared<FString>(Option));
				}
			}

			TSharedPtr<FString> InitiallySelected;
			for (const TSharedPtr<FString>& Ptr : *PtrsArray)
			{
				if (Ptr.IsValid() && *Ptr == CurrentValue)
				{
					InitiallySelected = Ptr;
					break;
				}
			}
			if (!InitiallySelected.IsValid() && PtrsArray->Num() > 0)
			{
				InitiallySelected = (*PtrsArray)[0];
			}

			InputWidget = SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(PtrsArray)
				.InitiallySelectedItem(InitiallySelected)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : FString()));
				})
				.OnSelectionChanged_Lambda([this, TestId, InputId](TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
				{
					if (SelectedItem.IsValid())
					{
						SetTestInputValue(TestId, InputId, *SelectedItem);
					}
				})
				[
					SNew(STextBlock).Text_Lambda([this, TestId, InputId, PtrsArray]()
					{
						return FText::FromString(GetTestInputValue(TestId, InputId, PtrsArray->Num() > 0 ? *(*PtrsArray)[0] : FString()));
					})
				];
		}
		else
		{
			InputWidget = SNew(SEditableTextBox)
				.Text(FText::FromString(CurrentValue))
				.OnTextChanged_Lambda([this, TestId, InputId](const FText& NewText)
				{
					SetTestInputValue(TestId, InputId, NewText.ToString());
				});
		}

		Box->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Input.DisplayName.IsEmpty() ? FText::FromName(Input.Id) : Input.DisplayName)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				InputWidget
			]
		];
	}

	return Box;
}

void SGorgeousInsightDebugPanel::OnProviderSelectionChanged(TSharedPtr<FProviderEntry> Item, ESelectInfo::Type SelectInfo)
{
	if (bRefreshingProviderSelection)
	{
		return;
	}

	if (!Item.IsValid())
	{
		return;
	}

	SelectedProvider = Item.IsValid() ? Item->ProviderName : NAME_None;
	RefreshProviderData();
	BroadcastStateChanged();
}

FReply SGorgeousInsightDebugPanel::OnRefreshClicked()
{
	RefreshFromSubsystem();
	return FReply::Handled();
}

FReply SGorgeousInsightDebugPanel::OnActionClicked(FName ActionId)
{
	if (SelectedProvider.IsNone())
	{
		return FReply::Handled();
	}

	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		FGorgeousInsightActionContext Context;
		Context.WorldContextObject = nullptr;
		Subsystem->ExecuteAction(SelectedProvider, ActionId, Context);
	}
	RebuildActions();

	return FReply::Handled();
}


FReply SGorgeousInsightDebugPanel::OnRunTestClicked(TSharedPtr<FTestRow> RowData)
{
	if (!RowData.IsValid() || SelectedProvider.IsNone())
	{
		return FReply::Handled();
	}

	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		FGorgeousInsightTestContext Context;
		Context.WorldContextObject = nullptr;
		ApplyHarnessParameters(Context.Parameters);
		ApplyTestInputParameters(RowData->Test, Context.Parameters);
		RowData->LastResult = Subsystem->RunTest(SelectedProvider, RowData->Test.Id, Context);
		RowData->bHasRun = true;
		if (TestsListView.IsValid())
		{
			TestsListView->RebuildList();
		}
		
		if (CurrentContext.Mode == EGorgeousInsightContextMode::Baseline && !RowData->LastResult.Metrics.IsEmpty())
		{
			OpenBaselineTestResultPopup(RowData->Test, RowData->LastResult);
		}
	}

	return FReply::Handled();
}

FReply SGorgeousInsightDebugPanel::OnQueueTestClicked(TSharedPtr<FTestRow> RowData)
{
	if (!RowData.IsValid() || SelectedProvider.IsNone())
	{
		return FReply::Handled();
	}

	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		FGorgeousInsightTestContext Context;
		Context.WorldContextObject = nullptr;
		ApplyHarnessParameters(Context.Parameters);
		ApplyTestInputParameters(RowData->Test, Context.Parameters);
		Subsystem->EnqueueTest(SelectedProvider, RowData->Test.Id, Context);
	}

	return FReply::Handled();
}

FReply SGorgeousInsightDebugPanel::OnRunQueuedTestsClicked()
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		const TArray<UGorgeousInsightMatrixSubsystem::FGorgeousInsightQueuedTestResult> Results = Subsystem->RunQueuedTests();
		for (const UGorgeousInsightMatrixSubsystem::FGorgeousInsightQueuedTestResult& ResultEntry : Results)
		{
			for (const TSharedPtr<FTestRow>& Row : TestItems)
			{
				if (!Row.IsValid())
				{
					continue;
				}
				if (ResultEntry.ProviderName == SelectedProvider && Row->Test.Id == ResultEntry.TestId)
				{
					Row->LastResult = ResultEntry.Result;
					Row->bHasRun = true;
				}
			}
		}
		if (TestsListView.IsValid())
		{
			TestsListView->RebuildList();
		}
	}

	return FReply::Handled();
}

FReply SGorgeousInsightDebugPanel::OnExportStatsCsvClicked()
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		FString FilePath = FPaths::ProjectSavedDir() / TEXT("GorgeousInsightMatrix") / TEXT("InsightMatrix_Stats.csv");
#if WITH_EDITOR
		if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
		{
			TArray<FString> SaveFilenames;
			const void* ParentWindow = FSlateApplication::IsInitialized()
				? FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr)
				: nullptr;
			const bool bSaved = DesktopPlatform->SaveFileDialog(
				ParentWindow,
				TEXT("Export Stats CSV"),
				FPaths::GetPath(FilePath),
				FPaths::GetCleanFilename(FilePath),
				TEXT("CSV Files (*.csv)|*.csv"),
				EFileDialogFlags::None,
				SaveFilenames);
			if (!bSaved || SaveFilenames.IsEmpty())
			{
				return FReply::Handled();
			}
			FilePath = SaveFilenames[0];
		}
#endif
		Subsystem->ExportStatsToCSV(FilePath, SelectedProvider);
	}

	return FReply::Handled();
}

FReply SGorgeousInsightDebugPanel::OnExportStatsJsonClicked()
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		FString FilePath = FPaths::ProjectSavedDir() / TEXT("GorgeousInsightMatrix") / TEXT("InsightMatrix_Stats.json");
#if WITH_EDITOR
		if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
		{
			TArray<FString> SaveFilenames;
			const void* ParentWindow = FSlateApplication::IsInitialized()
				? FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr)
				: nullptr;
			const bool bSaved = DesktopPlatform->SaveFileDialog(
				ParentWindow,
				TEXT("Export Stats JSON"),
				FPaths::GetPath(FilePath),
				FPaths::GetCleanFilename(FilePath),
				TEXT("JSON Files (*.json)|*.json"),
				EFileDialogFlags::None,
				SaveFilenames);
			if (!bSaved || SaveFilenames.IsEmpty())
			{
				return FReply::Handled();
			}
			FilePath = SaveFilenames[0];
		}
#endif
		Subsystem->ExportStatsToJson(FilePath, SelectedProvider);
	}

	return FReply::Handled();
}

void SGorgeousInsightDebugPanel::OnProviderFilterChanged(const FText& InFilterText)
{
	ProviderFilter = InFilterText.ToString();
	RefreshProviders();
	BroadcastStateChanged();
}

void SGorgeousInsightDebugPanel::OnHarnessToggleChanged(ECheckBoxState NewState)
{
	bRunWithHarness = (NewState == ECheckBoxState::Checked);
	BroadcastStateChanged();
}

ECheckBoxState SGorgeousInsightDebugPanel::GetHarnessToggleState() const
{
	return bRunWithHarness ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SGorgeousInsightDebugPanel::ApplyHarnessParameters(TMap<FString, FString>& Parameters) const
{
	Parameters.Add(TEXT("Harness"), bRunWithHarness ? TEXT("true") : TEXT("false"));
	Parameters.Add(TEXT("MultiSession"), bRunWithHarness ? TEXT("true") : TEXT("false"));
}

void SGorgeousInsightDebugPanel::ApplyTestInputParameters(const FGorgeousInsightTest& Test, TMap<FString, FString>& Parameters) const
{
	for (const FGorgeousInsightTest::FGorgeousInsightTestInput& Input : Test.Inputs)
	{
		const FString Value = GetTestInputValue(Test.Id, Input.Id, Input.DefaultValue);
		Parameters.Add(Input.Id.ToString(), Value);
	}
}

void SGorgeousInsightDebugPanel::SetTestInputValue(FName TestId, FName InputId, const FString& Value)
{
	TestInputValues.FindOrAdd(TestId).Add(InputId, Value);
	BroadcastStateChanged();
}

void SGorgeousInsightDebugPanel::SetStatThresholdValue(const FGorgeousInsightStat& Stat, bool bCritical, const FString& Value)
{
	const FName Key = MakeStatThresholdKey(Stat);
	if (Key.IsNone())
	{
		return;
	}

	const FString Trimmed = Value.TrimStartAndEnd();
	if (Trimmed.IsEmpty())
	{
		if (bCritical)
		{
			StatCriticalThresholds.Remove(Key);
		}
		else
		{
			StatWarningThresholds.Remove(Key);
		}
		BroadcastStateChanged();
		return;
	}

	double ParsedValue = 0.0;
	if (LexTryParseString(ParsedValue, *Trimmed))
	{
		if (bCritical)
		{
			StatCriticalThresholds.Add(Key, ParsedValue);
		}
		else
		{
			StatWarningThresholds.Add(Key, ParsedValue);
		}
		BroadcastStateChanged();
	}
}

FString SGorgeousInsightDebugPanel::GetStatThresholdText(const FGorgeousInsightStat& Stat, bool bCritical) const
{
	const FName Key = MakeStatThresholdKey(Stat);
	if (Key.IsNone())
	{
		return FString();
	}

	const TMap<FName, double>& Map = bCritical ? StatCriticalThresholds : StatWarningThresholds;
	if (const double* Found = Map.Find(Key))
	{
		return FString::SanitizeFloat(*Found);
	}
	return FString();
}

FName SGorgeousInsightDebugPanel::MakeStatThresholdKey(const FGorgeousInsightStat& Stat) const
{
	if (SelectedProvider.IsNone() || Stat.Id.IsNone())
	{
		return NAME_None;
	}

	const FString ProviderString = SelectedProvider.GetPlainNameString();
	const FString StatString = Stat.Id.GetPlainNameString();
	if (ProviderString.IsEmpty() || StatString.IsEmpty())
	{
		return NAME_None;
	}

	FString KeyString = ProviderString;
	KeyString.AppendChar(TEXT('.'));
	KeyString.Append(StatString);
	return FName(*KeyString);
}

FSlateColor SGorgeousInsightDebugPanel::GetStatValueColor(const FGorgeousInsightStat& Stat) const
{
	if (SelectedProvider.IsNone() || Stat.Id.IsNone())
	{
		return FSlateColor::UseForeground();
	}

	const FName Key = MakeStatThresholdKey(Stat);
	if (!Key.IsNone())
	{
		const double Value = Stat.NumericValue;
		if (const double* Critical = StatCriticalThresholds.Find(Key))
		{
			if (Value >= *Critical)
			{
				return FSlateColor(FLinearColor(0.95f, 0.2f, 0.2f));
			}
		}
		if (const double* Warning = StatWarningThresholds.Find(Key))
		{
			if (Value >= *Warning)
			{
				return FSlateColor(FLinearColor(0.95f, 0.7f, 0.2f));
			}
		}
	}

	return FSlateColor::UseForeground();
}

const FGorgeousInsightTestResult* SGorgeousInsightDebugPanel::GetBaselineResult(FName ProviderName, FName TestId) const
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		return Subsystem->GetBaselineResult(ProviderName, TestId);
	}
	return nullptr;
}

FText SGorgeousInsightDebugPanel::FormatBaselineText(const FTestRow& Row) const
{
	const FGorgeousInsightTestResult* Baseline = GetBaselineResult(SelectedProvider, Row.Test.Id);
	if (!Baseline)
	{
		return NSLOCTEXT("GorgeousInsightDebugPanel", "BaselineNone", "Baseline: None");
	}

	const FText StatusText = Baseline->bSuccess
		? NSLOCTEXT("GorgeousInsightDebugPanel", "BaselinePass", "Baseline: Pass")
		: NSLOCTEXT("GorgeousInsightDebugPanel", "BaselineFail", "Baseline: Fail");

	return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "BaselineSummary", "{0} | E:{1} W:{2} N:{3}"),
		StatusText,
		FText::AsNumber(Baseline->Errors.Num()),
		FText::AsNumber(Baseline->Warnings.Num()),
		FText::AsNumber(Baseline->Notes.Num()));
}

FLinearColor SGorgeousInsightDebugPanel::GetTestStatusAccent(const FTestRow& Row) const
{
	if (!Row.bHasRun)
	{
		return FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
	}
	if (!Row.LastResult.bSuccess)
	{
		return FLinearColor(0.85f, 0.25f, 0.35f, 1.f);
	}
	if (Row.LastResult.Warnings.Num() > 0)
	{
		return FLinearColor(0.85f, 0.6f, 0.25f, 1.f);
	}
	return FLinearColor(0.25f, 0.8f, 0.45f, 1.f);
}

void SGorgeousInsightDebugPanel::BroadcastStateChanged()
{
	if (!bSuppressStateBroadcast)
	{
		StateChanged.Broadcast(ExportState());
	}
}

FString SGorgeousInsightDebugPanel::GetTestInputValue(FName TestId, FName InputId, const FString& DefaultValue) const
{
	if (const TMap<FName, FString>* TestMap = TestInputValues.Find(TestId))
	{
		if (const FString* Found = TestMap->Find(InputId))
		{
			return *Found;
		}
	}
	return DefaultValue;
}

FText SGorgeousInsightDebugPanel::GetSelectedProviderText() const
{
	if (SelectedProvider.IsNone())
	{
		return NSLOCTEXT("GorgeousInsightDebugPanel", "NoProvider", "No provider selected");
	}

	return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "ProviderLabel", "Provider: {0}"), FText::FromName(SelectedProvider));
}

FText SGorgeousInsightDebugPanel::GetStatsCountText() const
{
	return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "StatsCount", "{0} stats"), FText::AsNumber(StatItems.Num()));
}

FText SGorgeousInsightDebugPanel::GetActionsCountText() const
{
	return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "ActionsCount", "{0} actions"), FText::AsNumber(ActionItems.Num()));
}

FText SGorgeousInsightDebugPanel::GetTestsCountText() const
{
	return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "TestsCount", "{0} tests"), FText::AsNumber(TestItems.Num()));
}

FText SGorgeousInsightDebugPanel::FormatStatValue(const FGorgeousInsightStat& Stat) const
{
	switch (Stat.ValueType)
	{
	case EGorgeousInsightStatValueType::Text:
		return Stat.TextValue.IsEmpty() ? FText::FromString(FString::Printf(TEXT("%.2f"), Stat.NumericValue)) : Stat.TextValue;
	case EGorgeousInsightStatValueType::TimeSeconds:
		return FormatSeconds(Stat.NumericValue);
	case EGorgeousInsightStatValueType::Bytes:
		return FormatBytes(Stat.NumericValue);
	case EGorgeousInsightStatValueType::Percent:
		return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "PercentFormat", "{0}%"), FText::AsNumber(Stat.NumericValue));
	case EGorgeousInsightStatValueType::Number:
	default:
		if (!Stat.Unit.IsEmpty())
		{
			return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "UnitFormat", "{0} {1}"), FText::AsNumber(Stat.NumericValue), Stat.Unit);
		}
		return FText::AsNumber(Stat.NumericValue);
	}
}

FText SGorgeousInsightDebugPanel::FormatTestResultText(const FTestRow& Row) const
{
	if (!Row.bHasRun)
	{
		return NSLOCTEXT("GorgeousInsightDebugPanel", "TestNotRun", "Not run");
	}

	const FText BaseText = Row.LastResult.bSuccess
		? NSLOCTEXT("GorgeousInsightDebugPanel", "TestPassed", "Passed")
		: NSLOCTEXT("GorgeousInsightDebugPanel", "TestFailed", "Failed");

	const FGorgeousInsightTestResult* Baseline = GetBaselineResult(SelectedProvider, Row.Test.Id);
	if (!Baseline)
	{
		return BaseText;
	}

	const int32 DeltaErrors = Row.LastResult.Errors.Num() - Baseline->Errors.Num();
	const int32 DeltaWarnings = Row.LastResult.Warnings.Num() - Baseline->Warnings.Num();
	const int32 DeltaNotes = Row.LastResult.Notes.Num() - Baseline->Notes.Num();
	if (DeltaErrors == 0 && DeltaWarnings == 0 && DeltaNotes == 0)
	{
		return BaseText;
	}

	const FText DeltaErrorsText = FText::FromString(FString::Printf(TEXT("%+d"), DeltaErrors));
	const FText DeltaWarningsText = FText::FromString(FString::Printf(TEXT("%+d"), DeltaWarnings));
	const FText DeltaNotesText = FText::FromString(FString::Printf(TEXT("%+d"), DeltaNotes));

	return FText::Format(
		NSLOCTEXT("GorgeousInsightDebugPanel", "TestResultWithDelta", "{0} (ΔE:{1} ΔW:{2} ΔN:{3})"),
		BaseText,
		DeltaErrorsText,
		DeltaWarningsText,
		DeltaNotesText);
}

FText SGorgeousInsightDebugPanel::FormatProviderBadgeText(const FProviderEntry& Entry) const
{
	return FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "ProviderBadges", "S:{0} A:{1} T:{2}"),
		FText::AsNumber(Entry.StatCount),
		FText::AsNumber(Entry.ActionCount),
		FText::AsNumber(Entry.TestCount));
}

TSharedRef<SWidget> SGorgeousInsightDebugPanel::BuildContextSwitcher()
{
	auto MakeContextButton = [this](const FText& Text, EGorgeousInsightContextMode Mode) -> TSharedRef<SWidget>
	{
		TSharedRef<SCheckBox> CheckBox = SNew(SCheckBox)
			.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
			.Padding(FMargin(12.f, 16.f))
			.IsChecked_Lambda([this, Mode]() { return CurrentContext.Mode == Mode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([this, Mode](ECheckBoxState State) {
				if (State == ECheckBoxState::Checked) {
					CurrentContext.Mode = Mode;
					
					if (Mode == EGorgeousInsightContextMode::Live)
					{
						if (GEngine)
						{
							for (const FWorldContext& Context : GEngine->GetWorldContexts())
							{
								if (Context.WorldType == EWorldType::PIE)
								{
									CurrentContext.WorldContext = Context.World();
									CurrentContext.InstanceName = Context.ContextHandle.ToString();
									break;
								}
							}
						}
					}
					else
					{
						CurrentContext.WorldContext = nullptr;
						CurrentContext.InstanceName.Empty();
					}
					
					RefreshProviderData();
					BroadcastStateChanged();
				}
			})
			[
				SNew(STextBlock)
				.Text(Text)
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
				.Justification(ETextJustify::Center)
			];
			
		if (Mode == EGorgeousInsightContextMode::Live)
		{
			CheckBox->SetEnabled(TAttribute<bool>::CreateLambda([]() {
				if (GEngine) {
					for (const FWorldContext& Context : GEngine->GetWorldContexts()) {
						if (Context.WorldType == EWorldType::PIE) return true;
					}
				}
				return false;
			}));
			
			CheckBox->SetToolTipText(TAttribute<FText>::CreateLambda([]() {
				bool bHasPIE = false;
				if (GEngine) {
					for (const FWorldContext& Context : GEngine->GetWorldContexts()) {
						if (Context.WorldType == EWorldType::PIE) { bHasPIE = true; break; }
					}
				}
				return bHasPIE ? FText::GetEmpty() : NSLOCTEXT("GorgeousInsightDebugPanel", "LiveDisabledTooltip", "Only in PIE we can receive data from this source");
			}));
		}
		
		return CheckBox;
	};

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(4.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeContextButton(FText::FromString(TEXT("Live")), EGorgeousInsightContextMode::Live)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeContextButton(FText::FromString(TEXT("Editor")), EGorgeousInsightContextMode::Editor)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MakeContextButton(FText::FromString(TEXT("Baseline")), EGorgeousInsightContextMode::Baseline)
			]
		];
}

void SGorgeousInsightDebugPanel::OpenBaselineTestResultPopup(const FGorgeousInsightTest& Test, const FGorgeousInsightTestResult& Result)
{
	TSharedPtr<SWindow> PopupWindow = SNew(SWindow)
		.Title(FText::Format(NSLOCTEXT("GorgeousInsightDebugPanel", "BaselineResultsTitle", "Baseline Results: {0}"), Test.DisplayName))
		.ClientSize(FVector2D(450, 400))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.IsPopupWindow(false)
		.SizingRule(ESizingRule::UserSized);

	TSharedPtr<SVerticalBox> MetricsBox;
	
	// Keep track of which metrics the user checks
	TSharedRef<TArray<TSharedPtr<bool>>> SelectionState = MakeShared<TArray<TSharedPtr<bool>>>();
	
	for (int32 i = 0; i < Result.Metrics.Num(); ++i)
	{
		SelectionState->Add(MakeShared<bool>(true));
	}

	TSharedPtr<bool> bJunctionSelected = MakeShared<bool>(false);
	TSharedPtr<FText> JunctionName = MakeShared<FText>(FText::FromString(TEXT("Performance Summary")));

	TSharedRef<SWidget> WindowContent = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(16.f)
		[
			SNew(SVerticalBox)
			
			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "SelectBaselineStats", "Select the metrics to commit to Baseline Stats:"))
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
				.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f))
			]
			
			// Metrics List
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(4.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(MetricsBox, SVerticalBox)
					]
				]
			]
			
			// Junction Options
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 12, 0, 0)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.Padding(8.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 8)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0, 0, 8, 0)
						[
							SNew(SCheckBox)
							.IsChecked_Lambda([bJunctionSelected]() { return *bJunctionSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
							.OnCheckStateChanged_Lambda([bJunctionSelected](ECheckBoxState State) { *bJunctionSelected = (State == ECheckBoxState::Checked); })
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "JunctionSelected", "Junction Selected Metrics"))
							.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0, 0, 8, 0)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "JunctionName", "Junction Name:"))
							.IsEnabled_Lambda([bJunctionSelected]() { return *bJunctionSelected; })
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SEditableTextBox)
							.Text_Lambda([JunctionName]() { return *JunctionName; })
							.OnTextChanged_Lambda([JunctionName](const FText& InText) { *JunctionName = InText; })
							.IsEnabled_Lambda([bJunctionSelected]() { return *bJunctionSelected; })
						]
					]
				]
			]
			
			// Commit Button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 12, 0, 0)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "Commit", "Commit to Baseline"))
				.ContentPadding(FMargin(16.f, 4.f))
				.ButtonStyle(FCoreStyle::Get(), "PrimaryButton")
				.TextStyle(FCoreStyle::Get(), "PrimaryButtonText")
				.OnClicked_Lambda([this, Test, Result, PopupWindow, SelectionState, bJunctionSelected, JunctionName]() {
					if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
					{
						TArray<FGorgeousInsightStat> NewStats;
						
						if (*bJunctionSelected)
						{
							FString JunctionedText;
							int32 Idx = 0;
							bool bFirst = true;
							for (const auto& Pair : Result.Metrics)
							{
								if (*(*SelectionState)[Idx])
								{
									if (!bFirst)
									{
										JunctionedText += TEXT("; ");
									}
									JunctionedText += FString::Printf(TEXT("%s: %s"), *Pair.Key, *Pair.Value);
									bFirst = false;
								}
								Idx++;
							}
							
							if (!JunctionedText.IsEmpty())
							{
								FGorgeousInsightStat JunctionStat;
								FString SafeId = JunctionName->ToString().Replace(TEXT(" "), TEXT(""));
								JunctionStat.Id = FName(*FString::Printf(TEXT("%s.%s"), *Test.Id.ToString(), *SafeId));
								JunctionStat.DisplayName = *JunctionName;
								JunctionStat.Category = Test.Category;
								JunctionStat.ValueType = EGorgeousInsightStatValueType::Text;
								JunctionStat.TextValue = FText::FromString(JunctionedText);
								
								NewStats.Add(JunctionStat);
							}
						}
						else
						{
							int32 Idx = 0;
							for (const auto& Pair : Result.Metrics)
							{
								if (*(*SelectionState)[Idx])
								{
									FGorgeousInsightStat NewStat;
									NewStat.Id = FName(*FString::Printf(TEXT("%s.%s"), *Test.Id.ToString(), *Pair.Key));
									NewStat.DisplayName = FText::FromString(Pair.Key);
									NewStat.Category = Test.Category;
									NewStat.ValueType = EGorgeousInsightStatValueType::Number;
									
									// Try to parse the value
									FString ValStr = Pair.Value;
									ValStr.ReplaceInline(TEXT("ops/sec"), TEXT(""));
									ValStr.ReplaceInline(TEXT("ms"), TEXT(""));
									ValStr.ReplaceInline(TEXT(" "), TEXT(""));
									
									if (ValStr.IsNumeric()) {
										NewStat.NumericValue = FCString::Atod(*ValStr);
									} else {
										NewStat.ValueType = EGorgeousInsightStatValueType::Text;
										NewStat.TextValue = FText::FromString(Pair.Value);
									}
									
									NewStats.Add(NewStat);
								}
								Idx++;
							}
						}
						
						if (NewStats.Num() > 0)
						{
							Subsystem->AddBaselineStats(SelectedProvider, NewStats);
							RefreshProviderData();
						}
					}
					PopupWindow->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
		];

	int32 RowIdx = 0;
	for (const auto& Pair : Result.Metrics)
	{
		TSharedPtr<bool> bSelected = (*SelectionState)[RowIdx];
		
		const FSlateColor RowColor = (RowIdx % 2 == 0) ? FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.5f)) : FSlateColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.5f));
		
		MetricsBox->AddSlot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(RowColor)
			.Padding(FMargin(8.f, 6.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 12, 0)
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([bSelected]() { return *bSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([bSelected](ECheckBoxState State) { *bSelected = (State == ECheckBoxState::Checked); })
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%s: %s"), *Pair.Key, *Pair.Value)))
				]
			]
		];
		
		RowIdx++;
	}

	PopupWindow->SetContent(WindowContent);
	FSlateApplication::Get().AddWindow(PopupWindow.ToSharedRef());
}
