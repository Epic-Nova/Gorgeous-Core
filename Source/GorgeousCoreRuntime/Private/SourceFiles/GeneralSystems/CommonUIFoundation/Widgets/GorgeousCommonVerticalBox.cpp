// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonVerticalBox.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonVerticalBox)
 
UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonVerticalBox)

void UGorgeousCommonVerticalBox::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousCommonVerticalBox::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// no-op default
}
