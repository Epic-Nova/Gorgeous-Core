// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousCommonBorderProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonBorder.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

UGorgeousCommonBorderProcessor::UGorgeousCommonBorderProcessor()
{
	TargetWidgetClass = UGorgeousCommonBorder::StaticClass();
}

void UGorgeousCommonBorderProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	UGorgeousCommonBorder* Border = Cast<UGorgeousCommonBorder>(Widget);
	if (!Border) return;

	const FGorgeousUIUpdatePayload* MasterPayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!MasterPayload) return;

	for (const auto& Pair : MasterPayload->Updates)
	{
		FName PropertyName = Pair.Key;
		const FInstancedStruct& Value = Pair.Value;

		if (!UGorgeousUIProcessor::IsStylePropertyAllowed(Border, PropertyName))
		{
			continue;
		}

		if (PropertyName == "ContentColor")
		{
			if (const FLinearColor* ColorValue = Value.GetPtr<FLinearColor>())
			{
				Border->SetContentColorAndOpacity(*ColorValue);
			}
		}
		else if (PropertyName == "BrushColor")
		{
			if (const FLinearColor* ColorValue = Value.GetPtr<FLinearColor>())
			{
				Border->SetBrushColor(*ColorValue);
			}
		}
		else if (PropertyName == "Padding")
		{
			if (const FMargin* PaddingValue = Value.GetPtr<FMargin>())
			{
				Border->SetPadding(*PaddingValue);
			}
		}
	}
}

void UGorgeousCommonBorderProcessor::ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme)
{
	UGorgeousCommonBorder* Border = Cast<UGorgeousCommonBorder>(Widget);
	if (!Border)
	{
		Super::ApplyThemeToWidget(Widget, PrimaryTheme, FallbackTheme);
		return;
	}

	ApplyThemeToWidgetInternal(Widget, PrimaryTheme, FallbackTheme);

	if (!PrimaryTheme) return;

	if (const FInstancedStruct* BrushValue = PrimaryTheme->StyleProperties.Find("BorderBrush"))
	{
		ApplyStylePropertyToTarget(Border, "Brush", *BrushValue);
	}

	if (const FInstancedStruct* ColorValue = PrimaryTheme->StyleProperties.Find("BorderColor"))
	{
		if (const FLinearColor* ColorPtr = ColorValue->GetPtr<FLinearColor>())
		{
			Border->GetStartThemeColors().Add("BrushColor", Border->GetCurrentThemeColors().FindOrAdd("BrushColor", *ColorPtr));
			Border->GetTargetThemeColors().Add("BrushColor", *ColorPtr);
			Border->SetElapsedThemeTime(0.0f);
			Border->SetIsInterpTheme(true);
		}
	}
}
