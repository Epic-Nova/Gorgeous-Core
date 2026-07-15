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

using System.IO;
using UnrealBuildTool;

public class GorgeousCoreRuntime : GorgeousModuleRules
{
    public GorgeousCoreRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        ApplyGorgeousBuildSettings(new GorgeousBuildSettings {
            TargetModuleType = GorgeousModuleType.Game,
            ModulesToExclude = new[] { "GorgeousCoreRuntime" }
        });

        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        var privateIncludePath = Path.Combine(ModuleDirectory, "Private");
        
        PCHUsage = PCHUsageMode.UseSharedPCHs;
        SharedPCHHeaderFile = "../GorgeousCoreRuntimeUtilities/Public/GorgeousCoreRuntimeSharedPCH.h"; 
        PrivatePCHHeaderFile = SharedPCHHeaderFile;

        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;
        
        // The base class will auto-inject dependencies like Core, Engine, Slate, UMG, Json, etc.
        // We explicitly add things the auto-scanner might miss or that are critical third-party dependencies:
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "GorgeousVault",
            "FunctionalTesting", 
            "Niagara"
        });

        // Include the VaultLoader source files
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", "ThirdParty", "GorgeousVault", "Loader"));
        
        PublicDefinitions.Add("CSV_PROFILER=1");
        PrivateDefinitions.Add("CSV_PROFILER=1");
        PublicDefinitions.Add("GORGEOUSCORE_WITH_PLUS=0");

        // Automatically inject system macros dynamically via Reflection
        InjectForeignPluginMacros();

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new[] {
                "UnrealEd",
                "Blutility",
                "UMGEditor",
                "WorkspaceMenuStructure"
            });
        }

        // Gauntlet is only available for non-Editor program builds
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

        // Iris support
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

        PublicDefinitions.Add("GORGEOUSCORE_WITH_COG=0");

        DumpMacroDebug();
    }

    private static string GetModuleDir([System.Runtime.CompilerServices.CallerFilePath] string filePath = "")
    {
        return Path.GetDirectoryName(filePath);
    }

    [GorgeousMacroProvider]
    public static System.Collections.Generic.IEnumerable<string> ProvideGlobalMacros()
    {
        var moduleDir = GetModuleDir();
        var pluginsDir = Path.GetFullPath(Path.Combine(moduleDir, "..", "..", "..", ".."));
        
        var macros = new System.Collections.Generic.List<string>();
        macros.AddRange(InjectPluginMacros(pluginsDir));
        macros.AddRange(InjectGeneralSystemMacros("Core", moduleDir, true));
        
        return macros;
    }
}
