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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightStatBuilder.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Insight Stat Builder
| Functional Name: UGorgeousInsightStatBuilder
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Insight Stat Builder.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/GorgeousInsightStatBuilder",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousInsightStatBuilder : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// C++ Helpers for building arrays
	static void AddStat(TArray<FGorgeousInsightStat>& OutStats, FName Id, const FString& Label, FName Category, EGorgeousInsightStatValueType ValueType, double NumericValue, const FString& Unit = FString());
	static void AddTextStat(TArray<FGorgeousInsightStat>& OutStats, FName Id, const FString& Label, FName Category, const FString& Value);

	// Blueprint fast-path publishing (caches stats for the Matrix to pull)
	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Stats")
	static void PublishNumberStat(FName ProviderName, FName StatId, const FString& DisplayName, FName Category, float Value, const FString& Unit = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Stats")
	static void PublishTextStat(FName ProviderName, FName StatId, const FString& DisplayName, FName Category, const FString& Value);

	// Retrieve published BP stats for a provider
	static void GatherPublishedStats(FName ProviderName, TArray<FGorgeousInsightStat>& OutStats);

			// Retrieve published BP charts for a provider

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts")
	static void PublishCustomChart(FName ProviderName, FName ChartId, FName ChartType, const FString& Title, const FString& Subtitle, const FGorgeousInsightChartPayload& Payload);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts")
	static void PublishCustomChartInstanced(FName ProviderName, FName ChartId, FName ChartType, const FString& Title, const FString& Subtitle, const FInstancedStruct& Payload);


	// -------------------------------------------------------------------------
	// NATIVE SLATE CHARTS
	// Use these to publish high-performance native C++ charts without custom UMG widgets.
	// NOTE: In the future, we want to make these native charts more visually appealing
	// and informative, potentially tying them more tightly to the systems themselves.
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativePieChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Labels, const TArray<float>& Values, const TArray<FLinearColor>& Colors, bool bDonut = true);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeBarChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Labels, const TArray<float>& Values);


	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeLineChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<float>& Values);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeHistogramChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<int32>& Bins);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeScatterChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FVector2D>& Positions, const TArray<FLinearColor>& Colors, const TArray<float>& Radii);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeHeatmapChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<float>& Values, int32 Rows, int32 Columns);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeTimelineChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Labels, const TArray<float>& Starts, const TArray<float>& Durations, const TArray<FLinearColor>& Colors);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeFlowChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& NodeLabels, const TArray<FVector2D>& NodePositions, const TArray<FVector2D>& NodeSizes, const TArray<FLinearColor>& NodeColors, const TArray<int32>& EdgeFroms, const TArray<int32>& EdgeTos, const TArray<FLinearColor>& EdgeColors);

	UFUNCTION(BlueprintCallable, Category="Insight Matrix|Charts|Native")
	static void PublishNativeTableChart(FName ProviderName, FName ChartId, const FString& Title, const FString& Subtitle, const TArray<FString>& Names, const TArray<FString>& Values, const TArray<FString>& Categories);

	// TMap Payload Helpers
	UFUNCTION(BlueprintPure, Category="Insight Matrix|Charts")
	static FString GetChartLabel(const FGorgeousInsightChartPayload& Payload, FName Key, const FString& DefaultValue = TEXT(""));

	UFUNCTION(BlueprintPure, Category="Insight Matrix|Charts")
	static float GetChartValue(const FGorgeousInsightChartPayload& Payload, FName Key, float DefaultValue = 0.0f);

	UFUNCTION(BlueprintPure, Category="Insight Matrix|Charts")
	static FLinearColor GetChartColor(const FGorgeousInsightChartPayload& Payload, FName Key, FLinearColor DefaultValue = FLinearColor::White);

	static void GatherPublishedCharts(FName ProviderName, TArray<FGorgeousInsightChartDefinition>& OutCharts);


private:
	static FCriticalSection Mutex;
	static TMap<FName, TMap<FName, FGorgeousInsightStat>> PublishedStatsCache;
	static TMap<FName, TMap<FName, FGorgeousInsightChartDefinition>> PublishedChartsCache;
};