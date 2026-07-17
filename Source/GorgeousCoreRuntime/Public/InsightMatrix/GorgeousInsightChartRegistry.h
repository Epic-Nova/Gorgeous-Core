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
#include "Blueprint/UserWidget.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightChartRegistry.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Insight Chart Registry
| Functional Name: UGorgeousInsightChartRegistry
| Parent Class: UDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Registry mapping custom chart type names to UMG Widget blueprints. NOTE ON
| NATIVE SLATE CHARTS: The Insight Matrix inherently supports native
| high-performance Slate charts if you leave the CustomChartType empty and
| use the EGorgeousInsightChartType enum instead (e.g. Bar, Pie, Line). This
| registry is strictly for overriding or creating entirely new abstract
| visualization widgets via Blueprints.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Config=Game, DefaultConfig,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/GorgeousInsightChartRegistry",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousInsightChartRegistry : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousInsightChartRegistry()
	{
		CategoryName = TEXT("Gorgeous Things");
		SectionName = TEXT("Insight Matrix");
	}

	// Maps a custom chart type FName to a UMG Widget Class that implements IGorgeousInsightChartWidgetInterface.
	UPROPERTY(Config, EditAnywhere, Category="Charts", meta=(ToolTip="Map your custom Chart FName to a UMG Widget class. The widget must implement IGorgeousInsightChartWidgetInterface to receive the payload."))
	TMap<FName, TSoftClassPtr<UUserWidget>> RegisteredUMGCharts;

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
#endif
};
