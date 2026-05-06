// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonNumericTextBlock.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonNumericTextBlock)

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonNumericTextBlock)

void UGorgeousCommonNumericTextBlock::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousCommonNumericTextBlock::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// no-op default
}
