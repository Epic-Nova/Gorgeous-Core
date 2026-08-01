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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniPieChart.h"

#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniPieChart::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
	bDonut = false;
}

void SGorgeousInsightMiniPieChart::SetValues(const TArray<FGorgeousInsightPieSlice>& InSlices, const FText& InTitle, const FText& InSubtitle, bool bInDonut)
{
	Slices = InSlices;
	Title = InTitle;
	Subtitle = InSubtitle;
	bDonut = bInDonut;
}

FVector2D SGorgeousInsightMiniPieChart::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniPieChart::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
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

	if (Slices.IsEmpty() || ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	double TotalValue = 0.0;
	for (const FGorgeousInsightPieSlice& Slice : Slices)
	{
		TotalValue += FMath::Max(0.0, Slice.Value);
	}
	if (TotalValue <= 0.0)
	{
		return LayerId;
	}

	const FVector2D Center = ChartOrigin + (ChartSize * 0.5f);
	const float OuterRadius = 0.5f * FMath::Min(ChartSize.X, ChartSize.Y);
	const float InnerRadius = bDonut ? OuterRadius * 0.55f : 0.f;
	const FSlateRenderTransform LocalRenderTransform = AllottedGeometry.ToPaintGeometry().GetAccumulatedRenderTransform();
	const FSlateResourceHandle ResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*WhiteBrush);
	const FSlateShaderResourceProxy* ResourceProxy = ResourceHandle.GetResourceProxy();
	const FVector2f AtlasUV = ResourceProxy ? FVector2f(ResourceProxy->StartUV) : FVector2f(0.f, 0.f);
	const FVector2f AtlasUVSize = ResourceProxy ? FVector2f(ResourceProxy->SizeUV) : FVector2f(1.f, 1.f);
	const FVector2f VertexUV = AtlasUV + (AtlasUVSize * 0.5f);

	TArray<FSlateVertex> Verts;
	TArray<SlateIndex> Indices;
	Verts.Reserve(256);
	Indices.Reserve(512);

	auto AppendRingSegment = [&](float StartAngle, float EndAngle, const FLinearColor& Color, int32 Steps)
	{
		const int32 StartIndex = Verts.Num();
		for (int32 Step = 0; Step <= Steps; ++Step)
		{
			const float Alpha = static_cast<float>(Step) / static_cast<float>(Steps);
			const float Angle = FMath::Lerp(StartAngle, EndAngle, Alpha);
			const FVector2f OuterPos = FVector2f(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * OuterRadius);
			const FVector2f InnerPos = FVector2f(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * InnerRadius);
			const FColor VertexColor = Color.ToFColor(true);
			Verts.Add(FSlateVertex::Make(LocalRenderTransform, OuterPos, VertexUV, VertexColor));
			Verts.Add(FSlateVertex::Make(LocalRenderTransform, InnerPos, VertexUV, VertexColor));
		}

		for (int32 Step = 0; Step < Steps; ++Step)
		{
			const int32 BaseIndex = StartIndex + Step * 2;
			Indices.Add(BaseIndex);
			Indices.Add(BaseIndex + 1);
			Indices.Add(BaseIndex + 2);
			Indices.Add(BaseIndex + 2);
			Indices.Add(BaseIndex + 1);
			Indices.Add(BaseIndex + 3);
		}
	};

	float AngleCursor = -PI * 0.5f;
	for (const FGorgeousInsightPieSlice& Slice : Slices)
	{
		const double SliceValue = FMath::Max(0.0, Slice.Value);
		if (SliceValue <= 0.0)
		{
			continue;
		}
		const float SliceAngle = static_cast<float>((SliceValue / TotalValue) * 2.0 * PI);
		const float EndAngle = AngleCursor + SliceAngle;
		const int32 Steps = FMath::Clamp(FMath::RoundToInt(24.f * SliceAngle / (2.f * PI)), 4, 48);
		AppendRingSegment(AngleCursor, EndAngle, Slice.Color, Steps);
		AngleCursor = EndAngle;
	}

	FSlateDrawElement::MakeCustomVerts(
		OutDrawElements,
		LayerId++,
		ResourceHandle,
		Verts,
		Indices,
		nullptr,
		0,
		0);

	AngleCursor = -PI * 0.5f;
	for (const FGorgeousInsightPieSlice& Slice : Slices)
	{
		const double SliceValue = FMath::Max(0.0, Slice.Value);
		if (SliceValue <= 0.0)
		{
			continue;
		}
		const float SliceAngle = static_cast<float>((SliceValue / TotalValue) * 2.0 * PI);
		const float MidAngle = AngleCursor + SliceAngle * 0.5f;
		const FVector2D Direction(FMath::Cos(MidAngle), FMath::Sin(MidAngle));
		const FVector2D EdgePos = Center + Direction * OuterRadius;
		const FVector2D LabelAnchor = Center + Direction * (OuterRadius + 12.f);
		const float HorizontalOffset = Direction.X >= 0.f ? 28.f : -28.f;
		const FVector2D LabelPos = LabelAnchor + FVector2D(HorizontalOffset, 0.f);
		const FText LabelText = FText::Format(NSLOCTEXT("GorgeousInsightMiniPieChart", "SliceLabel", "{0}: {1}"),
			Slice.Label,
			FText::AsNumber(Slice.Value, &FNumberFormattingOptions().SetMaximumFractionalDigits(2)));
		TArray<FVector2D> LeaderLine;
		LeaderLine.Add(EdgePos);
		LeaderLine.Add(LabelAnchor);
		LeaderLine.Add(LabelPos);
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			LeaderLine,
			ESlateDrawEffect::None,
			FLinearColor(0.85f, 0.85f, 0.85f, 0.8f),
			true,
			1.f);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(FVector2f(ChartSize), FSlateLayoutTransform(FVector2f(LabelPos))),
			LabelText,
			LabelFont,
			ESlateDrawEffect::None,
			FLinearColor(0.95f, 0.95f, 0.95f, 1.f));
		AngleCursor += SliceAngle;
	}

	return LayerId + 1;
}
