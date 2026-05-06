// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonLazyImage.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonLazyImage)

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

void UGorgeousCommonLazyImage::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	UpdateActionIcon();
}

void UGorgeousCommonLazyImage::UpdateActionIcon()
{
	if (!ActionTag.IsValid()) return;

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
