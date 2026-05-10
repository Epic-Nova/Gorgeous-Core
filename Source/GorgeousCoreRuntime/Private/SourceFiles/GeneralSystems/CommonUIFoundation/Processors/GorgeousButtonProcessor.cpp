// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousButtonProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonButton.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"

UGorgeousButtonProcessor::UGorgeousButtonProcessor()
{
	TargetWidgetClass = UGorgeousCommonButton::StaticClass();
}

void UGorgeousButtonProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	UGorgeousCommonButton* Button = Cast<UGorgeousCommonButton>(Widget);
	if (!Button) return;

	const FGorgeousUIUpdatePayload* UpdatePayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!UpdatePayload) return;

	for (const auto& Pair : UpdatePayload->Updates)
	{
		if (!UGorgeousUIProcessor::IsStylePropertyAllowed(Button, Pair.Key))
		{
			continue;
		}

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
		ApplyStylePropertyToTarget(Button, Pair.Key, Pair.Value);
	}
}

void UGorgeousButtonProcessor::ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme)
{
	UGorgeousCommonButton* Button = Cast<UGorgeousCommonButton>(Widget);
	if (!Button)
	{
		Super::ApplyThemeToWidget(Widget, PrimaryTheme, FallbackTheme);
		return;
	}

	ApplyThemeToWidgetInternal(Widget, PrimaryTheme, FallbackTheme);

	// Reapply CommonUI style assets to ensure additive behavior.
	if (UCommonButtonStyle* Style = Button->GetStyle())
	{
		Button->SetStyle(Style->GetClass());
	}
}
