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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniHistogram.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniHistogram::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
}

void SGorgeousInsightMiniHistogram::SetValues(const TArray<int32>& InBins, const FText& InTitle, const FText& InSubtitle)
{
	Bins = InBins;
	Title = InTitle;
	Subtitle = InSubtitle;
}

FVector2D SGorgeousInsightMiniHistogram::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniHistogram::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
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

	if (Bins.IsEmpty() || ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	int32 MaxBin = 1;
	for (int32 Count : Bins)
	{
		MaxBin = FMath::Max(MaxBin, Count);
	}

	const float BarSpacing = 4.f;
	const float BarWidth = (ChartSize.X - BarSpacing * (Bins.Num() - 1)) / FMath::Max(1, Bins.Num());
	const int32 LabelLayer = LayerId + 1;
	float X = ChartOrigin.X;
	for (int32 Count : Bins)
	{
		const float Normalized = static_cast<float>(Count) / static_cast<float>(MaxBin);
		const float BarHeight = ChartSize.Y * Normalized;
		const FVector2D BarPos(X, ChartOrigin.Y + (ChartSize.Y - BarHeight));
		const FVector2D BarSize(FMath::Max(1.f, BarWidth), FMath::Max(1.f, BarHeight));
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(BarSize), FSlateLayoutTransform(FVector2f(BarPos))),
			WhiteBrush,
			ESlateDrawEffect::None,
			FLinearColor(0.85f, 0.6f, 0.25f, 1.f));
		const FVector2D TextPos(BarPos.X, FMath::Max(ChartOrigin.Y, BarPos.Y - 12.f));
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LabelLayer,
			AllottedGeometry.ToPaintGeometry(FVector2f(ChartSize), FSlateLayoutTransform(FVector2f(TextPos))),
			FText::AsNumber(Count),
			SubtitleFont,
			ESlateDrawEffect::None,
			FLinearColor(0.9f, 0.9f, 0.9f, 1.f));
		X += BarWidth + BarSpacing;
	}

	return LayerId + 2;
}
