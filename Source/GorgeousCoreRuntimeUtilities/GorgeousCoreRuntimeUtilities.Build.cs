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
                "DeveloperSettings",
                "GameplayTags",
                "Projects",
                "Json",
                "JsonUtilities"
            });

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
            // Public inline logging helpers invoke Slate notifications in editor consumers.
            PublicDependencyModuleNames.AddRange(new[]
            {
                "Slate",
                "SlateCore"
            });
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}
