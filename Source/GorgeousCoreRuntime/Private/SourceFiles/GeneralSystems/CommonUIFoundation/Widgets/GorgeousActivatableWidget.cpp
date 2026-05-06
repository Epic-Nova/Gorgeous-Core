// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "Engine/LocalPlayer.h"

void UGorgeousActivatableWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UGorgeousUIFoundationSubsystem* Subsystem = LP->GetSubsystem<UGorgeousUIFoundationSubsystem>())
		{
			Subsystem->RegisterWidget(this);
		}
	}
}

void UGorgeousActivatableWidget::NativeDestruct()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UGorgeousUIFoundationSubsystem* Subsystem = LP->GetSubsystem<UGorgeousUIFoundationSubsystem>())
		{
			Subsystem->UnregisterWidget(this);
		}
	}

	Super::NativeDestruct();
}

void UGorgeousActivatableWidget::OnThemeApplied(const UGorgeousUITheme_DA* Theme)
{
	if (Theme)
	{
		OnThemeApplied_BP_Implementation(Theme);
	}
}

void UGorgeousActivatableWidget::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Default C++ implementation: forward to Blueprint default.
	OnThemeApplied_BP_Implementation(Theme);
}

void UGorgeousActivatableWidget::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Default: no-op. Override in Blueprints to react to theme changes.
}

void UGorgeousActivatableWidget::OnStateSwitched_Implementation(UCommonUIState_DA* NewState)
{
	// Default: no-op. Override in Blueprints to react to state changes.
}
