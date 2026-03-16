// Copyright (c) 2026-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              GorgeousVaultLoader — Runtime DLL Loader Implementation       |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026-2026 Gorgeous Things by Simsalabim Studios,    |
|              administrated by Epic Nova. All rights reserved.             |
<==========================================================================*/

#include "GorgeousVaultLoader.h"
#include "GorgeousVaultAPI.h"

#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"
#include "Logging/LogMacros.h"

// License evaluation needs the PluginHelper for sibling discovery
#include "Helpers/GorgeousPluginHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousVault, Log, All);

//@TODO: Multi version Engine loading support
// 1. Engine Compability
// 1.1 Multi Version DLLs: Some engine versions may not change much between them, allowing a single "generic" DLL to support multiple versions. The loader would attempt to load the generic DLL first, and if it fails (e.g., due to missing exports), it would fall back to version-specific DLLs.
// 1.1.1: The generic DLL would be placed under Prebuilt/<Platform>/ and named like GorgeousVault_Generic.dll. It would be compiled with the oldest supported Engine version's API to maximize compatibility.
// 1.2 Version-Specific DLLs: For engine versions with significant API changes, separate DLLs would be compiled against each version's specific API. The loader would detect the Engine version at runtime and load the appropriate DLL. This allows the Vault to take advantage of newer APIs in later Engine versions, but requires maintaining multiple versions of the Vault DLL.
// 1.2.1: These DLLs would be located under Prebuilt/<Platform>/Versioned/ and named like GorgeousVault_5.2.dll, GorgeousVault_5.3.dll, etc. The loader would construct the DLL name based on the detected Engine version and attempt to load it from that subdirectory.
// 1.3 API Versioning: The Vault DLL could expose an API version export (e.g., GVault_API_Version) that the loader checks after loading. If the API version is incompatible with what the loader expects, it can trigger a fallback to another DLL or disable functionality gracefully.
// 1.4 Patching: For minor Engine updates that break compatibility, the Vault team could release patched DLLs without changing the API version, allowing users to update the DLL without changing their code. The loader would just need to ensure it always loads the latest compatible DLL.

// ═══════════════════════════════════════════════════════════════════════════
//  Singleton
// ═══════════════════════════════════════════════════════════════════════════

