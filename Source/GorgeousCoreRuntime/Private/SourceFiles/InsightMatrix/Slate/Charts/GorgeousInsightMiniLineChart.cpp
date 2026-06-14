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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniLineChart.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniLineChart::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
}

void SGorgeousInsightMiniLineChart::SetValues(const TArray<double>& InValues, const FText& InTitle, const FText& InSubtitle)
{
	Values = InValues;
	Title = InTitle;
	Subtitle = InSubtitle;
}

FVector2D SGorgeousInsightMiniLineChart::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniLineChart::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
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

	if (Values.Num() < 2 || ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	double MinValue = Values[0];
	double MaxValue = Values[0];
	for (double Value : Values)
	{
		MinValue = FMath::Min(MinValue, Value);
		MaxValue = FMath::Max(MaxValue, Value);
	}
	if (FMath::IsNearlyEqual(MinValue, MaxValue))
	{
		MaxValue = MinValue + 1.0;
	}

	TArray<FVector2D> Points;
	Points.Reserve(Values.Num());
	const float StepX = ChartSize.X / FMath::Max(1, Values.Num() - 1);
	for (int32 Index = 0; Index < Values.Num(); ++Index)
	{
		const double Normalized = (Values[Index] - MinValue) / (MaxValue - MinValue);
		const float X = ChartOrigin.X + StepX * Index;
		const float Y = ChartOrigin.Y + ChartSize.Y * (1.f - static_cast<float>(Normalized));
		Points.Add(FVector2D(X, Y));
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		FLinearColor(0.22f, 0.65f, 1.0f, 1.f),
		true,
		2.f);

	for (int32 Index = 0; Index < Points.Num(); ++Index)
	{
		const FText ValueText = FText::AsNumber(Values[Index], &FNumberFormattingOptions().SetMaximumFractionalDigits(3));
		const FVector2D TextPos = Points[Index] + FVector2D(2.f, -10.f);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(ChartSize), FSlateLayoutTransform(FVector2f(TextPos))),
			ValueText,
			LabelFont,
			ESlateDrawEffect::None,
			FLinearColor(0.9f, 0.9f, 0.9f, 1.f));
	}

	return LayerId + 1;
}
