// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Blueprint/UserWidget.h"
#include "GorgeousInsightChartRegistry.generated.h"

/**
 * Registry mapping custom chart type names to UMG Widget blueprints.
 * 
 * NOTE ON NATIVE SLATE CHARTS:
 * The Insight Matrix inherently supports native high-performance Slate charts if you leave the 
 * CustomChartType empty and use the EGorgeousInsightChartType enum instead (e.g. Bar, Pie, Line). 
 * This registry is strictly for overriding or creating entirely new abstract visualization widgets via Blueprints.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Gorgeous Insight Matrix"))
class GORGEOUSCORERUNTIME_API UGorgeousInsightChartRegistry : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousInsightChartRegistry()
	{
		CategoryName = TEXT("Gorgeous Things");
		SectionName = TEXT("Insight Matrix");
	}

	/** Maps a custom chart type FName to a UMG Widget Class that implements IGorgeousInsightChartWidgetInterface. */
	UPROPERTY(Config, EditAnywhere, Category="Charts", meta=(ToolTip="Map your custom Chart FName to a UMG Widget class. The widget must implement IGorgeousInsightChartWidgetInterface to receive the payload."))
	TMap<FName, TSoftClassPtr<UUserWidget>> RegisteredUMGCharts;
};
