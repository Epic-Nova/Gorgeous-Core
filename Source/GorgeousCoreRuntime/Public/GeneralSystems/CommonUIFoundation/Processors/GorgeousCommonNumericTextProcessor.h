// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousCommonNumericTextProcessor.generated.h"

/**
 * Processor for updating Numeric Text Blocks.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonNumericTextProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousCommonNumericTextProcessor();
	virtual void OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload) override;
};
