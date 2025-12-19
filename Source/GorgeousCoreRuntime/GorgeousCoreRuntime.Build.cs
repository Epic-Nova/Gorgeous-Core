// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/


using System.IO;
using UnrealBuildTool;

public class GorgeousCoreRuntime : ModuleRules
{
    public GorgeousCoreRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        var privateIncludePath = Path.Combine(ModuleDirectory, "Private");
        
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        SharedPCHHeaderFile = "../GorgeousCoreRuntimeUtilities/Public/GorgeousCoreRuntimeSharedPCH.h";
        PrivatePCHHeaderFile = SharedPCHHeaderFile;

        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;

        PublicIncludePaths.AddRange(new string[]
        {
            publicIncludePath,
            Path.Combine(publicIncludePath, "ModuleCore")
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(privateIncludePath, "HeaderFiles"),
        });
        
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "GameplayTags", "Projects", "EngineSettings", "ReplicationGraph" });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "GorgeousCoreRuntimeUtilities",
            "NetCore",
            "DeveloperSettings",
            "ReplicationGraph", 
            "TraceServices",
        });

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(new string[] {
                "BlueprintGraph", 
                "KismetCompiler", 
                "Kismet", 
                "UnrealEd"
            });
        }

        if (Target.bUseIris)
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
        
        throw new System.NotImplementedException("Gorgeous Core Runtime module is not finished yet and lacks important features. Please be patient until the full release of v0.9");
    }
}