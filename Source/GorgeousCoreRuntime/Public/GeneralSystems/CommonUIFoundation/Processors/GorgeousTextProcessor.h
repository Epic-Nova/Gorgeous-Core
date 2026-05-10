// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousTextProcessor.generated.h"

/**
 * Processor for updating text-based widgets.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousTextProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousTextProcessor();
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;
	virtual void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr) override;
};
