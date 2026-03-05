// Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
// GorgeousVault — DLL Entry Point & Lifecycle
// ============================================================================
// This file handles DLL load/unload and Vault-wide initialization.
// ============================================================================

#include "GorgeousVaultAPI.h"

// Forward declarations for internal subsystem init
namespace GVaultInternal
{
    void InitializeAntiTamper();
    void ShutdownAntiTamper();
    void InitializeLicenseSystem();
    void ShutdownLicenseSystem();
    void InitializeChallengeResponse();
    void ShutdownChallengeResponse();
}

// ── Global state ─────────────────────────────────────────────────────────
static bool bVaultInitialized = false;

// ═══════════════════════════════════════════════════════════════════════════
//  Exported Lifecycle Functions
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_Initialize(void)
{
    if (bVaultInitialized)
    {
        return GVault_OK;
    }

    // Phase 1: Anti-tamper self-check
    GVaultInternal::InitializeAntiTamper();

    // Phase 2: License system
    GVaultInternal::InitializeLicenseSystem();

    // Phase 3: Challenge-response subsystem
    GVaultInternal::InitializeChallengeResponse();

    bVaultInitialized = true;
    return GVault_OK;
}

void GVault_Shutdown(void)
{
    if (!bVaultInitialized)
    {
        return;
    }

    GVaultInternal::ShutdownChallengeResponse();
    GVaultInternal::ShutdownLicenseSystem();
    GVaultInternal::ShutdownAntiTamper();

    bVaultInitialized = false;
}

int32_t GVault_GetVersion(void)
{
    return (GORGEOUS_VAULT_VERSION_MAJOR * 10000)
         + (GORGEOUS_VAULT_VERSION_MINOR * 100)
         + GORGEOUS_VAULT_VERSION_PATCH;
}

// ── DLL entry point (Windows) ────────────────────────────────────────────
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        // Vault initialization is deferred to GVault_Initialize()
        // which is called explicitly by the VaultLoader.
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif
