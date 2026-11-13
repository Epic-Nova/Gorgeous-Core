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

public class GorgeousCoreRuntimeUtilities : GorgeousModuleRules
{
    public GorgeousCoreRuntimeUtilities(ReadOnlyTargetRules Target) : base(Target)
    {
        ApplyGorgeousBuildSettings(new GorgeousBuildSettings {
            TargetModuleType = GorgeousModuleType.Game,
            ModulesToExclude = new[] { "GorgeousCoreRuntimeUtilities" }
        });

        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        SharedPCHHeaderFile = Path.Combine(publicIncludePath, "GorgeousCoreRuntimeSharedPCH.h");
        PrivatePCHHeaderFile = SharedPCHHeaderFile;
        
        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;
	
        PublicDependencyModuleNames.AddRange(new string[]
            {
                "Core", 
                "CoreUObject", 
                "Engine",
                "GameplayTags",
                "Projects",
                "Json",
                "JsonUtilities"
            });

        //@TODO: Also remove this one here as the build pipeline handles this way better now

        /*
        // Dynamic Gorgeous General System definitions (C++)
        var GeneralSystemsPath = Path.Combine(ModuleDirectory, "..", "GorgeousCoreRuntime", "Public", "GeneralSystems");
        if (Directory.Exists(GeneralSystemsPath))
        {
            foreach (var SystemPath in Directory.GetDirectories(GeneralSystemsPath))
            {
                var SystemName = Path.GetFileName(SystemPath).ToUpper();
                PublicDefinitions.Add($"GORGEOUS_SYSTEM_INSTALLED_{SystemName}=1");
            }
        }

        // Dynamic Gorgeous Blueprint System definitions (Content/Systems)
        var BlueprintSystemsPath = Path.Combine(ModuleDirectory, "..", "..", "..", "Content", "Systems");
        if (Directory.Exists(BlueprintSystemsPath))
        {
            foreach (var SystemPath in Directory.GetDirectories(BlueprintSystemsPath))
            {
                var SystemName = Path.GetFileName(SystemPath).ToUpper();
                var MacroName = $"GORGEOUS_SYSTEM_INSTALLED_{SystemName}=1";

                if (!PublicDefinitions.Contains(MacroName))
                {
                    PublicDefinitions.Add(MacroName);
                }
            }
        }*/

        //@TODO: Remove this, the new build pipeline module handles this way better, but still take a look if we can recycle some snippets here
        // Dynamic Gorgeous Plugin definitions
        /*var PluginsDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", ".."));
        if (Directory.Exists(PluginsDir))
        {
            var AllUPlugins = Directory.GetFiles(PluginsDir, "*.uplugin", SearchOption.AllDirectories);
            foreach (var UPlugin in AllUPlugins)
            {
                var PluginName = Path.GetFileNameWithoutExtension(UPlugin);
                bool bIsGorgeousPlugin = false;

                if (PluginName.StartsWith("Gorgeous", System.StringComparison.OrdinalIgnoreCase))
                {
                    bIsGorgeousPlugin = true;
                }
                else
                {
                    var Content = File.ReadAllText(UPlugin);
                    if (Content.Contains("\"Simsalabim Studios\"") || Content.Contains("\"Gorgeous\""))
                    {
                        bIsGorgeousPlugin = true;
                    }
                }

                if (bIsGorgeousPlugin && !PluginName.Equals("GorgeousCore", System.StringComparison.OrdinalIgnoreCase))
                {
                    var CleanName = PluginName.Replace("Gorgeous-", "", System.StringComparison.OrdinalIgnoreCase)
                                              .Replace("Gorgeous", "", System.StringComparison.OrdinalIgnoreCase)
                                              .ToUpper();
                    if (!string.IsNullOrEmpty(CleanName))
                    {
                        PublicDefinitions.Add($"GORGEOUSTHINGS_WITH_{CleanName}=1");
                    }
                }
            }
        }*/

        // Installer is an editor tool, built explicitly by a developer. UBT must
        // never install toolchains, accept licences, fetch code or block on a GUI.
        if (Target.Type == TargetType.Editor)
        {
            var exe = Target.Platform == UnrealTargetPlatform.Win64 ? "gorgeous-installer.exe" : "gorgeous-installer";
            var tool = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Binaries", Target.Platform.ToString(), exe));
            if (File.Exists(tool)) RuntimeDependencies.Add(tool, StagedFileType.NonUFS);
        }

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new[]
            {
                "UnrealEd",
                "Slate",
                "SlateCore"
            });
        }
    }
}
