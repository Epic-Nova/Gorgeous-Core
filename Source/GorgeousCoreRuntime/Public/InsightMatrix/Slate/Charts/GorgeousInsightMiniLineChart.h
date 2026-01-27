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
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "Widgets/SLeafWidget.h"

class SGorgeousInsightMiniLineChart : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousInsightMiniLineChart) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetValues(const TArray<double>& InValues, const FText& InTitle, const FText& InSubtitle);

	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TArray<double> Values;
	FText Title;
	FText Subtitle;
};
