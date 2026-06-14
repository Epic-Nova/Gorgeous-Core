// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousCommonNumericTextProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonNumericTextBlock.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"

UGorgeousCommonNumericTextProcessor::UGorgeousCommonNumericTextProcessor()
{
	TargetWidgetClass = UGorgeousCommonNumericTextBlock::StaticClass();
}

void UGorgeousCommonNumericTextProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	UGorgeousCommonNumericTextBlock* TextBlock = Cast<UGorgeousCommonNumericTextBlock>(Widget);
	if (!TextBlock) return;

	const FGorgeousUIUpdatePayload* MasterPayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!MasterPayload) return;

	for (const auto& Pair : MasterPayload->Updates)
	{
		FName PropertyName = Pair.Key;
		const FInstancedStruct& Value = Pair.Value;

		if (!UGorgeousUIProcessor::IsStylePropertyAllowed(TextBlock, PropertyName))
		{
			continue;
		}

		if (PropertyName == "Value" || PropertyName == "CurrentValue" || PropertyName == "TargetValue")
		{
			float FloatValueOut = 0.0f;
			if (GorgeousUIInstanced::TryGetFloat(Value, FloatValueOut))
			{
				TextBlock->SetCurrentValue(FloatValueOut);
			}
		}
	}
}
