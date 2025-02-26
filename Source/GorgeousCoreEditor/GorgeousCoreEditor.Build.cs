/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/


using System.IO;
using UnrealBuildTool;

public class GorgeousCoreEditor : ModuleRules
{
    public GorgeousCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        var publicIncludePath = Path.Combine(ModuleDirectory, "Public");
        var privateIncludePath = Path.Combine(ModuleDirectory, "Private");

        PublicIncludePaths.AddRange(new string[]
        {
            publicIncludePath,
            Path.Combine(publicIncludePath, "ModuleCore"),
        });
        
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(privateIncludePath, "HeaderFiles"),
            Path.Combine(privateIncludePath, "HeaderFiles", "DetailCustomizations"),
        });
        
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "EditorSubsystem" });
        
        PrivateDependencyModuleNames.AddRange(new[] { "PropertyEditor", "SlateCore", "Slate", "UMG", "UnrealEd", "BlueprintGraph" });
        
        PrivateDependencyModuleNames.AddRange(new[] { "GorgeousCoreRuntime", "GorgeousCoreRuntimeUtilities" });
    }
}