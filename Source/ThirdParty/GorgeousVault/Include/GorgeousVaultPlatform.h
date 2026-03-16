// Copyright (c) 2026-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              GorgeousVault — Platform Configuration Header                |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026-2026 Gorgeous Things by Simsalabim Studios,    |
|              administrated by Epic Nova. All rights reserved.             |
<==========================================================================*/
#pragma once

// ── Export/Import Macro ──────────────────────────────────────────────────
// When building the Vault DLL (CMake), GORGEOUS_VAULT_EXPORTS is defined.
// When consuming (UE module side), it is NOT defined → symbols are imported.
#if defined(_WIN32) || defined(_WIN64)
    #ifdef GORGEOUS_VAULT_EXPORTS
        #define VAULT_API __declspec(dllexport)
    #else
        #define VAULT_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #ifdef GORGEOUS_VAULT_EXPORTS
        #define VAULT_API __attribute__((visibility("default")))
    #else
        #define VAULT_API
    #endif
#else
    #define VAULT_API
#endif

// ── Vault Version ────────────────────────────────────────────────────────
#define GORGEOUS_VAULT_VERSION_MAJOR 1
#define GORGEOUS_VAULT_VERSION_MINOR 0
#define GORGEOUS_VAULT_VERSION_PATCH 0

// ── Opaque Handles ───────────────────────────────────────────────────────
// The proxy layer passes `this` pointers as opaque void* handles.
// The Vault casts them back to the real UE types internally
// (it links against the same UE headers at build time).
typedef void* GVaultOVHandle;       // UGorgeousObjectVariable*
typedef void* GVaultHandle;         // Generic opaque handle (UObject*, etc.)
typedef void* GVaultWorldHandle;    // UWorld*
typedef void* GVaultPayloadHandle;  // Serialized payload pointer

// ── Result Codes ─────────────────────────────────────────────────────────
typedef enum EGVaultResult
{
    GVault_OK                       = 0,
    GVault_Error_NotInitialized     = -1,
    GVault_Error_InvalidHandle      = -2,
    GVault_Error_IntegrityFailed    = -3,
    GVault_Error_LicenseInvalid     = -4,
    GVault_Error_LicenseExpired     = -5,
    GVault_Error_TamperDetected     = -6,
    GVault_Error_ChallengeFailed    = -7,
    GVault_Error_NotImplemented     = -99,
} EGVaultResult;
