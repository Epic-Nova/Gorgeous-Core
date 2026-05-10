// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonCarousel.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonCarousel)

UGorgeousCommonCarousel::UGorgeousCommonCarousel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StylePropertyAllowList = {
		"ActiveWidgetIndex",
		"RenderOpacity",
		"Visibility",
		"IsEnabled"
	};
}

void UGorgeousCommonCarousel::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonCarousel::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonCarousel::ReleaseSlateResources(bool bReleaseChildren)
{
	UE_UI_UNREGISTER_WIDGET()
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UGorgeousCommonCarousel::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->ApplyThemeToWidget(this, Theme);
		return;
	}

	UGorgeousUIProcessor::ApplyThemeToWidgetInternal(this, Theme);
}
