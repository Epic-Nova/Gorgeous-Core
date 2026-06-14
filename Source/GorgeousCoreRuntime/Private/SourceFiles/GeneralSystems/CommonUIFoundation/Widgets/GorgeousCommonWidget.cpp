// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidget.h"

#include "Animation/WidgetAnimation.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "QualityOfLife/GorgeousPlayerController.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonWidget)

UGorgeousCommonWidget::UGorgeousCommonWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StylePropertyAllowList = {
		"Brush",
		"RenderOpacity",
		"Visibility",
		"IsEnabled"
	};
}

void UGorgeousCommonWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_UI_REGISTER_WIDGET_USER()
}

void UGorgeousCommonWidget::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
	Super::NativeDestruct();
}

void UGorgeousCommonWidget::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
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

void UGorgeousCommonWidget::UpdateActionIcon(const UGorgeousUITheme_DA* ThemeOverride)
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

void UGorgeousCommonWidget::OnUIStateChanged_Implementation(UGorgeousUIState_DA* NewState)
{
	if (!NewState) return;

	if (const FName* AnimName = StateAnimations.Find(NewState))
	{
		PlayAnimationByName(*AnimName);
	}
}

void UGorgeousCommonWidget::PlayAnimationByName(const FName& AnimName)
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
