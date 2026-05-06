// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousButtonProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonButton.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"

UGorgeousButtonProcessor::UGorgeousButtonProcessor()
{
	TargetWidgetClass = UGorgeousCommonButton::StaticClass();
}

void UGorgeousButtonProcessor::OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload)
{
	UGorgeousCommonButton* Button = Cast<UGorgeousCommonButton>(Widget);
	if (!Button) return;

	const FGorgeousUIUpdatePayload* UpdatePayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!UpdatePayload) return;

	for (const auto& Pair : UpdatePayload->Updates)
	{
		if (Pair.Key == FName("Enabled") || Pair.Key == FName("IsEnabled"))
		{
			bool bEnabledVal = false;
			if (GorgeousUIInstanced::TryGetBool(Pair.Value, bEnabledVal))
			{
				Button->SetIsEnabled(bEnabledVal);
				continue;
			}
		}

		// Fallback to universal reflection for any other property
		ApplyPropertyToTarget(Button, Pair.Key, Pair.Value);
	}
}
