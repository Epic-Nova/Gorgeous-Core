// Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
// GorgeousVault — GorgeousAutoReplicationIrisBackend Implementation (Stub)
// ============================================================================
// Only compiled when GORGEOUSCORE_WITH_IRIS=1
// ============================================================================

#include "GorgeousVaultAPI.h"

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — Iris Backend
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_Iris_Initialize(GVaultHandle Self, GVaultWorldHandle World)
{
    return GVault_Error_NotImplemented;
}

void GVault_Iris_Reset(GVaultHandle Self)
{
    // TODO: Implement
}

int32_t GVault_Iris_RegisterStream(GVaultHandle Self, GVaultOVHandle Variable,
                                    const void* Config, const void* StreamGuid)
{
    return GVault_Error_NotImplemented;
}

void GVault_Iris_UnregisterStream(GVaultHandle Self, GVaultOVHandle Variable)
{
    // TODO: Implement
}

void GVault_Iris_MarkStreamDirty(GVaultHandle Self, GVaultOVHandle Variable)
{
    // TODO: Implement
}

void GVault_Iris_Tick(GVaultHandle Self, float DeltaSeconds)
{
    // TODO: Implement
}
