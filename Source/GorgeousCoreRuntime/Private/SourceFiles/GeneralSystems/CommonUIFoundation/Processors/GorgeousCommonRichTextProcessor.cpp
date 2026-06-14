// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousCommonRichTextProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonRichTextBlock.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"

UGorgeousCommonRichTextProcessor::UGorgeousCommonRichTextProcessor()
{
	TargetWidgetClass = UGorgeousCommonRichTextBlock::StaticClass();
}

void UGorgeousCommonRichTextProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	UGorgeousCommonRichTextBlock* TextBlock = Cast<UGorgeousCommonRichTextBlock>(Widget);
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

		if (PropertyName == "Text")
		{
			FText TextValueOut;
			if (GorgeousUIInstanced::TryGetText(Value, TextValueOut))
			{
				TextBlock->SetText(TextValueOut);
			}
		}
	}
}
