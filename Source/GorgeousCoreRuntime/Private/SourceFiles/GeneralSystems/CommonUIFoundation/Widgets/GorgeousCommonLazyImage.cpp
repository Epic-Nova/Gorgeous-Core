// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonLazyImage.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"

void UGorgeousCommonLazyImage::NativeConstruct()
{
	// 1. Register with the Subsystem for Signal updates
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->RegisterWidget(this);

		// 2. Initial Theme Application
		if (UGorgeousUITheme_DA* Theme = Subsystem->GetCurrentTheme())
		{
			OnThemeApplied_Implementation(Theme);
		}
	}
}

void UGorgeousCommonLazyImage::NativeDestruct()
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->UnregisterWidget(this);
	}

}

void UGorgeousCommonLazyImage::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	UpdateActionIcon();
}

void UGorgeousCommonLazyImage::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Blueprint hook for additional theme application logic if needed.
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonLazyImage)

void UGorgeousCommonLazyImage::UpdateActionIcon()
{
	if (!ActionTag.IsValid()) return;

	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		if (UGorgeousUITheme_DA* Theme = Subsystem->GetCurrentTheme())
		{
			const FName PlatformName = Subsystem->GetCurrentPlatformName();
			FSlateBrush IconBrush = Theme->GetActionIcon(ActionTag, PlatformName);
			if (IconBrush.HasUObject() || IconBrush.GetResourceName() != NAME_None)
			{
				// Even though it's a LazyImage, we apply the themed icon as a standard brush
				FInstancedStruct Payload = FInstancedStruct::Make(IconBrush);
				UGorgeousUIProcessor::ApplyPropertyToTarget(this, "Brush", Payload);
			}
		}
	}
}
