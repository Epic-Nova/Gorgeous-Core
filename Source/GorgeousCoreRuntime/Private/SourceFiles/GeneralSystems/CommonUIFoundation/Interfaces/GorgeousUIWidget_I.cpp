// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidget.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"

void IGorgeousUIWidget_I::ApplyOverlayConfig(const FGorgeousUIStateConfig& Config)
{
	if (UWidget* Widget = Cast<UWidget>(GetAsWidget()))
	{
		Widget->SetVisibility(Config.bIsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		Widget->SetRenderOpacity(Config.Opacity);

		// If it's a Gorgeous Widget, try to play the animation
		if (Config.AnimationName != NAME_None)
		{
			if (UGorgeousCommonWidget* CommonWidget = Cast<UGorgeousCommonWidget>(Widget))
			{
				CommonWidget->PlayAnimationByName(Config.AnimationName);
			}
			else if (UGorgeousActivatableWidget* Activatable = Cast<UGorgeousActivatableWidget>(Widget))
			{
				Activatable->PlayAnimationByName(Config.AnimationName);
			}
		}
	}
}
