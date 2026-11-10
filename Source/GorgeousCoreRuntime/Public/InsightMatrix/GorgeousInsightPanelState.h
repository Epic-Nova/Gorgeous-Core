// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
/** Serializable presentation data. Core deliberately exposes no Slate widget types. */
struct GORGEOUSCORERUNTIME_API FGorgeousInsightPanelState
{
		FName SelectedProvider = NAME_None;
		FGorgeousInsightGatherContext Context;
		TMap<FName, double> LastToastTimes;
		FName SelectedStatCategory = NAME_None;
		FString ProviderFilter;
		bool bRunWithHarness = false;
		TMap<FName, TMap<FName, FString>> TestInputValues;
		TMap<FName, double> StatWarningThresholds;
		TMap<FName, double> StatCriticalThresholds;
};
