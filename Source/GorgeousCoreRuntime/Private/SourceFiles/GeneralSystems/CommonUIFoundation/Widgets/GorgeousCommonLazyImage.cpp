// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonLazyImage.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonLazyImage)

UGorgeousCommonLazyImage::UGorgeousCommonLazyImage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StylePropertyAllowList = {
		"Brush",
		"ColorAndOpacity"
	};
}

void UGorgeousCommonLazyImage::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonLazyImage::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonLazyImage::ReleaseSlateResources(bool bReleaseChildren)
{
	UE_UI_UNREGISTER_WIDGET()
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UGorgeousCommonLazyImage::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->ApplyThemeToWidget(this, Theme);
	}
	else
	{
		UGorgeousUIProcessor::ApplyThemeToWidgetInternal(this, Theme);
	}

	if (Theme)
	{
		UpdateActionIcon(Theme);
	}
}

void UGorgeousCommonLazyImage::UpdateActionIcon(const UGorgeousUITheme_DA* ThemeOverride)
{
	if (!ActionTag.IsValid()) return;
	if (!UGorgeousUIProcessor::IsStylePropertyAllowed(this, "Brush")) return;

	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		// Reverse iterate to fetch resources from the most recent theme first
		for (int32 i = 0; i < Subsystem->GetCurrentThemes().Num() - 1; --i)
		{
			const UGorgeousUITheme_DA* FinalTheme = ThemeOverride ? ThemeOverride : Subsystem->GetCurrentThemes()[i];
			FSlateBrush IconBrush = FinalTheme->GetActionIcon(ActionTag, Subsystem->GetCurrentPlatformName());
		
			if (IconBrush.HasUObject() || IconBrush.GetResourceName() != NAME_None)
			{
				FInstancedStruct Payload = FInstancedStruct::Make(IconBrush);
				UGorgeousUIProcessor::ApplyStylePropertyToTarget(this, "Brush", Payload);
				break;
			}
		}
	}
}
