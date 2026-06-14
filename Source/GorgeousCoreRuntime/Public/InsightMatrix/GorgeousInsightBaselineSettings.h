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

#pragma once

#include "CoreMinimal.h"
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "GorgeousInsightBaselineSettings.generated.h"

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

UCLASS(config = GorgeousInsightMatrix, defaultconfig)
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
