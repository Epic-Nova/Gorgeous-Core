// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousCommonRichTextProcessor.generated.h"

/**
 * Processor for updating Rich Text Blocks.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonRichTextProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousCommonRichTextProcessor();
	virtual void OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload) override;
};
