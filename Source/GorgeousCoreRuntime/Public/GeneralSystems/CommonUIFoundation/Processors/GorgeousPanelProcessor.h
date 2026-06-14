// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousPanelProcessor.generated.h"

/**
 * Smart processor for all Panel Widgets (VerticalBox, HorizontalBox, Overlay, etc.).
 * Handles dynamic child injection and cleanup.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPanelProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousPanelProcessor();
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;
};
