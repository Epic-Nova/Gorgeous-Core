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

public class GorgeousCoreEditor : GorgeousModuleRules
{
    public GorgeousCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        ApplyGorgeousBuildSettings(new GorgeousBuildSettings {
            TargetModuleType = GorgeousModuleType.Editor,
            ModulesToExclude = new[] { "GorgeousCoreEditor" }
        });

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        SharedPCHHeaderFile = "../GorgeousCoreRuntimeUtilities/Public/GorgeousCoreRuntimeSharedPCH.h";
        PrivatePCHHeaderFile = SharedPCHHeaderFile;

        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = false;

        // The base class handles core Unreal dependencies automatically based on headers.
        // We explicitly include non-header dependencies or third-party crypto here:
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "PlatformCryptoContext", 
            "OpenSSL",
            "HTTP",
            "ToolWidgets",
            "ContentBrowser",
            "InputCore",
            "GameplayTags",
            "DeveloperSettings",
            "WebBrowser"
        });
    }
}