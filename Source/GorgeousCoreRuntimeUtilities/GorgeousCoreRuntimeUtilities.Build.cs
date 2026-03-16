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

public class GorgeousCoreRuntimeUtilities : ModuleRules
{
    public GorgeousCoreRuntimeUtilities(ReadOnlyTargetRules Target) : base(Target)
    {
        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        SharedPCHHeaderFile = Path.Combine(publicIncludePath, "GorgeousCoreRuntimeSharedPCH.h");
        PrivatePCHHeaderFile = SharedPCHHeaderFile;
        
        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;

        PublicIncludePaths.AddRange(new string[]
        {
            publicIncludePath,
            Path.Combine(publicIncludePath, "ModuleCore"),
            Path.Combine(publicIncludePath, "Libraries"),
            Path.Combine(publicIncludePath, "Templates"),
        });
	
        PublicDependencyModuleNames.AddRange(new string[]
            {
                "Core", 
                "CoreUObject", 
                "Engine",
                "GameplayTags",
                "Projects"
            });

        PublicDefinitions.Add("CSV_PROFILER=1");
        PrivateDefinitions.Add("CSV_PROFILER=1");

        if (Target.bBuildEditor)
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