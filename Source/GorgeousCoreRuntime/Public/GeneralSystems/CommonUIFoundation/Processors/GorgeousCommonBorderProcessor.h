// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousCommonBorderProcessor.generated.h"

/**
 * Processor for updating Borders.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonBorderProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousCommonBorderProcessor();
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;
	virtual void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr) override;
};
