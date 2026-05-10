// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonButton.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "Kismet/GameplayStatics.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonButton)

UGorgeousCommonButton::UGorgeousCommonButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StylePropertyAllowList = {
		"Enabled",
		"IsEnabled",
		"Style",
		"ContentPadding"
	};
}

void UGorgeousCommonButton::NativeConstruct()
{
	Super::NativeConstruct();
	UE_UI_REGISTER_WIDGET_USER()
}

void UGorgeousCommonButton::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
	Super::NativeDestruct();
}

void UGorgeousCommonButton::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->ApplyThemeToWidget(this, Theme);
		return;
	}

	UGorgeousUIProcessor::ApplyThemeToWidgetInternal(this, Theme);
}

void UGorgeousCommonButton::NativeOnHovered()
{
	Super::NativeOnHovered();
	PlayThemedSound(HoverSoundTag);
}

void UGorgeousCommonButton::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
}

void UGorgeousCommonButton::NativeOnClicked()
{
	Super::NativeOnClicked();
	PlayThemedSound(ClickSoundTag);
}

void UGorgeousCommonButton::PlayThemedSound(FGameplayTag SoundTag)
{
	if (!SoundTag.IsValid()) return;

	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		if (UGorgeousUITheme_DA* Theme = Subsystem->GetCurrentTheme())
		{
			if (USoundBase* Sound = Theme->GetThemedSound(SoundTag))
			{
				UGameplayStatics::PlaySound2D(this, Sound);
			}
		}
	}
}


