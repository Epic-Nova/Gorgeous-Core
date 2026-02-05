// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

using System.IO;
using UnrealBuildTool;

public class GorgeousCoreEditor : ModuleRules
{
    public GorgeousCoreEditor(ReadOnlyTargetRules Target) : base(Target)
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
            Path.Combine(publicIncludePath, "FunctionalStructures"),
            Path.Combine(privateIncludePath, "HeaderFiles", "FunctionalStructures")
        });
        
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(privateIncludePath, "HeaderFiles"),
            Path.Combine(privateIncludePath, "HeaderFiles", "CodeGenerators"),
            Path.Combine(privateIncludePath, "HeaderFiles", "Factories"),
            Path.Combine(privateIncludePath, "HeaderFiles", "K2Nodes"),
            Path.Combine(privateIncludePath, "HeaderFiles", "PropertyTypeCustomizations")
        });
        
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "EditorSubsystem",
            "GameplayTags"
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
                "GraphEditor",
                "PropertyEditor", 
                "MessageLog",
                "UMG", 
                "Kismet",
                "KismetCompiler",
                "HTTP",
                "Json",
                "JsonUtilities",
                "ContentBrowser",
                "DeveloperSettings"
            });
        
        AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
        
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "GorgeousCoreEditorUtilities",
            "GorgeousCoreRuntime", 
            "GorgeousCoreRuntimeUtilities"
        });
    }
}