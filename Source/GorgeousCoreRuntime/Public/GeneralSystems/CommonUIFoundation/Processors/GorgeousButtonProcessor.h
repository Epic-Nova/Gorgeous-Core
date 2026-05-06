// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousButtonProcessor.generated.h"

/**
 * Processor for updating buttons.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousButtonProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousButtonProcessor();
	virtual void OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload) override;
};
