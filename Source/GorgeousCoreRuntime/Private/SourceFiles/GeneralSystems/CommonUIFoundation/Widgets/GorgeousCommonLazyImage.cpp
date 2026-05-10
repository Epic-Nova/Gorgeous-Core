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

	if (ThemeOverride)
	{
		FSlateBrush IconBrush = ThemeOverride->GetActionIcon(ActionTag, TEXT("Generic"));
		SetBrush(IconBrush);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				if (UGorgeousUIFoundationSubsystem* Subsystem = LP->GetSubsystem<UGorgeousUIFoundationSubsystem>())
				{
					if (UGorgeousUITheme_DA* Theme = Subsystem->GetCurrentTheme())
					{
						const FName PlatformName = Subsystem->GetCurrentPlatformName();
						FSlateBrush IconBrush = Theme->GetActionIcon(ActionTag, PlatformName);
						SetBrush(IconBrush);
					}
				}
			}
		}
	}
}
