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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniTable.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

namespace
{
	const FName ColumnMiniTableName(TEXT("MiniTableName"));
	const FName ColumnMiniTableValue(TEXT("MiniTableValue"));
	const FName ColumnMiniTableCategory(TEXT("MiniTableCategory"));
}

class SGorgeousInsightMiniTableRow : public SMultiColumnTableRow<TSharedPtr<FGorgeousInsightTableRow>>
{
public:
	SLATE_BEGIN_ARGS(SGorgeousInsightMiniTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FGorgeousInsightTableRow>, RowData)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		RowData = InArgs._RowData;
		SMultiColumnTableRow<TSharedPtr<FGorgeousInsightTableRow>>::Construct(
			FSuperRowType::FArguments().Padding(FMargin(4.f, 2.f)),
			InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!RowData.IsValid())
		{
			return SNew(STextBlock).Text(FText::GetEmpty());
		}

		if (ColumnName == ColumnMiniTableName)
		{
			return SNew(STextBlock).Text(RowData->Name);
		}
		if (ColumnName == ColumnMiniTableValue)
		{
			return SNew(STextBlock).Text(RowData->Value);
		}
		if (ColumnName == ColumnMiniTableCategory)
		{
			return SNew(STextBlock).Text(RowData->Category);
		}

		return SNew(STextBlock).Text(FText::GetEmpty());
	}

private:
	TSharedPtr<FGorgeousInsightTableRow> RowData;
};

void SGorgeousInsightMiniTable::Construct(const FArguments& InArgs)
{
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
	SortColumn = ColumnMiniTableValue;
	SortMode = EColumnSortMode::Descending;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(6.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return Title; })
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 2.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return Subtitle; })
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(ListView, SListView<TSharedPtr<FGorgeousInsightTableRow>>)
				.ListItemsSource(&Rows)
				.SelectionMode(ESelectionMode::None)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(ColumnMiniTableName)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "MiniTableName", "Name"))
						.FillWidth(0.5f)
						.SortMode_Lambda([this]() { return GetSortMode(ColumnMiniTableName); })
						.OnSort(this, &SGorgeousInsightMiniTable::OnSort)
					+ SHeaderRow::Column(ColumnMiniTableValue)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "MiniTableValue", "Value"))
						.FillWidth(0.3f)
						.SortMode_Lambda([this]() { return GetSortMode(ColumnMiniTableValue); })
						.OnSort(this, &SGorgeousInsightMiniTable::OnSort)
					+ SHeaderRow::Column(ColumnMiniTableCategory)
						.DefaultLabel(NSLOCTEXT("GorgeousInsightDebugPanel", "MiniTableCategory", "Category"))
						.FillWidth(0.2f)
						.SortMode_Lambda([this]() { return GetSortMode(ColumnMiniTableCategory); })
						.OnSort(this, &SGorgeousInsightMiniTable::OnSort)
					)
				.OnGenerateRow_Lambda([](TSharedPtr<FGorgeousInsightTableRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SGorgeousInsightMiniTableRow, OwnerTable).RowData(Item);
				})
			]
		]
	];
}

void SGorgeousInsightMiniTable::SetRows(const TArray<FGorgeousInsightTableRow>& InRows, const FText& InTitle, const FText& InSubtitle)
{
	Rows.Reset();
	for (const FGorgeousInsightTableRow& Row : InRows)
	{
		Rows.Add(MakeShared<FGorgeousInsightTableRow>(Row));
	}
	Title = InTitle;
	Subtitle = InSubtitle;
	SortRows();
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

void SGorgeousInsightMiniTable::OnSort(EColumnSortPriority::Type, const FName& ColumnId, EColumnSortMode::Type NewMode)
{
	SortColumn = ColumnId;
	SortMode = NewMode;
	SortRows();
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

EColumnSortMode::Type SGorgeousInsightMiniTable::GetSortMode(const FName& ColumnId) const
{
	return ColumnId == SortColumn ? SortMode : EColumnSortMode::None;
}

void SGorgeousInsightMiniTable::SortRows()
{
	if (SortMode == EColumnSortMode::None)
	{
		return;
	}

	Rows.Sort([this](const TSharedPtr<FGorgeousInsightTableRow>& A, const TSharedPtr<FGorgeousInsightTableRow>& B)
	{
		if (!A.IsValid() || !B.IsValid())
		{
			return false;
		}
		int32 Result = 0;
		if (SortColumn == ColumnMiniTableName)
		{
			Result = A->Name.ToString().Compare(B->Name.ToString());
		}
		else if (SortColumn == ColumnMiniTableCategory)
		{
			Result = A->Category.ToString().Compare(B->Category.ToString());
		}
		else
		{
			Result = A->Value.ToString().Compare(B->Value.ToString());
		}
		return SortMode == EColumnSortMode::Ascending ? Result < 0 : Result > 0;
	});
}
