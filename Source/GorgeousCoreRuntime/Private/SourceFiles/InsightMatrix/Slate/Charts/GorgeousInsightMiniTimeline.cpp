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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniTimeline.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniTimeline::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
}

void SGorgeousInsightMiniTimeline::SetSegments(const TArray<FGorgeousInsightTimelineSegment>& InSegments, const FText& InTitle, const FText& InSubtitle)
{
	Segments = InSegments;
	Title = InTitle;
	Subtitle = InSubtitle;
}

FVector2D SGorgeousInsightMiniTimeline::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniTimeline::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
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

	if (Segments.IsEmpty() || ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	const float RowHeight = ChartSize.Y / static_cast<float>(Segments.Num());
	for (int32 Index = 0; Index < Segments.Num(); ++Index)
	{
		const FGorgeousInsightTimelineSegment& Segment = Segments[Index];
		const float BarX = ChartOrigin.X + ChartSize.X * Segment.Start;
		const float BarWidth = ChartSize.X * Segment.Duration;
		const float BarY = ChartOrigin.Y + RowHeight * Index + 2.f;
		const FVector2D BarSize(FMath::Max(1.f, BarWidth), FMath::Max(1.f, RowHeight - 4.f));
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(BarSize), FSlateLayoutTransform(FVector2f(BarX, BarY))),
			WhiteBrush,
			ESlateDrawEffect::None,
			Segment.Color);
		const float Luminance = 0.2126f * Segment.Color.R + 0.7152f * Segment.Color.G + 0.0722f * Segment.Color.B;
		const bool bLightBackground = Luminance > 0.58f;
		const FLinearColor TextColor = bLightBackground
			? FLinearColor(0.08f, 0.08f, 0.08f, 1.f)
			: FLinearColor(0.96f, 0.96f, 0.96f, 1.f);
		const FLinearColor ShadowColor = bLightBackground
			? FLinearColor(1.f, 1.f, 1.f, 0.25f)
			: FLinearColor(0.f, 0.f, 0.f, 0.45f);
		const FText SegmentLabel = FText::Format(NSLOCTEXT("GorgeousInsightMiniTimeline", "TimelineLabel", "{0} ({1}s)"),
			Segment.Label,
			FText::AsNumber(Segment.Duration, &FNumberFormattingOptions().SetMaximumFractionalDigits(2)));
		const FVector2D LabelPos(BarX + 4.f, BarY + 2.f);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(FVector2f(ChartSize), FSlateLayoutTransform(FVector2f(LabelPos + FVector2D(1.f, 1.f)))),
			SegmentLabel,
			LabelFont,
			ESlateDrawEffect::None,
			ShadowColor);

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(FVector2f(ChartSize), FSlateLayoutTransform(FVector2f(LabelPos))),
			SegmentLabel,
			LabelFont,
			ESlateDrawEffect::None,
			TextColor);
	}

	return LayerId + 2;
}
