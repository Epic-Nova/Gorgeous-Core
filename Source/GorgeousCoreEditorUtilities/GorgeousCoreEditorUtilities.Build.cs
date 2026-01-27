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

public class GorgeousCoreEditorUtilities : ModuleRules
{
    public GorgeousCoreEditorUtilities(ReadOnlyTargetRules Target) : base(Target)
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
        });

        PrivateIncludePaths.AddRange(new[]
        {
            privateIncludePath
        });
        
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "EditorSubsystem",
            "DeveloperToolSettings"
        });
        
        PrivateDependencyModuleNames.AddRange(
            new[] 
            {
                "Slate", 
                "SlateCore", 
                "Projects",
                "UnrealEd", 
                "EditorStyle",
                "ToolMenus",
                "BlueprintGraph",
                "PropertyEditor", 
                "MessageLog",
                "DataValidation",
                "AssetRegistry",
                "AssetTools",
                "UMG", 
                "Kismet",
                "HTTP",
                "Json",
                "JsonUtilities",
                "ContentBrowser",
                "DeveloperSettings",
                "GameplayTags"
            });
    
        
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "GorgeousCoreRuntime", 
            "GorgeousCoreRuntimeUtilities"
        });
    }
}