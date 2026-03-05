// Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
// GorgeousVault — License Helper Implementation (Vault-Side)
// ============================================================================
// This file implements the license helper logic INSIDE the Vault DLL.
// The open-source FGorgeousCoreLicenseHelper in GorgeousCoreRuntimeUtilities
// becomes a thin proxy that forwards here.
//
// This is the real implementation of:
//   - Encrypted license file creation (per-plugin)
//   - License file decryption and validation
//   - License entry management
//   - Plugin License Registry (tracks per-plugin license status)
//   - Cascading death enforcement (clears function ref table on ANY failure)
//
// The PluginHelper (UGorgeousPluginHelper) on the proxy side discovers
// sibling plugins and passes them to the Vault via
// GVault_License_RegisterSiblingPlugins(). The Vault then validates
// each plugin's license and builds the Plugin License Registry.
// ============================================================================

#include "GorgeousVaultAPI.h"

#include <stdint.h>
#include <string.h>

// ── Internal state ───────────────────────────────────────────────────────
namespace GVaultLicenseInternal
{
    // ── Plugin License Registry ──────────────────────────────────────────
    // Simple fixed-size registry for sibling plugins.
    // In the real build this would use a more sophisticated structure;
    // for the skeleton we keep it plain-C compatible.

    static constexpr int32_t MaxSiblingPlugins = 64;

    struct FPluginLicenseEntry
    {
        char PluginName[256];
        int32_t Status; // 0 = unknown, 1 = valid, -1 = invalid, -2 = expired, -3 = missing
    };

    static FPluginLicenseEntry PluginRegistry[MaxSiblingPlugins] = {};
    static int32_t RegisteredPluginCount = 0;

    static bool bRegistryEvaluated = false;
    static bool bCascadingDeathTriggered = false;

    // ── License file helpers ─────────────────────────────────────────────
    // Placeholder SharedSecret — replaced during CI.
    static const uint8_t SharedSecret[32] = { 0 };

