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
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

class SGorgeousInsightMiniTable : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousInsightMiniTable) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetRows(const TArray<FGorgeousInsightTableRow>& InRows, const FText& InTitle, const FText& InSubtitle);

private:
	void OnSort(EColumnSortPriority::Type, const FName& ColumnId, EColumnSortMode::Type NewMode);
	EColumnSortMode::Type GetSortMode(const FName& ColumnId) const;
	void SortRows();

	TArray<TSharedPtr<FGorgeousInsightTableRow>> Rows;
	TSharedPtr<SListView<TSharedPtr<FGorgeousInsightTableRow>>> ListView;
	FName SortColumn;
	EColumnSortMode::Type SortMode = EColumnSortMode::Descending;
	FText Title;
	FText Subtitle;
};