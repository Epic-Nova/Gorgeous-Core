// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonHorizontalBox.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonHorizontalBox)

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonHorizontalBox)

void UGorgeousCommonHorizontalBox::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousCommonHorizontalBox::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// no-op default
}
