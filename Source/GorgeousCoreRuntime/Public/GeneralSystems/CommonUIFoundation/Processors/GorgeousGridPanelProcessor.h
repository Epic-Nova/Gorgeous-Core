// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousGridPanelProcessor.generated.h"

/**
 * Advanced processor for Grid Panels.
 * Can handle dynamic slot management and child injection.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousGridPanelProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousGridPanelProcessor();
	virtual void OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload) override;
};
