// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#include "InsightMatrix/GorgeousInsightStatBuilder.h"

FCriticalSection UGorgeousInsightStatBuilder::Mutex;
TMap<FName, TMap<FName, FGorgeousInsightStat>> UGorgeousInsightStatBuilder::PublishedStatsCache;
TMap<FName, TMap<FName, FGorgeousInsightChartDefinition>> UGorgeousInsightStatBuilder::PublishedChartsCache;


void UGorgeousInsightStatBuilder::PublishCustomChart(FName ProviderName, FName ChartId, FName ChartType, const FString& Title, const FString& Subtitle, const FGorgeousInsightChartPayload& Payload)
{
	FScopeLock Lock(&Mutex);
	FGorgeousInsightChartDefinition Chart;
	Chart.Id = ChartId;
	Chart.CustomChartType = ChartType;
	Chart.Title = FText::FromString(Title);
	Chart.Subtitle = FText::FromString(Subtitle);
	Chart.Payload = Payload;
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishCustomChartInstanced(FName ProviderName, FName ChartId, FName ChartType, const FString& Title, const FString& Subtitle, const FInstancedStruct& Payload)
{
	FScopeLock Lock(&Mutex);
	FGorgeousInsightChartDefinition Chart;
	Chart.Id = ChartId;
	Chart.CustomChartType = ChartType;
	Chart.Title = FText::FromString(Title);
	Chart.Subtitle = FText::FromString(Subtitle);
	Chart.InstancedPayload = Payload;
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}


void UGorgeousInsightStatBuilder::PublishNativePieChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Labels, const TArray<float>& Values, const TArray<FLinearColor>& Colors, bool bDonut)
{
	FScopeLock Lock(&Mutex);
	TArray<FGorgeousInsightPieSlice> Slices;
	int32 Count = FMath::Min(Labels.Num(), Values.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FGorgeousInsightPieSlice Slice;
		Slice.Label = FText::FromString(Labels[i]);
		Slice.Value = Values[i];
		if (Colors.IsValidIndex(i))
		{
			Slice.Color = Colors[i];
		}
		Slices.Add(Slice);
	}
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakePieChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Slices, bDonut);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishNativeBarChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Labels, const TArray<float>& Values)
{
	FScopeLock Lock(&Mutex);
	TArray<FGorgeousInsightBarValue> Bars;
	int32 Count = FMath::Min(Labels.Num(), Values.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FGorgeousInsightBarValue Bar;
		Bar.Label = FText::FromString(Labels[i]);
		Bar.Value = Values[i];
		Bars.Add(Bar);
	}
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeBarChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Bars);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}


void UGorgeousInsightStatBuilder::PublishNativeLineChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<float>& Values)
{
	FScopeLock Lock(&Mutex);
	TArray<double> Doubles;
	for (float V : Values) Doubles.Add(V);
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeLineChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Doubles);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishNativeHistogramChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<int32>& Bins)
{
	FScopeLock Lock(&Mutex);
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeHistogramChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Bins);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishNativeScatterChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FVector2D>& Positions, const TArray<FLinearColor>& Colors, const TArray<float>& Radii)
{
	FScopeLock Lock(&Mutex);
	TArray<FGorgeousInsightScatterPoint> Points;
	for (int32 i = 0; i < Positions.Num(); ++i)
	{
		FGorgeousInsightScatterPoint Pt;
		Pt.Position = Positions[i];
		if (Colors.IsValidIndex(i)) Pt.Color = Colors[i];
		if (Radii.IsValidIndex(i)) Pt.Radius = Radii[i];
		Points.Add(Pt);
	}
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeScatterChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Points);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishNativeHeatmapChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<float>& Values, int32 Rows, int32 Columns)
{
	FScopeLock Lock(&Mutex);
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeHeatmapChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Values, Rows, Columns);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishNativeTimelineChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Labels, const TArray<float>& Starts, const TArray<float>& Durations, const TArray<FLinearColor>& Colors)
{
	FScopeLock Lock(&Mutex);
	TArray<FGorgeousInsightTimelineSegment> Segments;
	int32 Count = FMath::Min3(Labels.Num(), Starts.Num(), Durations.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FGorgeousInsightTimelineSegment Seg;
		Seg.Label = FText::FromString(Labels[i]);
		Seg.Start = Starts[i];
		Seg.Duration = Durations[i];
		if (Colors.IsValidIndex(i)) Seg.Color = Colors[i];
		Segments.Add(Seg);
	}
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeTimelineChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Segments);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishNativeFlowChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& NodeLabels, const TArray<FVector2D>& NodePositions, const TArray<FVector2D>& NodeSizes, const TArray<FLinearColor>& NodeColors, const TArray<int32>& EdgeFroms, const TArray<int32>& EdgeTos, const TArray<FLinearColor>& EdgeColors)
{
	FScopeLock Lock(&Mutex);
	TArray<FGorgeousInsightFlowNode> Nodes;
	for (int32 i = 0; i < NodeLabels.Num(); ++i)
	{
		FGorgeousInsightFlowNode Node;
		Node.Label = FText::FromString(NodeLabels[i]);
		if (NodePositions.IsValidIndex(i)) Node.Position = NodePositions[i];
		if (NodeSizes.IsValidIndex(i)) Node.Size = NodeSizes[i];
		if (NodeColors.IsValidIndex(i)) Node.Color = NodeColors[i];
		Nodes.Add(Node);
	}

	TArray<FGorgeousInsightFlowEdge> Edges;
	int32 EdgeCount = FMath::Min(EdgeFroms.Num(), EdgeTos.Num());
	for (int32 i = 0; i < EdgeCount; ++i)
	{
		FGorgeousInsightFlowEdge Edge;
		Edge.From = EdgeFroms[i];
		Edge.To = EdgeTos[i];
		if (EdgeColors.IsValidIndex(i)) Edge.Color = EdgeColors[i];
		Edges.Add(Edge);
	}

	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeFlowChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Nodes, Edges);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

