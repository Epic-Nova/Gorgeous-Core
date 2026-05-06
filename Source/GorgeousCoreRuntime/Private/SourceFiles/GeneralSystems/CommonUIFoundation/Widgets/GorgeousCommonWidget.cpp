// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidget.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/CommonUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "QualityOfLife/GorgeousPlayerController.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonWidget)

void UGorgeousCommonWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_UI_REGISTER_WIDGET()
}

void UGorgeousCommonWidget::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
	Super::NativeDestruct();
}

void UGorgeousCommonWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsInterpTheme)
	{
		bool bStillInterp = false;
		for (auto& Pair : TargetThemeColors)
		{
			FLinearColor& Current = CurrentThemeColors.FindOrAdd(Pair.Key);
			const FLinearColor& Target = Pair.Value;

			if (!Current.Equals(Target, 0.001f))
			{
				Current = FMath::CInterpTo(Current, Target, InDeltaTime, ThemeInterpSpeed);
				bStillInterp = true;
			}
			else
			{
				Current = Target; // Snap to avoid infinite near-zero interp
			}

			// Apply the interpolated color directly to the widget via reflection
			FInstancedStruct ColorPayload = FInstancedStruct::Make(Current);
			UGorgeousUIProcessor::ApplyPropertyToTarget(this, Pair.Key, ColorPayload);
		}

		if (!bStillInterp)
		{
			bIsInterpTheme = false;
			// Tick control removed; rely on widget tick configuration
		}
	}
}

#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "Animation/WidgetAnimation.h"

void UGorgeousCommonWidget::OnThemeApplied(const UGorgeousUITheme_DA* Theme)
{
	if (!Theme) return;

	// 1. Update Action Icons based on current platform/theme
	UpdateActionIcon();

	// 2. Capture and Interpolate Theme Colors
	for (auto& Pair : Theme->StyleProperties)
	{
		if (const FLinearColor* ColorPtr = Pair.Value.GetPtr<FLinearColor>())
		{
			TargetThemeColors.Add(Pair.Key, *ColorPtr);
			bIsInterpTheme = true;
			// Tick control removed; rely on widget tick configuration
		}
		else
		{
			// 2. Reflective Magic: Automatically apply non-color properties (Fonts, Sizes, etc.)
			UGorgeousUIProcessor::ApplyPropertyToTarget(this, Pair.Key, Pair.Value);
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
				// Try the property first; if that fails try the setter — never both.
				if (!UGorgeousUIProcessor::ApplyPropertyToTarget(this, "Brush", Payload))
				{
					UGorgeousUIProcessor::ApplyPropertyToTarget(this, "Brush", Payload); // SetBrush via prefix rule
				}
			}
		}
	}
}

void UGorgeousCommonWidget::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	if (Theme)
	{
		OnThemeApplied(Theme);
	}
}

void UGorgeousCommonWidget::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Default Blueprint hook: no-op
}

void UGorgeousCommonWidget::OnStateSwitched_Implementation(UCommonUIState_DA* NewState)
{
	if (!NewState) return;

	if (FName* AnimName = StateAnimations.Find(NewState))
	{
		if (!AnimName->IsNone())
			{
				PlayAnimationByName(*AnimName);
			}
	}
}

void UGorgeousCommonWidget::PlayAnimationByName(const FName& AnimName)
{
	if (AnimName.IsNone()) return;

	// Try to resolve a UWidgetAnimation property on this widget by name
	UWidgetAnimation* Found = FindObject<UWidgetAnimation>(GetClass(), *AnimName.ToString());
	if (Found)
	{
		PlayAnimation(Found);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("PlayAnimationByName: animation '%s' not found on %s"), *AnimName.ToString(), *GetName());
	}
}
