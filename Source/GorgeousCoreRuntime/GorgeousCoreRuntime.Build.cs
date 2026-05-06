// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

using System.IO;
using UnrealBuildTool;

public class GorgeousCoreRuntime : ModuleRules
{
    public GorgeousCoreRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        var privateIncludePath = Path.Combine(ModuleDirectory, "Private");
        
        PCHUsage = PCHUsageMode.UseSharedPCHs;
        SharedPCHHeaderFile = "../GorgeousCoreRuntimeUtilities/Public/GorgeousCoreRuntimeSharedPCH.h"; //Workaround for PCH includes as for newer Unreal versions PCH's don't work the same as they did in older ones??? (as for 5.7)
        PrivatePCHHeaderFile = SharedPCHHeaderFile;

        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;

        PublicIncludePaths.AddRange(new string[]
        {
            publicIncludePath,
            Path.Combine(publicIncludePath, "ModuleCore"),
            Path.Combine(ModuleDirectory, "..", "GorgeousCoreRuntimeUtilities", "Public") 
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(privateIncludePath, "HeaderFiles"),
            Path.Combine(ModuleDirectory, "..", "GorgeousCoreRuntimeUtilities", "Public"),
        });
        
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "GameplayTags", "Projects", "ReplicationGraph" , "UMG", "Slate", "SlateCore", "InputCore", "OnlineSubsystemUtils", "StructUtils" });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "GorgeousCoreRuntimeUtilities",
            "EngineSettings",
            "NetCore",
            "DeveloperSettings",
            "ReplicationGraph", 
            "TraceServices",
            "Json",
            "JsonUtilities",
            "GameFeatures"
        });

        // ── GorgeousVault (precompiled third-party DLL) ──────────────────
        // Provides the C-linkage API header and runtime DLL deployment.
        // The Vault is loaded at runtime via GorgeousVaultLoader.
        PrivateDependencyModuleNames.Add("GorgeousVault");

        // Include the VaultLoader source files
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", "ThirdParty", "GorgeousVault", "Loader"));
        
        
        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(new string[] {
                "BlueprintGraph", 
                "KismetCompiler", 
                "Kismet", 
                "UnrealEd"
            });

            PrivateDependencyModuleNames.AddRange(new string[] {
                "PropertyEditor"
            });
        }

        PublicDefinitions.Add("GORGEOUSTHINGS_WITH_CORE=1");
        PublicDefinitions.Add("CSV_PROFILER=1");
        PrivateDefinitions.Add("CSV_PROFILER=1");
        PublicDefinitions.Add("GORGEOUSCORE_WITH_PLUS=0"); // Placeholder for the upcoming Gorgeous Plus module, which will provide additional features and optimizations. Set to 1 when Gorgeous Plus is released.

        // Gauntlet is only available for non-Editor program builds (e.g. automation test executables)
        // It's an experimental plugin that must be enabled and doesn't ship Editor DLLs
        bool bWithGauntlet = Target.Type == TargetType.Program && !Target.bBuildEditor;
        if (bWithGauntlet)
        {
            PrivateDependencyModuleNames.Add("Gauntlet");
            PublicDefinitions.Add("GORGEOUSCORE_WITH_GAUNTLET=1");
        }
        else
        {
            PublicDefinitions.Add("GORGEOUSCORE_WITH_GAUNTLET=0");
        }

        // Workaround for the Iris support toggle because somehow UHT does not like direct access to Target.bUseIris (as for 5.7)
        var bUseIrisSupport = true;
        var bUseIrisProperty = Target.GetType().GetProperty("bUseIris");
        if (bUseIrisProperty != null && bUseIrisProperty.GetValue(Target) is bool bUseIrisValue)
        {
            bUseIrisSupport = bUseIrisValue;
        }

        if (bUseIrisSupport)
        {
            SetupIrisSupport(Target);
            PublicDefinitions.Add("GORGEOUSCORE_WITH_IRIS=1");
            PublicDefinitions.Add("GORGEOUSCORE_WITH_REPLICATION_GRAPH=0");
        }
        else
        {
            PublicDefinitions.Add("GORGEOUSCORE_WITH_IRIS=0");
            PublicDefinitions.Add("GORGEOUSCORE_WITH_REPLICATION_GRAPH=1");
        }
        
        //throw new System.NotImplementedException("Gorgeous Core Runtime module is not finished yet and lacks important features. Please be patient until the full release of v0.9");
    }
}