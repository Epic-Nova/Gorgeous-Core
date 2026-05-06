// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonGridPanel.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonGridPanel)

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonGridPanel)

void UGorgeousCommonGridPanel::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousCommonGridPanel::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// no-op default
}
