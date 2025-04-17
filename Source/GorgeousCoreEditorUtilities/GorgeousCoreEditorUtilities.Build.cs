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

public class GorgeousCoreEditorUtilities : ModuleRules
{
    public GorgeousCoreEditorUtilities(ReadOnlyTargetRules Target) : base(Target)
    {
        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        var privateIncludePath = Path.Combine(ModuleDirectory, "Private");

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        SharedPCHHeaderFile = "../GorgeousCoreRuntimeUtilities/Public/GorgeousCoreRuntimeSharedPCH.h";
        PrivatePCHHeaderFile = SharedPCHHeaderFile;
        
        PublicIncludePaths.AddRange(new string[]
        {
            publicIncludePath,
        });
        
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(privateIncludePath, "HeaderFiles"),
        });
        
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "EditorSubsystem"
        });
        
        PrivateDependencyModuleNames.AddRange(
            new[] 
            {
                "Slate", 
                "SlateCore", 
                "Projects",
                "UnrealEd", 
                "EditorStyle",
                "BlueprintGraph",
                "PropertyEditor", 
                "MessageLog",
                "UMG", 
                "Kismet",
                "HTTP",
                "Json",
                "JsonUtilities",
                "ContentBrowser",
                "DeveloperSettings"
            });
    
        
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "GorgeousCoreRuntime", 
            "GorgeousCoreRuntimeUtilities"
        });
    }
}