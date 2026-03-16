// Copyright (c) 2026-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|            GorgeousVault — Unreal Build Tool ThirdParty Module             |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026-2026 Gorgeous Things by Simsalabim Studios,    |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|  This Build.cs configures the GorgeousVault precompiled DLL as a          |
|  third-party dependency for GorgeousCoreRuntime.                          |
|                                                                           |
|  The Vault DLL is loaded at runtime via FPlatformProcess::GetDllHandle.   |
|  This module provides:                                                    |
|    - Include paths for GorgeousVaultAPI.h                                 |
|    - Runtime DLL deployment (copy to Binaries)                            |
|    - Platform-specific library paths                                      |
<==========================================================================*/

using System.IO;
using UnrealBuildTool;

public class GorgeousVault : ModuleRules
{
    public GorgeousVault(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string VaultRoot = ModuleDirectory;
        string IncludePath = Path.Combine(VaultRoot, "Include");
        string PrebuiltPath = Path.Combine(VaultRoot, "Prebuilt");

        // ── Public include path (GorgeousVaultAPI.h / GorgeousVaultPlatform.h) ──
        PublicIncludePaths.Add(IncludePath);

        // ── Platform-specific DLL deployment ─────────────────────────────
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string PlatformDir = Path.Combine(PrebuiltPath, "Win64");
            string DllPath = Path.Combine(PlatformDir, "GorgeousVault.dll");
            string LibPath = Path.Combine(PlatformDir, "GorgeousVault.lib");

            // If an import library exists, link against it.
            // (The Vault uses runtime loading via GetDllHandle, but having an
            // import lib is useful for static analysis and IDE support.)
            if (File.Exists(LibPath))
            {
                PublicAdditionalLibraries.Add(LibPath);
            }

            // Ensure the DLL is copied to the output Binaries directory
            // so FPlatformProcess::GetDllHandle can find it at runtime.
            if (File.Exists(DllPath))
            {
                RuntimeDependencies.Add("$(BinaryOutputDir)/GorgeousVault.dll", DllPath);
                PublicDelayLoadDLLs.Add("GorgeousVault.dll");
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string PlatformDir = Path.Combine(PrebuiltPath, "Linux");
            string SoPath = Path.Combine(PlatformDir, "libGorgeousVault.so");

            if (File.Exists(SoPath))
            {
                PublicAdditionalLibraries.Add(SoPath);
                RuntimeDependencies.Add("$(BinaryOutputDir)/libGorgeousVault.so", SoPath);
                PublicDelayLoadDLLs.Add("libGorgeousVault.so");
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string PlatformDir = Path.Combine(PrebuiltPath, "Mac");
            string DylibPath = Path.Combine(PlatformDir, "libGorgeousVault.dylib");

            if (File.Exists(DylibPath))
            {
                PublicAdditionalLibraries.Add(DylibPath);
                RuntimeDependencies.Add("$(BinaryOutputDir)/libGorgeousVault.dylib", DylibPath);
                PublicDelayLoadDLLs.Add("libGorgeousVault.dylib");
            }
        }

        // ── Definitions ──────────────────────────────────────────────────
        // Signal to consuming modules that the Vault third-party module is available.
        // Code can use #if GORGEOUSCORE_WITH_VAULT to conditionally compile proxy logic.
        PublicDefinitions.Add("GORGEOUSCORE_WITH_VAULT=1");
    }
}
