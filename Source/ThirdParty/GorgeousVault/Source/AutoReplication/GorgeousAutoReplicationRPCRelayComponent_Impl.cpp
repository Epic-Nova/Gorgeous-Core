// Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
// GorgeousVault — GorgeousAutoReplicationRPCRelayComponent Implementation (Stub)
// ============================================================================

#include "GorgeousVaultAPI.h"

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — RPC Relay Component
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_Relay_RelayResultToServer(GVaultHandle Self, const void* Result)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_Relay_RelayPropertyPayloadToServer(GVaultHandle Self, const void* Envelope,
                                                   GVaultHandle TargetMixin)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_Relay_RelayPropertyPayloadToClient(GVaultHandle Self, const void* Envelope)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_Relay_RelayRPCToServer(GVaultHandle Self, const void* QueuedRPC,
                                       int32_t bReliable, GVaultHandle TargetMixin)
{
    return GVault_Error_NotImplemented;
}
