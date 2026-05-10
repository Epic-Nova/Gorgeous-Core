// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonVerticalBox.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonVerticalBox)

UGorgeousCommonVerticalBox::UGorgeousCommonVerticalBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StylePropertyAllowList = {
		"RenderOpacity",
		"Visibility",
		"IsEnabled"
	};
}

void UGorgeousCommonVerticalBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonVerticalBox::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonVerticalBox::ReleaseSlateResources(bool bReleaseChildren)
{
	UE_UI_UNREGISTER_WIDGET()
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UGorgeousCommonVerticalBox::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->ApplyThemeToWidget(this, Theme);
		return;
	}

	UGorgeousUIProcessor::ApplyThemeToWidgetInternal(this, Theme);
}
