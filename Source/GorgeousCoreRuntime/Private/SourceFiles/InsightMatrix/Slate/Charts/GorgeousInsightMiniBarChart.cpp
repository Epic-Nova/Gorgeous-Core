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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniBarChart.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniBarChart::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
}

void SGorgeousInsightMiniBarChart::SetValues(const TArray<FGorgeousInsightBarValue>& InValues, const FText& InTitle, const FText& InSubtitle)
{
	Values = InValues;
	Title = InTitle;
	Subtitle = InSubtitle;
}

FVector2D SGorgeousInsightMiniBarChart::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniBarChart::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
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

	if (Values.IsEmpty() || ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	double MaxValue = 0.0;
	for (const FGorgeousInsightBarValue& Value : Values)
	{
		MaxValue = FMath::Max(MaxValue, FMath::Abs(Value.Value));
	}
	if (MaxValue <= 0.0)
	{
		MaxValue = 1.0;
	}

	const float BarSpacing = 6.f;
	const float BarWidth = (ChartSize.X - BarSpacing * (Values.Num() - 1)) / FMath::Max(1, Values.Num());
	const int32 LabelLayer = LayerId + 1;
	float X = ChartOrigin.X;
	for (const FGorgeousInsightBarValue& Value : Values)
	{
		const float Normalized = static_cast<float>(FMath::Abs(Value.Value) / MaxValue);
		const float BarHeight = ChartSize.Y * Normalized;
		const FVector2D BarPos(X, ChartOrigin.Y + (ChartSize.Y - BarHeight));
		const FVector2D BarSize(FMath::Max(1.f, BarWidth), FMath::Max(1.f, BarHeight));
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(BarSize), FSlateLayoutTransform(FVector2f(BarPos))),
			WhiteBrush,
			ESlateDrawEffect::None,
			FLinearColor(0.35f, 0.85f, 0.35f, 1.f));
		const FText ValueText = FText::AsNumber(Value.Value, &FNumberFormattingOptions().SetMaximumFractionalDigits(2));
		const FVector2D TextPos(BarPos.X, FMath::Max(ChartOrigin.Y, BarPos.Y - 12.f));
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LabelLayer,
			AllottedGeometry.ToPaintGeometry(FVector2f(ChartSize), FSlateLayoutTransform(FVector2f(TextPos))),
			ValueText,
			SubtitleFont,
			ESlateDrawEffect::None,
			FLinearColor(0.9f, 0.9f, 0.9f, 1.f));
		X += BarWidth + BarSpacing;
	}

	return LayerId + 2;
}
