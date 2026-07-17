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
#include "Features/IModularFeatures.h"
#include "Interfaces/GorgeousStatisticsInterface.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

/**
 * Modular feature interface for providing an editor stats widget implementation.
 */
class GORGEOUSCORERUNTIME_API IGorgeousStatsWidgetProvider : public IModularFeature
{
public:
	static FName GetModularFeatureName()
	{
		static const FName FeatureName(TEXT("GorgeousStatsWidgetProvider"));
		return FeatureName;
	}

	virtual ~IGorgeousStatsWidgetProvider() = default;

	virtual void ShowStatsWidget(const TArray<FGorgeousStatisticsSnapshot>& Snapshots, int32 ZOrder) = 0;
	virtual void UpdateStatsWidget(const TArray<FGorgeousStatisticsSnapshot>& Snapshots) = 0;
	virtual void HideStatsWidget() = 0;

	static IGorgeousStatsWidgetProvider* Get()
	{
		if (!IModularFeatures::Get().IsModularFeatureAvailable(GetModularFeatureName()))
		{
			return nullptr;
		}
		return &IModularFeatures::Get().GetModularFeature<IGorgeousStatsWidgetProvider>(GetModularFeatureName());
	}
};