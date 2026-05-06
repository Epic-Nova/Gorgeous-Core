// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"

#include "Animation/WidgetAnimation.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousActivatableWidget)

void UGorgeousActivatableWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_UI_REGISTER_WIDGET_USER()
}

void UGorgeousActivatableWidget::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
	Super::NativeDestruct();
}

void UGorgeousActivatableWidget::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
}

void UGorgeousActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (AutoState)
	{
		UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
		if (Subsystem)
		{
			PreviousState = Subsystem->GetCurrentUIState();
			Subsystem->SwitchUIState(AutoState);
		}
	}
}

void UGorgeousActivatableWidget::NativeOnDeactivated()
{
	if (bRevertStateOnDeactivation && PreviousState)
	{
		UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
		if (Subsystem)
		{
			Subsystem->SwitchUIState(PreviousState);
		}
	}

	Super::NativeOnDeactivated();
}

void UGorgeousActivatableWidget::PlayAnimationByName(const FName& AnimName)
{
	if (AnimName.IsNone()) return;

	UWidgetAnimation* Found = nullptr;
	if (FProperty* Property = GetClass()->FindPropertyByName(AnimName))
	{
		if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			Found = Cast<UWidgetAnimation>(ObjectProperty->GetObjectPropertyValue_InContainer(this));
		}
	}

	if (!Found) Found = FindObject<UWidgetAnimation>(GetClass(), *AnimName.ToString());

	if (Found) PlayAnimation(Found);
}

void UGorgeousActivatableWidget::OnUIStateChanged_Implementation(UGorgeousUIState_DA* NewState)
{
	if (!NewState) return;

	if (const FName* AnimName = StateAnimations.Find(NewState))
	{
		PlayAnimationByName(*AnimName);
	}
}
