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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniHeatmap.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniHeatmap::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
	Rows = 0;
	Columns = 0;
}

void SGorgeousInsightMiniHeatmap::SetValues(const TArray<float>& InValues, int32 InRows, int32 InColumns, const FText& InTitle, const FText& InSubtitle)
{
	Values = InValues;
	Rows = InRows;
	Columns = InColumns;
	Title = InTitle;
	Subtitle = InSubtitle;
}

FVector2D SGorgeousInsightMiniHeatmap::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniHeatmap::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
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

	if (Rows <= 0 || Columns <= 0 || ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	const float CellWidth = ChartSize.X / static_cast<float>(Columns);
	const float CellHeight = ChartSize.Y / static_cast<float>(Rows);
	float MaxValue = 1.f;
	for (float Value : Values)
	{
		MaxValue = FMath::Max(MaxValue, Value);
	}

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Col = 0; Col < Columns; ++Col)
		{
			const int32 Index = Row * Columns + Col;
			const float Value = Values.IsValidIndex(Index) ? Values[Index] : 0.f;
			const float Normalized = FMath::Clamp(Value / MaxValue, 0.f, 1.f);
			const FLinearColor CellColor = FLinearColor::LerpUsingHSV(
				FLinearColor(0.1f, 0.2f, 0.3f, 1.f),
				FLinearColor(0.9f, 0.2f, 0.2f, 1.f),
				Normalized);
			const FVector2D CellPos(ChartOrigin.X + Col * CellWidth, ChartOrigin.Y + Row * CellHeight);
			const FVector2D CellSize(FMath::Max(1.f, CellWidth - 2.f), FMath::Max(1.f, CellHeight - 2.f));
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(CellSize), FSlateLayoutTransform(FVector2f(CellPos))),
				WhiteBrush,
				ESlateDrawEffect::None,
				CellColor);
			const float Luminance = 0.2126f * CellColor.R + 0.7152f * CellColor.G + 0.0722f * CellColor.B;
			const FLinearColor TextColor = Luminance > 0.6f
				? FLinearColor(0.08f, 0.08f, 0.08f, 1.f)
				: FLinearColor(0.95f, 0.95f, 0.95f, 1.f);
			const FText ValueText = FText::AsNumber(Value, &FNumberFormattingOptions().SetMaximumFractionalDigits(2));
			const FVector2D TextPos(CellPos.X + 2.f, CellPos.Y + 2.f);
			FSlateDrawElement::MakeText(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(CellSize), FSlateLayoutTransform(FVector2f(TextPos))),
				ValueText,
				LabelFont,
				ESlateDrawEffect::None,
				TextColor);
		}
	}

	return LayerId + 2;
}
