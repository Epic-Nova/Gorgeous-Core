// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonBorder.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

void UGorgeousCommonBorder::NativeConstruct()
{
	UE_UI_REGISTER_WIDGET()
}

void UGorgeousCommonBorder::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
}

void UGorgeousCommonBorder::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	UE_UI_TICK_THEME_INTERP()
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonBorder)

void UGorgeousCommonBorder::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
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

void UGorgeousCommonBorder::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Blueprint hook for additional theme application logic if needed.
}