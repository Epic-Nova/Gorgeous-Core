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

public class GorgeousCoreEditorUtilities : GorgeousModuleRules
{
    public GorgeousCoreEditorUtilities(ReadOnlyTargetRules Target) : base(Target)
    {
        ApplyGorgeousBuildSettings(new GorgeousBuildSettings {
            TargetModuleType = GorgeousModuleType.Editor,
            ModulesToExclude = new[] { "GorgeousCoreEditorUtilities" }
        });

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        SharedPCHHeaderFile = "../GorgeousCoreRuntimeUtilities/Public/GorgeousCoreRuntimeSharedPCH.h";
        PrivatePCHHeaderFile = SharedPCHHeaderFile;
        
        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;
        
        // The base class will auto-inject dependencies like Core, Engine, Slate, UMG, Json, etc.
        // We explicitly add things the auto-scanner might miss or that are critical third-party dependencies:
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "DeveloperSettings"
        });
    }
}