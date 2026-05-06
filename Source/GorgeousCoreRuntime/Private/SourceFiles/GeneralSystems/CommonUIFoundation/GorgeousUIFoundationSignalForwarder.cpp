// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSignalForwarder.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"

void UGorgeousSignalBridgeTagForwarder::OnBridgePayload(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
    if (OwningSubsystem)
    {
        OwningSubsystem->HandlePayloadForTag(Tag, Payload);
    }
}
