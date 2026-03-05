// Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
// GorgeousVault — License System Internal State
// ============================================================================
// This file holds shared license system state (SharedSecret, init/shutdown).
// All per-plugin validation logic lives in:
//   Source/LicenseHelper/GorgeousLicenseHelper_Impl.cpp
// ============================================================================

#include "GorgeousVaultAPI.h"

#include <stdint.h>
#include <string.h>

// ── Internal state ───────────────────────────────────────────────────────
namespace GVaultInternal
{
    static bool bLicenseSystemInitialized = false;
    static bool bLicenseValid = false;

    // The SharedSecret is the crown jewel — protected by obfuscation.
    // In the real implementation, this is split across multiple functions
    // and assembled at runtime behind obfuscation passes.
    // PLACEHOLDER: 32 zero bytes (will be replaced during CI pipeline).
    static const uint8_t SharedSecret[32] = { 0 };

    void InitializeLicenseSystem()
    {
        if (bLicenseSystemInitialized)
        {
            return;
        }

        // TODO: Phase 4 implementation:
        // 1. Locate .glic / .golic
        // 2. Derive AES key via HKDF(SharedSecret, ProjectID)
        // 3. Decrypt → extract JWT
        // 4. Validate JWT signature
        // 5. Check expiry

        bLicenseSystemInitialized = true;
    }

    void ShutdownLicenseSystem()
    {
        // Zero out sensitive state
        bLicenseValid = false;
        bLicenseSystemInitialized = false;
    }
}