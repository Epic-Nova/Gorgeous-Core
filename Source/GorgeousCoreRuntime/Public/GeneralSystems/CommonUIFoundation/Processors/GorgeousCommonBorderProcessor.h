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
	virtual void OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload) override;
};
