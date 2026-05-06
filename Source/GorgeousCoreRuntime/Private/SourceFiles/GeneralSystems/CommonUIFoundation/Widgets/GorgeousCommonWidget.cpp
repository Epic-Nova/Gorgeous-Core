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
	if (Theme)
	{
		// 1. Update Action Icons based on current platform/theme
		UpdateActionIcon();

		// 2. Setup Theme Color Interpolation Targets
		for (auto& Pair : Theme->StyleProperties)
		{
			if (const FLinearColor* ColorPtr = Pair.Value.GetPtr<FLinearColor>())
			{
				StartThemeColors.Add(Pair.Key, CurrentThemeColors.FindOrAdd(Pair.Key, *ColorPtr));
				TargetThemeColors.Add(Pair.Key, *ColorPtr);
				ElapsedThemeTime = 0.0f;
				bIsInterpTheme = true;
			}
			else
			{
				// Reflective Magic: Automatically apply non-color properties (Fonts, Sizes, etc.)
				UGorgeousUIProcessor::ApplyPropertyToTarget(this, Pair.Key, Pair.Value);
			}
		}
	}
}

void UGorgeousCommonWidget::UpdateActionIcon()
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
				FInstancedStruct Payload = FInstancedStruct::Make(IconBrush);
				UGorgeousUIProcessor::ApplyPropertyToTarget(this, "Brush", Payload);
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
