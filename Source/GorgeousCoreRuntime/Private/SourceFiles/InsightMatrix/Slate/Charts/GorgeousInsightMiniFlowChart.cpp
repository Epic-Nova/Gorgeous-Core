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

#include "InsightMatrix/Slate/Charts/GorgeousInsightMiniFlowChart.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SGorgeousInsightMiniFlowChart::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);
	Title = FText::GetEmpty();
	Subtitle = FText::GetEmpty();
}

void SGorgeousInsightMiniFlowChart::SetGraph(const TArray<FGorgeousInsightFlowNode>& InNodes, const TArray<FGorgeousInsightFlowEdge>& InEdges, const FText& InTitle, const FText& InSubtitle)
{
	Nodes = InNodes;
	Edges = InEdges;
	Title = InTitle;
	Subtitle = InSubtitle;
}

FVector2D SGorgeousInsightMiniFlowChart::ComputeDesiredSize(float) const
{
	return FVector2D(240.f, 120.f);
}

int32 SGorgeousInsightMiniFlowChart::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
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
	const FSlateFontInfo NodeFont = FCoreStyle::Get().GetFontStyle("SmallFont");

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

	if (Nodes.IsEmpty() || ChartSize.X <= 1.f || ChartSize.Y <= 1.f)
	{
		return LayerId;
	}

	TArray<FVector2D> NodeCenters;
	NodeCenters.Reserve(Nodes.Num());
	for (const FGorgeousInsightFlowNode& Node : Nodes)
	{
		const FVector2D NodeSize(ChartSize.X * Node.Size.X, ChartSize.Y * Node.Size.Y);
		const FVector2D NodePos = ChartOrigin + FVector2D(ChartSize.X * Node.Position.X, ChartSize.Y * Node.Position.Y);
		NodeCenters.Add(NodePos + NodeSize * 0.5f);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(NodeSize), FSlateLayoutTransform(FVector2f(NodePos))),
			WhiteBrush,
			ESlateDrawEffect::None,
			Node.Color);
		const float Luminance = 0.2126f * Node.Color.R + 0.7152f * Node.Color.G + 0.0722f * Node.Color.B;
		const bool bLightBackground = Luminance > 0.58f;
		const FLinearColor TextColor = bLightBackground
			? FLinearColor(0.08f, 0.08f, 0.08f, 1.f)
			: FLinearColor(0.96f, 0.96f, 0.96f, 1.f);
		const FLinearColor ShadowColor = bLightBackground
			? FLinearColor(1.f, 1.f, 1.f, 0.25f)
			: FLinearColor(0.f, 0.f, 0.f, 0.45f);
		const FVector2D LabelPos = NodePos + FVector2D(4.f, 2.f);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(NodeSize), FSlateLayoutTransform(FVector2f(LabelPos + FVector2D(1.f, 1.f)))),
			Node.Label,
			NodeFont,
			ESlateDrawEffect::None,
			ShadowColor);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(NodeSize), FSlateLayoutTransform(FVector2f(LabelPos))),
			Node.Label,
			NodeFont,
			ESlateDrawEffect::None,
			TextColor);
	}

	for (const FGorgeousInsightFlowEdge& Edge : Edges)
	{
		if (!NodeCenters.IsValidIndex(Edge.From) || !NodeCenters.IsValidIndex(Edge.To))
		{
			continue;
		}
		TArray<FVector2D> Points;
		Points.Add(NodeCenters[Edge.From]);
		Points.Add(NodeCenters[Edge.To]);
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(),
			Points,
			ESlateDrawEffect::None,
			Edge.Color,
			true,
			2.f);
	}

	return LayerId + 3;
}
