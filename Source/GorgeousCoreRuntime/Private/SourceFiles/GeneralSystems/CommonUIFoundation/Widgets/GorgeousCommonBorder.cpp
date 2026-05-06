// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonBorder.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonBorder)

void UGorgeousCommonBorder::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonBorder::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonBorder::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	if (Theme)
	{
		SetBrush(Theme->GetBrush("BorderBrush"));
	}
	
	if (TargetThemeColors.Contains("BorderColor"))
	{
		FLinearColor DisplayColor = CurrentThemeColors.Contains("BorderColor") ? CurrentThemeColors["BorderColor"] : TargetThemeColors["BorderColor"];
		SetBrushColor(DisplayColor);
	}
}
