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
#include "Widgets/SLeafWidget.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

class SGorgeousInsightMiniScatterPlot : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousInsightMiniScatterPlot) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetValues(const TArray<FGorgeousInsightScatterPoint>& InPoints, const FText& InTitle, const FText& InSubtitle);

	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TArray<FGorgeousInsightScatterPoint> Points;
	FText Title;
	FText Subtitle;
};