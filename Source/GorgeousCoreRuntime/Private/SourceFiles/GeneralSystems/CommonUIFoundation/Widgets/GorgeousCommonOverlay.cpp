// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonOverlay.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonOverlay)

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonOverlay)

void UGorgeousCommonOverlay::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousCommonOverlay::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// no-op default
}
