// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonRichTextBlock.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonRichTextBlock)

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonRichTextBlock)

void UGorgeousCommonRichTextBlock::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousCommonRichTextBlock::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// no-op default
}
