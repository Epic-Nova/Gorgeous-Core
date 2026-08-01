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
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightBaselineSettings.generated.h"
//<-------------------------------------------------------------------------->

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousInsightBaselineEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Config, Category = "Baseline")
	bool bSuccess = true;

	UPROPERTY(EditAnywhere, Config, Category = "Baseline")
	TArray<FString> Errors;

	UPROPERTY(EditAnywhere, Config, Category = "Baseline")
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, Config, Category = "Baseline")
	TArray<FString> Notes;

	UPROPERTY(EditAnywhere, Config, Category = "Baseline")
	TMap<FString, FString> Metrics;

	static FGorgeousInsightBaselineEntry FromResult(const FGorgeousInsightTestResult& Result);
	FGorgeousInsightTestResult ToResult() const;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Insight Baseline Settings
| Functional Name: UGorgeousInsightBaselineSettings
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Insight Baseline Settings.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(config = GorgeousInsightMatrix, defaultconfig,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/GorgeousInsightBaselineSettings",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousInsightBaselineSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Baseline")
	TMap<FString, FGorgeousInsightBaselineEntry> Baselines;

	static FString MakeKey(FName ProviderName, FName TestId)
	{
		return ProviderName.ToString() + TEXT(".") + TestId.ToString();
	}
};