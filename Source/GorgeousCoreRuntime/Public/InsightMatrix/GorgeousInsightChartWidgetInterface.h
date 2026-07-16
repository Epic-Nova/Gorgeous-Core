// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "GorgeousInsightChartWidgetInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousInsightChartWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface that your custom UMG widgets must implement to receive 
 * dynamic data payloads from the Insight Matrix Publisher.
 */
class GORGEOUSCORERUNTIME_API IGorgeousInsightChartWidgetInterface
{
	GENERATED_BODY()

public:
	/** Fired when the widget is created and supplied with standard payload data. */
	UFUNCTION(BlueprintImplementableEvent, Category="Insight Matrix|Charts")
	void ReceiveChartData(const FText& Title, const FText& Subtitle, const FGorgeousInsightChartPayload& Payload);

	/** Fired when the widget is created and supplied with an Instanced Struct payload. */
	UFUNCTION(BlueprintImplementableEvent, Category="Insight Matrix|Charts")
	void ReceiveInstancedChartData(const FText& Title, const FText& Subtitle, const FInstancedStruct& Payload);
};
