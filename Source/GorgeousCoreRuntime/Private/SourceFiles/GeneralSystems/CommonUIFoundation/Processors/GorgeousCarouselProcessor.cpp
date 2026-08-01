// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousCarouselProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonCarousel.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"

UGorgeousCarouselProcessor::UGorgeousCarouselProcessor()
{
	TargetWidgetClass = UGorgeousCommonCarousel::StaticClass();
}

void UGorgeousCarouselProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	UGorgeousCommonCarousel* Carousel = Cast<UGorgeousCommonCarousel>(Widget);
	if (!Carousel) return;

	const FGorgeousUIUpdatePayload* UpdatePayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!UpdatePayload) return;

	// 1. Navigation Logic
	if (SignalTag == TAG_Gorgeous_UI_Signal_Carousel_Next)
	{
		Carousel->NextPage();
	}
	else if (SignalTag == TAG_Gorgeous_UI_Signal_Carousel_Prev)
	{
		Carousel->PreviousPage();
	}

	// 2. Child Injection Logic
	for (auto& Pair : UpdatePayload->Updates)
	{
		if (const FGorgeousPanelUpdatePayload* PanelPayload = Pair.Value.GetPtr<FGorgeousPanelUpdatePayload>())
		{
			if (PanelPayload->ChildWidget)
			{
				if (PanelPayload->bClearChildren)
				{
					Carousel->ClearChildren();
				}
				Carousel->AddChild(PanelPayload->ChildWidget);
			}
			else if (PanelPayload->WidgetToRemove)
			{
				Carousel->RemoveChild(PanelPayload->WidgetToRemove);
			}
		}
	}

	// 3. Fallback to base reflection (for properties like ActiveIndex)
	Super::OnSignalReceived(Widget, SignalTag, Payload);
}
