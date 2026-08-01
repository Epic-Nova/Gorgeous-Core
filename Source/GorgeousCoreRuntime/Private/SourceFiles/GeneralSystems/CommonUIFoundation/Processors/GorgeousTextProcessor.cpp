// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousTextProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonTextBlock.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

UGorgeousTextProcessor::UGorgeousTextProcessor()
{
	TargetWidgetClass = UGorgeousCommonTextBlock::StaticClass();
}

void UGorgeousTextProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	UGorgeousCommonTextBlock* TextBlock = Cast<UGorgeousCommonTextBlock>(Widget);
	if (!TextBlock) return;

	const FGorgeousUIUpdatePayload* UpdatePayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!UpdatePayload) return;

	for (const auto& Pair : UpdatePayload->Updates)
	{
		if (!UGorgeousUIProcessor::IsStylePropertyAllowed(TextBlock, Pair.Key))
		{
			continue;
		}

		if (Pair.Key == FName("Text"))
		{
			FText TextOut;
			if (GorgeousUIInstanced::TryGetText(Pair.Value, TextOut))
			{
				TextBlock->SetText(TextOut);
				continue;
			}
		}
		else if (Pair.Key == FName("Color") || Pair.Key == FName("ColorAndOpacity"))
		{
			if (const FLinearColor* ColorValue = Pair.Value.GetPtr<FLinearColor>())
			{
				TextBlock->SetColorAndOpacity(FSlateColor(*ColorValue));
				continue;
			}
		}

		// Fallback to universal reflection for any other property
		ApplyStylePropertyToTarget(TextBlock, Pair.Key, Pair.Value);
	}
}

void UGorgeousTextProcessor::ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme)
{
	UGorgeousCommonTextBlock* TextBlock = Cast<UGorgeousCommonTextBlock>(Widget);
	if (!TextBlock)
	{
		Super::ApplyThemeToWidget(Widget, PrimaryTheme, FallbackTheme);
		return;
	}

	ApplyThemeToWidgetInternal(Widget, PrimaryTheme, FallbackTheme);

	if (PrimaryTheme && TextBlock->TypographyTag.IsValid())
	{
		FGorgeousUITypography_S TypeInfo = PrimaryTheme->GetTypography(TextBlock->TypographyTag);
		if (IsStylePropertyAllowed(TextBlock, "Font"))
		{
			TextBlock->SetFont(TypeInfo.Font);
		}
		if (IsStylePropertyAllowed(TextBlock, "ColorAndOpacity"))
		{
			TextBlock->SetColorAndOpacity(FSlateColor(TypeInfo.Color));
		}
	}
}