void UGorgeousInsightStatBuilder::PublishNativeTableChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Names, const TArray<FString>& Values, const TArray<FString>& Categories)
{
	FScopeLock Lock(&Mutex);
	TArray<FGorgeousInsightTableRow> Rows;
	int32 Count = FMath::Min(Names.Num(), Values.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FGorgeousInsightTableRow Row;
		Row.Name = FText::FromString(Names[i]);
		Row.Value = FText::FromString(Values[i]);
		if (Categories.IsValidIndex(i)) Row.Category = FText::FromString(Categories[i]);
		Rows.Add(Row);
	}
	FGorgeousInsightChartDefinition Chart = FGorgeousInsightChartDefinition::MakeTableChart(ChartId, FText::FromString(Title), FText::FromString(Subtitle), Rows);
	PublishedChartsCache.FindOrAdd(ProviderName).Add(ChartId, Chart);
}

FString UGorgeousInsightStatBuilder::GetChartLabel(const FGorgeousInsightChartPayload& Payload, FName Key, const FString& DefaultValue)
{
	if (const FString* Found = Payload.Labels.Find(Key)) return *Found;
	return DefaultValue;
}

float UGorgeousInsightStatBuilder::GetChartValue(const FGorgeousInsightChartPayload& Payload, FName Key, float DefaultValue)
{
	if (const float* Found = Payload.Values.Find(Key)) return *Found;
	return DefaultValue;
}

FLinearColor UGorgeousInsightStatBuilder::GetChartColor(const FGorgeousInsightChartPayload& Payload, FName Key, FLinearColor DefaultValue)
{
	if (const FLinearColor* Found = Payload.Colors.Find(Key)) return *Found;
	return DefaultValue;
}

void UGorgeousInsightStatBuilder::GatherPublishedCharts(FName ProviderName, TArray<FGorgeousInsightChartDefinition>& OutCharts)
{
	FScopeLock Lock(&Mutex);
	if (const TMap<FName, FGorgeousInsightChartDefinition>* ProviderCharts = PublishedChartsCache.Find(ProviderName))
	{
		for (const auto& Pair : *ProviderCharts)
		{
			OutCharts.Add(Pair.Value);
		}
	}
}


void UGorgeousInsightStatBuilder::AddStat(TArray<FGorgeousInsightStat>& OutStats, FName Id, const FString& Label, FName Category, EGorgeousInsightStatValueType ValueType, double NumericValue, const FString& Unit)
{
	FGorgeousInsightStat Stat;
	Stat.Id = Id;
	Stat.DisplayName = FText::FromString(Label);
	Stat.Category = Category;
	Stat.ValueType = ValueType;
	Stat.NumericValue = NumericValue;
	Stat.Unit = FText::FromString(Unit);
	OutStats.Add(Stat);
}

void UGorgeousInsightStatBuilder::AddTextStat(TArray<FGorgeousInsightStat>& OutStats, FName Id, const FString& Label, FName Category, const FString& Value)
{
	FGorgeousInsightStat Stat;
	Stat.Id = Id;
	Stat.DisplayName = FText::FromString(Label);
	Stat.Category = Category;
	Stat.ValueType = EGorgeousInsightStatValueType::Text;
	Stat.TextValue = FText::FromString(Value);
	OutStats.Add(Stat);
}

void UGorgeousInsightStatBuilder::PublishNumberStat(FName ProviderName, FName StatId, const FString& DisplayName, FName Category, float Value, const FString& Unit)
{
	FScopeLock Lock(&Mutex);
	FGorgeousInsightStat Stat;
	Stat.Id = StatId;
	Stat.DisplayName = FText::FromString(DisplayName);
	Stat.Category = Category;
	Stat.ValueType = EGorgeousInsightStatValueType::Number;
	Stat.NumericValue = Value;
	Stat.Unit = FText::FromString(Unit);
	PublishedStatsCache.FindOrAdd(ProviderName).Add(StatId, Stat);
}

void UGorgeousInsightStatBuilder::PublishTextStat(FName ProviderName, FName StatId, const FString& DisplayName, FName Category, const FString& Value)
{
	FScopeLock Lock(&Mutex);
	FGorgeousInsightStat Stat;
	Stat.Id = StatId;
	Stat.DisplayName = FText::FromString(DisplayName);
	Stat.Category = Category;
	Stat.ValueType = EGorgeousInsightStatValueType::Text;
	Stat.TextValue = FText::FromString(Value);
	PublishedStatsCache.FindOrAdd(ProviderName).Add(StatId, Stat);
}

void UGorgeousInsightStatBuilder::GatherPublishedStats(FName ProviderName, TArray<FGorgeousInsightStat>& OutStats)
{
	FScopeLock Lock(&Mutex);
	if (const TMap<FName, FGorgeousInsightStat>* ProviderStats = PublishedStatsCache.Find(ProviderName))
	{
		for (const auto& Pair : *ProviderStats)
		{
			OutStats.Add(Pair.Value);
		}
	}
}
