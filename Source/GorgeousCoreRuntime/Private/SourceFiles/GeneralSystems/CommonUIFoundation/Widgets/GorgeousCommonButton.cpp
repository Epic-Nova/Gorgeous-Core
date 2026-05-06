// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonButton.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "Kismet/GameplayStatics.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonButton)

void UGorgeousCommonButton::NativeConstruct()
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->RegisterWidget(this);
	}
}

void UGorgeousCommonButton::NativeDestruct()
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->UnregisterWidget(this);
	}
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

void UGorgeousCommonButton::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Blueprint hook for additional theme application logic if needed.
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonButton)

void UGorgeousCommonButton::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Default C++ implementation: no-op. Blueprint implementations handled in BP_Implementation.
}
