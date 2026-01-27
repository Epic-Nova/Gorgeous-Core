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

/** Basic stat value type hints for the UI layer. */
enum class EGorgeousInsightStatValueType : uint8
{
	Number,
	TimeSeconds,
	Bytes,
	Percent,
	Text
};

/** Lightweight stat snapshot used by the panel. */
struct FGorgeousInsightStat
{
	FName Id;
	FText DisplayName;
	FText Description;
	FName Category;
	EGorgeousInsightStatValueType ValueType = EGorgeousInsightStatValueType::Number;
	double NumericValue = 0.0;
	FText TextValue;
	FText Unit;
};

/** Action descriptor exposed by providers. */
struct FGorgeousInsightAction
{
	FName Id;
	FText DisplayName;
	FText Description;
	FName Category;
	bool bIsToggle = false;
	bool bDefaultToggleState = false;
};

/** Test descriptor exposed by providers. */
struct FGorgeousInsightTest
{
	FName Id;
	FText DisplayName;
	FText Description;
	FName Category;
	TArray<FName> Tags;

	/** Optional input definitions for dynamic test parameters. */
	struct FGorgeousInsightTestInput
	{
		FName Id;
		FText DisplayName;
		enum class EGorgeousInsightTestInputType : uint8
		{
			String,
			Number,
			Bool,
			Class
		} Type = EGorgeousInsightTestInputType::String;

		FString DefaultValue;
		bool bRequired = false;
		/** Optional allowed base class path for Class inputs. */
		FString AllowedClassPath;
	};

	TArray<FGorgeousInsightTestInput> Inputs;
};

/** Action execution context. */
struct FGorgeousInsightActionContext
{
	UObject* WorldContextObject = nullptr;
	TMap<FString, FString> Parameters;
};

/** Test execution context. */
struct FGorgeousInsightTestContext
{
	UObject* WorldContextObject = nullptr;
	TMap<FString, FString> Parameters;
};

/** Test result for panel + automation integration. */
struct FGorgeousInsightTestResult
{
	bool bSuccess = true;
	TArray<FString> Errors;
	TArray<FString> Warnings;
	TArray<FString> Notes;
	TMap<FString, FString> Metrics;
};

/** Shared chart value types for provider-defined charts. */
struct FGorgeousInsightBarValue
{
	FText Label;
	double Value = 0.0;
};

struct FGorgeousInsightPieSlice
{
	FText Label;
	double Value = 0.0;
	FLinearColor Color = FLinearColor::White;
};

struct FGorgeousInsightScatterPoint
{
	FVector2D Position = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	float Radius = 3.f;
};

struct FGorgeousInsightTimelineSegment
{
	FText Label;
	float Start = 0.f;
	float Duration = 0.f;
	FLinearColor Color = FLinearColor::White;
};

struct FGorgeousInsightFlowNode
{
	FText Label;
	FVector2D Position = FVector2D::ZeroVector;
	FVector2D Size = FVector2D(0.2f, 0.18f);
	FLinearColor Color = FLinearColor(0.2f, 0.4f, 0.7f, 1.f);
};

struct FGorgeousInsightFlowEdge
{
	int32 From = INDEX_NONE;
	int32 To = INDEX_NONE;
	FLinearColor Color = FLinearColor(0.7f, 0.7f, 0.7f, 1.f);
};

struct FGorgeousInsightTableRow
{
	FText Name;
	FText Value;
	FText Category;
};

enum class EGorgeousInsightChartType : uint8
{
	Bar,
	Line,
	Histogram,
	Pie,
	Scatter,
	Heatmap,
	Timeline,
	Flow,
	Table
};

struct FGorgeousInsightChartDefinition
{
	FName Id = NAME_None;
	EGorgeousInsightChartType Type = EGorgeousInsightChartType::Bar;
	FText Title;
	FText Subtitle;

	TArray<FGorgeousInsightBarValue> Bars;
	TArray<double> LineValues;
	TArray<int32> HistogramBins;
	TArray<FGorgeousInsightPieSlice> PieSlices;
	TArray<FGorgeousInsightScatterPoint> ScatterPoints;
	TArray<float> HeatmapValues;
	int32 HeatmapRows = 0;
	int32 HeatmapColumns = 0;
	TArray<FGorgeousInsightTimelineSegment> TimelineSegments;
	TArray<FGorgeousInsightFlowNode> FlowNodes;
	TArray<FGorgeousInsightFlowEdge> FlowEdges;
	TArray<FGorgeousInsightTableRow> TableRows;
	bool bPieDonut = true;

	static FGorgeousInsightChartDefinition MakeBarChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<FGorgeousInsightBarValue>& InBars)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Bar;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.Bars = InBars;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakeLineChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<double>& InValues)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Line;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.LineValues = InValues;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakeHistogramChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<int32>& InBins)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Histogram;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.HistogramBins = InBins;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakePieChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<FGorgeousInsightPieSlice>& InSlices, bool bInDonut = true)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Pie;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.PieSlices = InSlices;
		Chart.bPieDonut = bInDonut;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakeScatterChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<FGorgeousInsightScatterPoint>& InPoints)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Scatter;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.ScatterPoints = InPoints;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakeHeatmapChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<float>& InValues, int32 InRows, int32 InColumns)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Heatmap;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.HeatmapValues = InValues;
		Chart.HeatmapRows = InRows;
		Chart.HeatmapColumns = InColumns;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakeTimelineChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<FGorgeousInsightTimelineSegment>& InSegments)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Timeline;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.TimelineSegments = InSegments;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakeFlowChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<FGorgeousInsightFlowNode>& InNodes, const TArray<FGorgeousInsightFlowEdge>& InEdges)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Flow;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.FlowNodes = InNodes;
		Chart.FlowEdges = InEdges;
		return Chart;
	}

	static FGorgeousInsightChartDefinition MakeTableChart(const FName InId, const FText& InTitle, const FText& InSubtitle, const TArray<FGorgeousInsightTableRow>& InRows)
	{
		FGorgeousInsightChartDefinition Chart;
		Chart.Id = InId;
		Chart.Type = EGorgeousInsightChartType::Table;
		Chart.Title = InTitle;
		Chart.Subtitle = InSubtitle;
		Chart.TableRows = InRows;
		return Chart;
	}
};
