// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidgetSwitcher.h"

void UGorgeousCommonWidgetSwitcher::NativeConstruct()
{
	UE_UI_REGISTER_WIDGET()
}

void UGorgeousCommonWidgetSwitcher::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonWidgetSwitcher)

void UGorgeousCommonWidgetSwitcher::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousCommonWidgetSwitcher::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// no-op
}
