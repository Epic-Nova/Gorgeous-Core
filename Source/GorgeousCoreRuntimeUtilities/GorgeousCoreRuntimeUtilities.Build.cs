// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

using System.IO;
using UnrealBuildTool;

public class GorgeousCoreRuntimeUtilities : ModuleRules
{
    public GorgeousCoreRuntimeUtilities(ReadOnlyTargetRules Target) : base(Target)
    {
        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        var privateIncludePath = Path.Combine(ModuleDirectory, "Private");

        PublicIncludePaths.AddRange(new string[]
        {
            publicIncludePath,
            Path.Combine(publicIncludePath, "ModuleCore"),
            Path.Combine(publicIncludePath, "Libraries"),
            Path.Combine(publicIncludePath, "Templates"),
        });
        
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(privateIncludePath, "HeaderFiles"),
        });
	
        PublicDependencyModuleNames.AddRange(new string[]
            {
                "Core", 
                "CoreUObject", 
                "Engine",
                "InputCore",
                "Kismet",
                "GameplayTags"
            });

        PrivateDependencyModuleNames.AddRange(new string[] {  });
    }
}