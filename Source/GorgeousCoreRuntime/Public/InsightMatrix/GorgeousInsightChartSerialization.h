// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "Dom/JsonObject.h"

namespace GorgeousInsightCharts
{
GORGEOUSCORERUNTIME_API TSharedRef<FJsonObject> ToJson(const FGorgeousInsightChartDefinition& Chart);
GORGEOUSCORERUNTIME_API bool FromJson(const TSharedRef<FJsonObject>& Json, FGorgeousInsightChartDefinition& Chart);
GORGEOUSCORERUNTIME_API TArray<TSharedPtr<FJsonValue>> ToJsonArray(const TArray<FGorgeousInsightChartDefinition>& Charts);
GORGEOUSCORERUNTIME_API TArray<FGorgeousInsightChartDefinition> FromJsonArray(const TArray<TSharedPtr<FJsonValue>>& Json);
}