FGorgeousVaultLoader& FGorgeousVaultLoader::Get()
{
    static FGorgeousVaultLoader Instance;
    return Instance;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════

FGorgeousVaultLoader::FGorgeousVaultLoader()
    : DllHandle(nullptr)
    , bIntegrityVerified(false)
    , bVaultInitialized(false)
    , bSelfDestructed(false)
    , bCascadingDeath(false)
{
    LoadVault();
}

FGorgeousVaultLoader::~FGorgeousVaultLoader()
{
    Shutdown();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Public API
// ═══════════════════════════════════════════════════════════════════════════

bool FGorgeousVaultLoader::IsLoaded() const
{
    return DllHandle != nullptr && !bSelfDestructed;
}

bool FGorgeousVaultLoader::IsIntegrityVerified() const
{
    return bIntegrityVerified && !bSelfDestructed;
}

void FGorgeousVaultLoader::Shutdown()
{
    FScopeLock Lock(&InitLock);

    if (DllHandle)
    {
        // Call GVault_Shutdown if available
        typedef void (*GVaultShutdownFn)(void);
        GVaultShutdownFn ShutdownFn = reinterpret_cast<GVaultShutdownFn>(
            FPlatformProcess::GetDllExport(DllHandle, TEXT("GVault_Shutdown")));
        if (ShutdownFn)
        {
            ShutdownFn();
        }

        FPlatformProcess::FreeDllHandle(DllHandle);
        DllHandle = nullptr;
    }

    CachedFunctions.Empty();
    bIntegrityVerified = false;
    bVaultInitialized = false;
}

void FGorgeousVaultLoader::TriggerSelfDestruct()
{
    UE_LOG(LogGorgeousVault, Error,
        TEXT("GorgeousVault: Security check failed. Vault functionality disabled."));

    FScopeLock Lock(&InitLock);

    // Zero all cached function pointers
    CachedFunctions.Empty();

    // Unload the DLL
    if (DllHandle)
    {
        FPlatformProcess::FreeDllHandle(DllHandle);
        DllHandle = nullptr;
    }

    bSelfDestructed = true;
    bIntegrityVerified = false;
    bVaultInitialized = false;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Private: DLL Loading
// ═══════════════════════════════════════════════════════════════════════════

void FGorgeousVaultLoader::LoadVault()
{
    FScopeLock Lock(&InitLock);

    if (DllHandle)
    {
        return; // Already loaded
    }

    // ── Determine the platform-specific DLL name ─────────────────────────
#if PLATFORM_WINDOWS
    const FString DllName = TEXT("GorgeousVault.dll");
#elif PLATFORM_LINUX
    const FString DllName = TEXT("libGorgeousVault.so");
#elif PLATFORM_MAC
    const FString DllName = TEXT("libGorgeousVault.dylib");
#else
    UE_LOG(LogGorgeousVault, Error,
        TEXT("GorgeousVault: Unsupported platform. Vault will not be loaded."));
    return;
#endif

    // ── Search paths ─────────────────────────────────────────────────────
    // 1. Same directory as the executable (RuntimeDependencies copies it here)
    // 2. The plugin's Binaries directory
    // 3. The ThirdParty/GorgeousVault/Prebuilt/<Platform>/ directory

    // Try loading from the standard DLL search path first
    // (RuntimeDependencies should place it next to the executable)
    DllHandle = FPlatformProcess::GetDllHandle(*DllName);

    if (!DllHandle)
    {
        // Fallback: try the plugin's Binaries directory
        FString PluginBinariesDir = FPaths::Combine(
            FPaths::ProjectPluginsDir(),
            TEXT("GorgeousCore"), TEXT("Binaries"),
            FPlatformProcess::GetBinariesSubdirectory());

        FString FullPath = FPaths::Combine(PluginBinariesDir, DllName);
        DllHandle = FPlatformProcess::GetDllHandle(*FullPath);
    }

    if (!DllHandle)
    {
        UE_LOG(LogGorgeousVault, Warning,
            TEXT("GorgeousVault: Could not load %s. Object Variable functionality will be unavailable. "
                 "This is expected during initial development when the Vault DLL has not been built yet."),
            *DllName);
        return;
    }

    UE_LOG(LogGorgeousVault, Log, TEXT("GorgeousVault: DLL loaded successfully."));

    // ── Initialize the Vault ─────────────────────────────────────────────
    typedef int32_t (*GVaultInitFn)(void);
    GVaultInitFn InitFn = reinterpret_cast<GVaultInitFn>(
        FPlatformProcess::GetDllExport(DllHandle, TEXT("GVault_Initialize")));

    if (InitFn)
    {
        int32_t Result = InitFn();
        if (Result == GVault_OK)
        {
            bVaultInitialized = true;
            UE_LOG(LogGorgeousVault, Log, TEXT("GorgeousVault: Initialized successfully."));
        }
        else
        {
            UE_LOG(LogGorgeousVault, Error,
                TEXT("GorgeousVault: Initialization failed with code %d."), Result);
        }
    }
    else
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: GVault_Initialize export not found. DLL may be corrupted."));
    }

    // ── Verify integrity ─────────────────────────────────────────────────
    VerifyIntegrity();

    // ── Start challenge-response rotation ────────────────────────────────
    if (bIntegrityVerified)
    {
        InitializeChallengeRotation();
    }
}

void FGorgeousVaultLoader::VerifyIntegrity()
{
    if (!DllHandle)
    {
        return;
    }

    typedef int32_t (*GVaultIntegrityFn)(void);
    GVaultIntegrityFn IntegrityFn = reinterpret_cast<GVaultIntegrityFn>(
        FPlatformProcess::GetDllExport(DllHandle, TEXT("GVault_ValidateIntegrity")));

    if (IntegrityFn)
    {
        int32_t Result = IntegrityFn();
        if (Result == GVault_OK)
        {
            bIntegrityVerified = true;
            UE_LOG(LogGorgeousVault, Log, TEXT("GorgeousVault: Integrity check passed."));
        }
        else
        {
            UE_LOG(LogGorgeousVault, Error,
                TEXT("GorgeousVault: Integrity check FAILED (code %d). DLL may have been tampered with."),
                Result);
            TriggerSelfDestruct();
        }
    }
    else
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: GVault_ValidateIntegrity export not found."));
    }
}

void FGorgeousVaultLoader::InitializeChallengeRotation()
{
    // TODO: Phase 3 implementation:
    // 1. Generate first random 32-byte challenge
    // 2. Call GVault_ChallengeResponse to get the first response
    // 3. Set up a ticker delegate that rotates challenges every ~30 seconds
    // 4. Each rotation: send challenge, verify previous echo, issue new challenge
}

// ═══════════════════════════════════════════════════════════════════════════
//  Private: Function Resolution
// ═══════════════════════════════════════════════════════════════════════════

void* FGorgeousVaultLoader::ResolveRaw(const char* FunctionName)
{
    if (!DllHandle || bSelfDestructed)
    {
        return nullptr;
    }

    FString Key(FunctionName);

    // Check the cache first
    if (void** CachedPtr = CachedFunctions.Find(Key))
    {
        return *CachedPtr;
    }

    // Resolve from the DLL
    void* FuncPtr = FPlatformProcess::GetDllExport(DllHandle, *Key);

    if (FuncPtr)
    {
        // Cache for future calls
        CachedFunctions.Add(Key, FuncPtr);
    }
    else
    {
        UE_LOG(LogGorgeousVault, Warning,
            TEXT("GorgeousVault: Failed to resolve export '%s'."), *Key);
    }

    return FuncPtr;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Cascading Death — Clear & Rearm
// ═══════════════════════════════════════════════════════════════════════════

void FGorgeousVaultLoader::ClearAllCachedFunctions()
{
    FScopeLock Lock(&InitLock);

    UE_LOG(LogGorgeousVault, Error,
        TEXT("GorgeousVault: Cascading death triggered — clearing all cached function pointers. "
             "All OV/AR operations will return nullptr until licenses are resolved."));

    CachedFunctions.Empty();
    bCascadingDeath = true;

    // NOTE: Unlike TriggerSelfDestruct, we do NOT unload the DLL.
    // The DLL stays loaded so that:
    //  1. License re-validation can be performed without an editor restart
    //  2. The Vault's internal Plugin License Registry remains intact
    //  3. GVault_License_* functions can still be resolved on-demand
}

bool FGorgeousVaultLoader::RearmFunctionCache()
{
    FScopeLock Lock(&InitLock);

    if (!DllHandle || bSelfDestructed)
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: Cannot rearm — Vault DLL is not loaded or has been self-destructed."));
        return false;
    }

    if (!bCascadingDeath)
    {
        UE_LOG(LogGorgeousVault, Warning,
            TEXT("GorgeousVault: RearmFunctionCache called but cascading death is not active. No-op."));
        return true;
    }

    // Verify that the Vault's internal registry now reports all-clear
    typedef int32_t (*EvaluateRegistryFn)(void);
    EvaluateRegistryFn EvalFn = reinterpret_cast<EvaluateRegistryFn>(
        FPlatformProcess::GetDllExport(DllHandle, TEXT("GVault_License_EvaluateRegistry")));

    if (!EvalFn)
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: Cannot rearm — GVault_License_EvaluateRegistry export not found."));
        return false;
    }

    int32_t EvalResult = EvalFn();
    if (EvalResult != GVault_OK)
    {
        UE_LOG(LogGorgeousVault, Warning,
            TEXT("GorgeousVault: Cannot rearm — license registry still reports invalid (code %d)."),
            EvalResult);
        return false;
    }

    // All licenses valid — clear cascading death flag
    // CachedFunctions is already empty from ClearAllCachedFunctions().
    // Functions will be lazily re-resolved on next Resolve<>() call.
    bCascadingDeath = false;

    UE_LOG(LogGorgeousVault, Log,
        TEXT("GorgeousVault: Function cache rearmed. All OV/AR operations are available again."));

    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Sibling Plugin License Evaluation
// ═══════════════════════════════════════════════════════════════════════════

void FGorgeousVaultLoader::EvaluateSiblingPluginLicenses(const FString& ProjectId)
{
    if (!DllHandle || bSelfDestructed)
    {
        UE_LOG(LogGorgeousVault, Warning,
            TEXT("GorgeousVault: Cannot evaluate sibling licenses — Vault is not loaded."));
        return;
    }

    // ── 1. Query UGorgeousPluginHelper for known Gorgeous plugins ───────
    UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::Get();
    if (!PluginHelper)
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: UGorgeousPluginHelper singleton not available. "
                 "Cannot discover sibling plugins for license evaluation."));
        return;
    }

    TSet<FName> KnownPlugins = PluginHelper->GetKnownGorgeousPlugins();

    // ── 2. Duplicate detection — piracy canary ──────────────────────────
    // GetKnownGorgeousPlugins() returns a TSet, so the set itself is
    // unique. However, a pirate could install the same plugin at multiple
    // paths (project-level + engine-level, or inside multiple sub-folders).
    // We cross-reference the set against the plugin manager's full
    // discovery list: if the same FriendlyName appears more than once,
    // someone has duplicated a plugin — deny everything.
    {
        TMap<FString, int32> PluginInstanceCounts;
        TArray<TSharedRef<IPlugin>> AllPlugins = IPluginManager::Get().GetDiscoveredPlugins();
        for (const TSharedRef<IPlugin>& Plugin : AllPlugins)
        {
            const FString& FriendlyName = Plugin->GetDescriptor().FriendlyName;
            if (KnownPlugins.Contains(FName(*FriendlyName)))
            {
                int32& Count = PluginInstanceCounts.FindOrAdd(FriendlyName, 0);
                ++Count;
            }
        }

        for (const auto& [Name, Count] : PluginInstanceCounts)
        {
            if (Count > 1)
            {
                UE_LOG(LogGorgeousVault, Error,
                    TEXT("GorgeousVault: DUPLICATE PLUGIN DETECTED — '%s' was discovered %d times. "
                         "This indicates a tampered or pirated plugin installation. "
                         "All OV/AR functionality is denied."),
                    *Name, Count);

                ClearAllCachedFunctions();
                return; // Hard deny — do not proceed with license evaluation
            }
        }
    }

    // ── 3. Build sibling list (exclude GorgeousCore — it is always free) ─
    TArray<FString> SiblingNames;
    for (const FName& PluginFName : KnownPlugins)
    {
        if (!PluginFName.ToString().Equals(TEXT("GorgeousCore"), ESearchCase::IgnoreCase))
        {
            SiblingNames.Add(PluginFName.ToString());
        }
    }

    if (SiblingNames.Num() == 0)
    {
        UE_LOG(LogGorgeousVault, Log,
            TEXT("GorgeousVault: No sibling plugins discovered. License evaluation skipped."));
        return;
    }

    UE_LOG(LogGorgeousVault, Log,
        TEXT("GorgeousVault: Discovered %d sibling plugin(s) for license evaluation."),
        SiblingNames.Num());

    // ── 2. Convert to const char** for C-linkage ────────────────────────
    TArray<FTCHARToUTF8> Utf8Converters;
    TArray<const char*> RawNames;
    Utf8Converters.Reserve(SiblingNames.Num());
    RawNames.Reserve(SiblingNames.Num());

    for (const FString& Name : SiblingNames)
    {
        Utf8Converters.Emplace(*Name);
        RawNames.Add(Utf8Converters.Last().Get());
    }

    // ── 3. Register sibling plugins in the Vault's Plugin License Registry
    typedef int32_t (*RegisterSiblingsFn)(const char**, int32_t);
    RegisterSiblingsFn RegisterFn = reinterpret_cast<RegisterSiblingsFn>(
        FPlatformProcess::GetDllExport(DllHandle, TEXT("GVault_License_RegisterSiblingPlugins")));

    if (!RegisterFn)
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: GVault_License_RegisterSiblingPlugins export not found."));
        return;
    }

    RegisterFn(RawNames.GetData(), static_cast<int32_t>(RawNames.Num()));

    // ── 4. Validate each plugin's license ───────────────────────────────
    typedef int32_t (*ValidatePluginFn)(const char*, const char*, const char*);
    ValidatePluginFn ValidateFn = reinterpret_cast<ValidatePluginFn>(
        FPlatformProcess::GetDllExport(DllHandle, TEXT("GVault_License_ValidatePlugin")));

    FTCHARToUTF8 ProjectIdUtf8(*ProjectId);

    if (ValidateFn)
    {
        for (int32 i = 0; i < RawNames.Num(); ++i)
        {
            // Build the expected license file path for this plugin
            FString LicPath = FPaths::Combine(
                FPaths::ProjectPluginsDir(),
                TEXT("GorgeousThings"),
                FString(SiblingNames[i]),
                TEXT("License.lic"));

            FTCHARToUTF8 LicPathUtf8(*LicPath);

            int32_t Result = ValidateFn(LicPathUtf8.Get(), ProjectIdUtf8.Get(), RawNames[i]);

            if (Result != GVault_OK)
            {
                UE_LOG(LogGorgeousVault, Warning,
                    TEXT("GorgeousVault: License validation for '%s' returned code %d."),
                    *SiblingNames[i], Result);
            }
        }
    }
    else
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: GVault_License_ValidatePlugin export not found. "
                 "Cannot validate individual sibling licenses."));
    }

    // ── 5. Evaluate the full registry ───────────────────────────────────
    typedef int32_t (*EvaluateRegistryFn)(void);
    EvaluateRegistryFn EvalFn = reinterpret_cast<EvaluateRegistryFn>(
        FPlatformProcess::GetDllExport(DllHandle, TEXT("GVault_License_EvaluateRegistry")));

    if (!EvalFn)
    {
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: GVault_License_EvaluateRegistry export not found."));
        return;
    }

    int32_t EvalResult = EvalFn();

    if (EvalResult != GVault_OK)
    {
        // ── 6. Cascading death: wipe the function cache ─────────────────
        UE_LOG(LogGorgeousVault, Error,
            TEXT("GorgeousVault: License registry evaluation FAILED (code %d). "
                 "Triggering cascading death for all sibling plugin functionality."),
            EvalResult);

        ClearAllCachedFunctions();
    }
    else
    {
        UE_LOG(LogGorgeousVault, Log,
            TEXT("GorgeousVault: All sibling plugin licenses validated successfully."));

        // If we were in cascading death and now everything is valid, rearm
        if (bCascadingDeath)
        {
            RearmFunctionCache();
        }
    }
}
