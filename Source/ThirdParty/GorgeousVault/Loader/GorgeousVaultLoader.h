// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousVaultPlatform.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

/**
 * Singleton that manages the GorgeousVault DLL lifecycle.
 *
 * The proxy layer (.cpp files in GorgeousCoreRuntime) uses this to resolve
 * and call into the Vault's C-linkage exported functions.
 *
 * Thread safety: The loader is initialized once during module startup.
 * After initialization, Resolve() is safe to call from any thread
 * (it only reads cached function pointers).
 */
class GORGEOUSCORERUNTIME_API FGorgeousVaultLoader
{
public:
    /** Get the singleton instance. Creates and loads the Vault on first call. */
    static FGorgeousVaultLoader& Get();

    /** Returns true if the Vault DLL is loaded and integrity-verified. */
    bool IsLoaded() const;

    /** Returns true if the Vault passed its integrity self-check. */
    bool IsIntegrityVerified() const;

    /**
     * Resolve a C-linkage function pointer from the Vault DLL.
     *
     * Usage:
     *   auto Fn = FGorgeousVaultLoader::Get().Resolve<decltype(&GVault_OV_BeginDestroy)>("GVault_OV_BeginDestroy");
     *   if (Fn) { Fn(this); }
     *
     * @param FunctionName  The exported symbol name (e.g., "GVault_OV_BeginDestroy")
     * @return The typed function pointer, or nullptr if not found / Vault not loaded
     */
    template<typename FuncPtr>
    FuncPtr Resolve(const char* FunctionName)
    {
        return reinterpret_cast<FuncPtr>(ResolveRaw(FunctionName));
    }

    /** Unload the Vault DLL. Called during module shutdown. */
    void Shutdown();

    /**
     * Trigger the self-destruct sequence.
     * Called when a security check fails (challenge-response, canary, integrity).
     * This zeros out all cached function pointers and unloads the DLL.
     */
    void TriggerSelfDestruct();

    /**
     * Clear all cached function pointers (cascading death).
     * Called when the Plugin License Registry evaluation fails.
     * Unlike TriggerSelfDestruct, the DLL stays loaded so licenses
     * can be re-validated without an editor restart.
     */
    void ClearAllCachedFunctions();

    /**
     * Re-populate cached function pointers after license resolution.
     * Called when all sibling plugin licenses become valid again.
     * Returns true if the Vault was successfully re-armed.
     */
    bool RearmFunctionCache();

    /**
     * Run the sibling plugin license evaluation flow.
     * Queries UGorgeousPluginHelper for known sibling plugins,
     * passes them to the Vault, validates each license, evaluates
     * the registry, and triggers cascading death if needed.
     *
     * @param ProjectId  The project ID for AES key derivation
     */
    void EvaluateSiblingPluginLicenses(const FString& ProjectId);

private:
    FGorgeousVaultLoader();
    ~FGorgeousVaultLoader();

    // Non-copyable
    FGorgeousVaultLoader(const FGorgeousVaultLoader&) = delete;
    FGorgeousVaultLoader& operator=(const FGorgeousVaultLoader&) = delete;

    /** Load the Vault DLL from the expected platform-specific path. */
    void LoadVault();

    /** Run the Vault's integrity self-check (GVault_ValidateIntegrity). */
    void VerifyIntegrity();

    /** Initialize the challenge-response rotation system. */
    void InitializeChallengeRotation();

    /** Resolve a raw function pointer by name. */
    void* ResolveRaw(const char* FunctionName);

    /** The DLL handle returned by FPlatformProcess::GetDllHandle. */
    void* DllHandle;

    /** Whether the Vault passed its integrity self-check. */
    bool bIntegrityVerified;

    /** Whether the Vault has been fully initialized (Initialize() called). */
    bool bVaultInitialized;

    /** Whether a self-destruct has been triggered (all pointers zeroed). */
    bool bSelfDestructed;

    /** Whether cascading death is active (function pointers cleared due to license failure). */
    bool bCascadingDeath;

    /** Cached function pointers for performance (avoid repeated GetDllExport). */
    TMap<FString, void*> CachedFunctions;

    /** Critical section for thread-safe initialization. */
    FCriticalSection InitLock;
};