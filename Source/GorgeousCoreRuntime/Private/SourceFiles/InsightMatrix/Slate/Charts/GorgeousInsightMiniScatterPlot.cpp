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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniScatterPlot.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniScatterPlot::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
}

void SGorgeousInsightMiniScatterPlot::SetValues(const TArray<FGorgeousInsightScatterPoint>& InPoints, const FText& InTitle, const FText& InSubtitle)
{
	Points = InPoints;
	Title = InTitle;
	Subtitle = InSubtitle;
}

FVector2D SGorgeousInsightMiniScatterPlot::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniScatterPlot::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(),
		WhiteBrush,
		ESlateDrawEffect::None,
		FLinearColor(0.04f, 0.04f, 0.04f, 1.f));

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const float HeaderHeight = 32.f;
	const FVector2D ChartOrigin(8.f, HeaderHeight);
	const FVector2D ChartSize(FMath::Max(1.f, Size.X - 16.f), FMath::Max(1.f, Size.Y - HeaderHeight - 8.f));

	const FSlateFontInfo TitleFont = FCoreStyle::Get().GetFontStyle("BoldFont");
	const FSlateFontInfo SubtitleFont = FCoreStyle::Get().GetFontStyle("SmallFont");
 	const FSlateFontInfo LabelFont = FCoreStyle::Get().GetFontStyle("SmallFont");

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(FVector2f(Size), FSlateLayoutTransform(FVector2f(8.f, 6.f))),
		Title,
		TitleFont,
		ESlateDrawEffect::None,
		FLinearColor::White);

	if (!Subtitle.IsEmpty())
	{
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(FVector2f(Size), FSlateLayoutTransform(FVector2f(8.f, 18.f))),
			Subtitle,
			SubtitleFont,
			ESlateDrawEffect::None,
			FLinearColor(0.7f, 0.7f, 0.7f, 1.f));
	}

	if (ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	TArray<FVector2D> Axis;
	Axis.Add(ChartOrigin + FVector2D(0.f, ChartSize.Y));
	Axis.Add(ChartOrigin + FVector2D(ChartSize.X, ChartSize.Y));
	Axis.Add(ChartOrigin + FVector2D(0.f, ChartSize.Y));
	Axis.Add(ChartOrigin + FVector2D(0.f, 0.f));
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(),
		Axis,
		ESlateDrawEffect::None,
		FLinearColor(0.25f, 0.25f, 0.25f, 1.f),
		true,
		1.f);

	for (const FGorgeousInsightScatterPoint& Point : Points)
	{
		const float X = ChartOrigin.X + ChartSize.X * Point.Position.X;
		const float Y = ChartOrigin.Y + ChartSize.Y * (1.f - Point.Position.Y);
		const FVector2D DotSize(Point.Radius * 2.f, Point.Radius * 2.f);
		const FVector2D DotPos(X - Point.Radius, Y - Point.Radius);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(DotSize), FSlateLayoutTransform(FVector2f(DotPos))),
			WhiteBrush,
			ESlateDrawEffect::None,
			Point.Color);
		const FText LabelText = FText::Format(NSLOCTEXT("GorgeousInsightMiniScatterPlot", "ScatterLabel", "{0}, {1}"),
			FText::AsNumber(Point.Position.X, &FNumberFormattingOptions().SetMaximumFractionalDigits(2)),
			FText::AsNumber(Point.Position.Y, &FNumberFormattingOptions().SetMaximumFractionalDigits(2)));
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(FVector2f(ChartSize), FSlateLayoutTransform(FVector2f(X + Point.Radius + 2.f, Y - 10.f))),
			LabelText,
			LabelFont,
			ESlateDrawEffect::None,
			FLinearColor(0.9f, 0.9f, 0.9f, 1.f));
	}

	return LayerId + 2;
}
