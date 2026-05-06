// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonCarousel.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousCarouselProcessor.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonCarousel)

void UGorgeousCommonCarousel::NativeConstruct()
{
	// Automatically register with the subsystem
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->RegisterWidget(this);
	}
}

void UGorgeousCommonCarousel::NativeDestruct()
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->UnregisterWidget(this);
	}
}

void UGorgeousCommonCarousel::Next()
{
	int32 NextIndex = GetActiveWidgetIndex() + 1;
	if (NextIndex >= GetNumWidgets())
	{
		NextIndex = 0; // Wrap around
	}
	SetActiveWidgetIndex(NextIndex);
}

void UGorgeousCommonCarousel::Previous()
{
	int32 PrevIndex = GetActiveWidgetIndex() - 1;
	if (PrevIndex < 0)
	{
		PrevIndex = GetNumWidgets() - 1; // Wrap around
	}
	SetActiveWidgetIndex(PrevIndex);
}


void UGorgeousCommonCarousel::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Blueprint hook for additional theme application logic if needed
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonCarousel)

void UGorgeousCommonCarousel::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Default C++ implementation: no-op.
}
