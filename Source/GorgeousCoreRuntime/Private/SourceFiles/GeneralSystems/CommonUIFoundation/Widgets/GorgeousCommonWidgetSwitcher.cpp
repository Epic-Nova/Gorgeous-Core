// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidgetSwitcher.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonWidgetSwitcher)

void UGorgeousCommonWidgetSwitcher::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonWidgetSwitcher::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonWidgetSwitcher::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
}