    // Find a plugin entry by name, returns index or -1
    int32_t FindPluginIndex(const char* PluginName)
    {
        for (int32_t i = 0; i < RegisteredPluginCount; ++i)
        {
            if (strcmp(PluginRegistry[i].PluginName, PluginName) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    // Reset the registry to a clean state
    void ResetRegistry()
    {
        memset(PluginRegistry, 0, sizeof(PluginRegistry));
        RegisteredPluginCount = 0;
        bRegistryEvaluated = false;
        bCascadingDeathTriggered = false;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Exported: License Helper — File Operations
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_License_CreateEncryptedFile(const char* LicenseData, int32_t DataLen,
                                            const char* ProjectId, const char* PluginName)
{
    if (!LicenseData || !ProjectId || !PluginName || DataLen <= 0)
    {
        return GVault_Error_InvalidHandle;
    }

    // TODO: Phase 4 implementation:
    // 1. Derive AES key = HKDF-SHA256(SharedSecret, SHA-256(ProjectId || PluginName),
    //                                  "gorgeous-plugin-license-v1", 32)
    // 2. Derive IV = first 12 bytes of SHA-256(ProjectId || PluginName || "iv-derivation")
    // 3. AES-256-GCM encrypt the LicenseData
    // 4. Write the encrypted blob to <ProjectDir>/Saved/GorgeousLicense/<PluginName>.glic
    // 5. Return GVault_OK on success

    return GVault_Error_NotImplemented;
}

int32_t GVault_License_ReadAndDecryptFile(const char* ProjectId, const char* PluginName,
                                           char* OutData, int32_t* OutDataLen)
{
    if (!ProjectId || !PluginName || !OutData || !OutDataLen)
    {
        return GVault_Error_InvalidHandle;
    }

    // TODO: Phase 4 implementation:
    // 1. Read <PluginName>.glic from disk
    // 2. Derive AES key using HKDF(SharedSecret, ProjectId || PluginName)
    // 3. AES-256-GCM decrypt
    // 4. Copy decrypted data to OutData, set *OutDataLen
    // 5. Return GVault_OK on success

    return GVault_Error_NotImplemented;
}

int32_t GVault_License_AddEntry(const char* EntryString, const char* ProjectId, const char* PluginName)
{
    if (!EntryString || !ProjectId || !PluginName)
    {
        return GVault_Error_InvalidHandle;
    }

    // TODO: Phase 4 implementation:
    // 1. Read existing entries via GVault_License_ReadAndDecryptFile
    // 2. Append new timestamped entry
    // 3. Re-encrypt and write back
    // 4. Return GVault_OK on success

    return GVault_Error_NotImplemented;
}

int32_t GVault_License_ReadAllEntries(const char* ProjectId, const char* PluginName,
                                       void* OutEntries, int32_t* OutEntryCount)
{
    if (!ProjectId || !PluginName || !OutEntries || !OutEntryCount)
    {
        return GVault_Error_InvalidHandle;
    }

    // TODO: Phase 4 implementation:
    // 1. Read and decrypt the license file
    // 2. Parse timestamp|value entries
    // 3. Fill OutEntries array
    // 4. Return GVault_OK on success

    return GVault_Error_NotImplemented;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Exported: Sibling Plugin Discovery & Registry
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_License_RegisterSiblingPlugins(const char** PluginNames, int32_t PluginCount)
{
    if (!PluginNames || PluginCount <= 0)
    {
        // No sibling plugins — GorgeousCore is running alone. No licensing needed.
        GVaultLicenseInternal::ResetRegistry();
        return GVault_OK;
    }

    if (PluginCount > GVaultLicenseInternal::MaxSiblingPlugins)
    {
        return GVault_Error_InvalidHandle;
    }

    GVaultLicenseInternal::ResetRegistry();

    for (int32_t i = 0; i < PluginCount; ++i)
    {
        if (PluginNames[i])
        {
            auto& Entry = GVaultLicenseInternal::PluginRegistry[GVaultLicenseInternal::RegisteredPluginCount];
            strncpy(Entry.PluginName, PluginNames[i], sizeof(Entry.PluginName) - 1);
            Entry.PluginName[sizeof(Entry.PluginName) - 1] = '\0';
            Entry.Status = 0; // Unknown — not yet validated
            GVaultLicenseInternal::RegisteredPluginCount++;
        }
    }

    return GVault_OK;
}

int32_t GVault_License_GetPluginCount(void)
{
    return GVaultLicenseInternal::RegisteredPluginCount;
}

int32_t GVault_License_GetPluginStatus(const char* PluginName)
{
    if (!PluginName)
    {
        return GVault_Error_InvalidHandle;
    }

    int32_t Index = GVaultLicenseInternal::FindPluginIndex(PluginName);
    if (Index < 0)
    {
        return -3; // Missing — not registered
    }

    return GVaultLicenseInternal::PluginRegistry[Index].Status;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Exported: Per-Plugin License Validation
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_License_ValidatePlugin(const char* EncryptedLicPath, const char* ProjectId,
                                       const char* PluginName)
{
    if (!ProjectId || !PluginName)
    {
        return GVault_Error_LicenseInvalid;
    }

    int32_t Index = GVaultLicenseInternal::FindPluginIndex(PluginName);
    if (Index < 0)
    {
        // Plugin not registered as sibling — can't validate
        return GVault_Error_InvalidHandle;
    }

    // TODO: Phase 4 implementation:
    // 1. Read <PluginName>.glic from EncryptedLicPath
    // 2. Derive AES key = HKDF-SHA256(SharedSecret, SHA-256(ProjectId || PluginName),
    //                                   "gorgeous-plugin-license-v1", 32)
    // 3. Decrypt AES-256-GCM → get JWT intermediate key
    // 4. Validate JWT signature (server's public key)
    // 5. Check plugin_name claim matches PluginName parameter
    // 6. Check expiry timestamp
    // 7. Update registry: PluginRegistry[Index].Status = 1 (valid) or -1/-2
    // 8. Return GVault_OK / GVault_Error_LicenseExpired / GVault_Error_LicenseInvalid

    GVaultLicenseInternal::PluginRegistry[Index].Status = GVault_Error_NotImplemented;
    return GVault_Error_NotImplemented;
}

int32_t GVault_License_ValidatePluginOffline(const char* OfflineLicPath, const char* ProjectId,
                                              const char* PluginName)
{
    if (!ProjectId || !PluginName)
    {
        return GVault_Error_LicenseInvalid;
    }

    int32_t Index = GVaultLicenseInternal::FindPluginIndex(PluginName);
    if (Index < 0)
    {
        return GVault_Error_InvalidHandle;
    }

    // TODO: Phase 4 implementation:
    // 1. Read <PluginName>.golic
    // 2. Same AES decryption with plugin-scoped derivation
    // 3. Validate dual JWT signatures
    // 4. Verify hardware_fingerprint + plugin_name claims
    // 5. Check system clock drift
    // 6. Check expiry (365-day window)
    // 7. Update registry status

    GVaultLicenseInternal::PluginRegistry[Index].Status = GVault_Error_NotImplemented;
    return GVault_Error_NotImplemented;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Exported: Registry Evaluation & Cascading Death
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_License_EvaluateRegistry(void)
{
    // If no sibling plugins are registered, everything is fine
    // (GorgeousCore alone = free, no licensing)
    if (GVaultLicenseInternal::RegisteredPluginCount == 0)
    {
        GVaultLicenseInternal::bRegistryEvaluated = true;
        return GVault_OK;
    }

    // Check ALL registered plugins
    bool bAnyInvalid = false;
    for (int32_t i = 0; i < GVaultLicenseInternal::RegisteredPluginCount; ++i)
    {
        int32_t Status = GVaultLicenseInternal::PluginRegistry[i].Status;
        if (Status != 1) // 1 = valid
        {
            bAnyInvalid = true;
            // Don't break — we want to log ALL offending plugins
        }
    }

    GVaultLicenseInternal::bRegistryEvaluated = true;

    if (bAnyInvalid)
    {
        // ═══════════════════════════════════════════════════
        // CASCADING DEATH — signal the caller to clear
        // the function reference table
        // ═══════════════════════════════════════════════════
        GVaultLicenseInternal::bCascadingDeathTriggered = true;
        return GVault_Error_LicenseInvalid;
    }

    return GVault_OK;
}

int32_t GVault_License_IsCascadingDeathActive(void)
{
    return GVaultLicenseInternal::bCascadingDeathTriggered ? 1 : 0;
}

int32_t GVault_License_GetRegistrySnapshot(void* OutNames, int32_t* OutStatuses, int32_t* OutCount)
{
    if (!OutCount)
    {
        return GVault_Error_InvalidHandle;
    }

    *OutCount = GVaultLicenseInternal::RegisteredPluginCount;

    if (OutNames && OutStatuses)
    {
        const char** Names = (const char**)OutNames;
        for (int32_t i = 0; i < GVaultLicenseInternal::RegisteredPluginCount; ++i)
        {
            Names[i] = GVaultLicenseInternal::PluginRegistry[i].PluginName;
            OutStatuses[i] = GVaultLicenseInternal::PluginRegistry[i].Status;
        }
    }

    return GVault_OK;
}
