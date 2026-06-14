// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousCarouselProcessor.generated.h"

/**
 * Processor for the Gorgeous Carousel widget.
 * Handles navigation and child injection via signals.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCarouselProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousCarouselProcessor();
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;
};
