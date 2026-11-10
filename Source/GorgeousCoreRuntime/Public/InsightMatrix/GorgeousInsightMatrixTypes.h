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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightMatrixTypes.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UWorld;
//<------------------------------------------------------------->
UENUM(BlueprintType)
enum class EGorgeousInsightContextMode : uint8
{
	Live,
	Editor,
	Baseline
};

USTRUCT(BlueprintType)
struct FGorgeousInsightGatherContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Insight Matrix")
	EGorgeousInsightContextMode Mode = EGorgeousInsightContextMode::Editor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Insight Matrix")
	UWorld* WorldContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Insight Matrix")
	FString InstanceName;
};

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
			Class,
			Dropdown
		} Type = EGorgeousInsightTestInputType::String;

		FString DefaultValue;
		bool bRequired = false;
		/** Optional allowed base class path for Class inputs. */
		FString AllowedClassPath;

		/** Options for Dropdown inputs. */
		TArray<FString> DropdownOptions;

		/** Internal array of pointers required by SComboBox. */
		TArray<TSharedPtr<FString>> DropdownOptionPtrs;
	};

	TArray<FGorgeousInsightTestInput> Inputs;
};

/** Action execution context. */
struct FGorgeousInsightActionContext
{
	UObject* WorldContextObject = nullptr;
	TMap<FString, FString> Parameters;
};

enum class EGorgeousInsightActionStatus : uint8 { Dispatched, Succeeded, Failed };
struct FGorgeousInsightActionResult
{
    EGorgeousInsightActionStatus Status = EGorgeousInsightActionStatus::Dispatched;
    FString Message;
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
	TArray<FString> LogCapture;
};

/** Shared chart value types for provider-defined charts. */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousInsightBarValue
{
	GENERATED_BODY()
	UPROPERTY()
	FText Label;
	UPROPERTY()
	double Value = 0.0;
};

USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousInsightPieSlice
{
	GENERATED_BODY()
	UPROPERTY()
	FText Label;
	UPROPERTY()
	double Value = 0.0;
	UPROPERTY()
	FLinearColor Color = FLinearColor::White;
};

USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousInsightScatterPoint
{
	GENERATED_BODY()
	UPROPERTY()
	FVector2D Position = FVector2D::ZeroVector;
	UPROPERTY()
	FLinearColor Color = FLinearColor::White;
	UPROPERTY()
	float Radius = 3.f;
};

USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousInsightTimelineSegment
{
	GENERATED_BODY()
	UPROPERTY()
	FText Label;
	UPROPERTY()
	float Start = 0.f;
	UPROPERTY()
	float Duration = 0.f;
	UPROPERTY()
	FLinearColor Color = FLinearColor::White;
};

USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousInsightFlowNode
{
	GENERATED_BODY()
	UPROPERTY()
	FText Label;
	UPROPERTY()
	FVector2D Position = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D Size = FVector2D(0.2f, 0.18f);
	UPROPERTY()
	FLinearColor Color = FLinearColor(0.2f, 0.4f, 0.7f, 1.f);
};

USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousInsightFlowEdge
{
	GENERATED_BODY()
	UPROPERTY()
	int32 From = INDEX_NONE;
	UPROPERTY()
	int32 To = INDEX_NONE;
	UPROPERTY()
	FLinearColor Color = FLinearColor(0.7f, 0.7f, 0.7f, 1.f);
};

USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousInsightTableRow
{
	GENERATED_BODY()
	UPROPERTY()
	FText Name;
	UPROPERTY()
	FText Value;
	UPROPERTY()
	FText Category;
};


USTRUCT(BlueprintType)
struct FGorgeousInsightChartPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Insight Matrix")
	TMap<FName, FString> Labels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Insight Matrix")
	TMap<FName, float> Values;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Insight Matrix")
	TMap<FName, FLinearColor> Colors;
};

UENUM()
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

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousInsightChartDefinition
{
	GENERATED_BODY()

	UPROPERTY()
	FName Id = NAME_None;
	UPROPERTY()
	FName CustomChartType = NAME_None;
	UPROPERTY()
	FGorgeousInsightChartPayload Payload;
	UPROPERTY(Transient)
	FInstancedStruct InstancedPayload;
	UPROPERTY()
	EGorgeousInsightChartType Type = EGorgeousInsightChartType::Bar;
	UPROPERTY()
	FText Title;
	UPROPERTY()
	FText Subtitle;

	UPROPERTY()
	TArray<FGorgeousInsightBarValue> Bars;
	UPROPERTY()
	TArray<double> LineValues;
	UPROPERTY()
	TArray<int32> HistogramBins;
	UPROPERTY()
	TArray<FGorgeousInsightPieSlice> PieSlices;
	UPROPERTY()
	TArray<FGorgeousInsightScatterPoint> ScatterPoints;
	UPROPERTY()
	TArray<float> HeatmapValues;
	UPROPERTY()
	int32 HeatmapRows = 0;
	UPROPERTY()
	int32 HeatmapColumns = 0;
	UPROPERTY()
	TArray<FGorgeousInsightTimelineSegment> TimelineSegments;
	UPROPERTY()
	TArray<FGorgeousInsightFlowNode> FlowNodes;
	UPROPERTY()
	TArray<FGorgeousInsightFlowEdge> FlowEdges;
	UPROPERTY()
	TArray<FGorgeousInsightTableRow> TableRows;
	UPROPERTY()
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