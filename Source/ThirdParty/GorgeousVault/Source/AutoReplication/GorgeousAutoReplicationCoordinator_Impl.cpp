// Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
// GorgeousVault — GorgeousAutoReplicationCoordinator Implementation (Stub)
// ============================================================================

#include "GorgeousVaultAPI.h"

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — Coordinator
// ═══════════════════════════════════════════════════════════════════════════

GVaultHandle GVault_Coordinator_Get(GVaultWorldHandle World)
{
    // TODO: Implement per-world coordinator retrieval/creation
    return nullptr;
}

int32_t GVault_Coordinator_Initialize(GVaultHandle Self, GVaultWorldHandle World)
{
    return GVault_Error_NotImplemented;
}

void GVault_Coordinator_TearDown(GVaultHandle Self)
{
    // TODO: Implement
}

int32_t GVault_Coordinator_RegisterVariable(GVaultHandle Self, GVaultOVHandle Variable,
                                             const void* StreamConfig)
{
    return GVault_Error_NotImplemented;
}

void GVault_Coordinator_UnregisterVariable(GVaultHandle Self, GVaultOVHandle Variable)
{
    // TODO: Implement
}

void GVault_Coordinator_NotifyRPCBroadcast(GVaultHandle Self, const void* QueuedRPC,
                                            GVaultOVHandle TargetVariable)
{
    // TODO: Implement
}

void GVault_Coordinator_Tick(GVaultHandle Self, float DeltaSeconds)
{
    // TODO: Implement
}
