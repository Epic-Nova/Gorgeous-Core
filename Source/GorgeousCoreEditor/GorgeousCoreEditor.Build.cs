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
        var privateIncludePath = Path.Combine(ModuleDirectory, "Private", "HeaderFiles");

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        SharedPCHHeaderFile = "../GorgeousCoreRuntimeUtilities/Public/GorgeousCoreRuntimeSharedPCH.h";
        PrivatePCHHeaderFile = SharedPCHHeaderFile;

        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;

        PublicIncludePaths.AddRange(new string[]
        {
            publicIncludePath,
            Path.Combine(publicIncludePath, "FunctionalStructures"),
            Path.Combine(privateIncludePath, "PropertyTypeCustomizations"),
        });
        
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(privateIncludePath),
            Path.Combine(privateIncludePath, "CodeGenerators"),
            Path.Combine(privateIncludePath, "ExtensionResourceGuards"),
            Path.Combine(privateIncludePath, "Factories"),
            Path.Combine(privateIncludePath, "K2Nodes"),
            Path.Combine(privateIncludePath, "PropertyTypeCustomizations") // Public Export Path for the Macros
        });
        
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", 
            "PropertyEditor" 
        });
        
        PrivateDependencyModuleNames.AddRange(
            new[] 
            {
                "Engine",
                "CoreUObject",
                "InputCore", 
                "GameplayTags",
                "MessageLog",
                "Slate", 
                "ToolMenus",
                "SlateCore", 
                "Projects",
                "UnrealEd", 
                "AssetTools"
            });
        
        /* Planned for 2.0
            "HTTP",
            "Json",
            "JsonUtilities",
            AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
         */
        
        
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "GorgeousCoreRuntime", 
            "GorgeousCoreRuntimeUtilities",
            "GorgeousCoreEditorUtilities"
        });
    }
}