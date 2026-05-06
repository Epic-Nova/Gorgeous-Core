// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousCommonBorderProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonBorder.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"

UGorgeousCommonBorderProcessor::UGorgeousCommonBorderProcessor()
{
	TargetWidgetClass = UGorgeousCommonBorder::StaticClass();
}

void UGorgeousCommonBorderProcessor::OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload)
{
	UGorgeousCommonBorder* Border = Cast<UGorgeousCommonBorder>(Widget);
	if (!Border) return;

	const FGorgeousUIUpdatePayload* MasterPayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!MasterPayload) return;

	for (const auto& Pair : MasterPayload->Updates)
	{
		FName PropertyName = Pair.Key;
		const FInstancedStruct& Value = Pair.Value;

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
