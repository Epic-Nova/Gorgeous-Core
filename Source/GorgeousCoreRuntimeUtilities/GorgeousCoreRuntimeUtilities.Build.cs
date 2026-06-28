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
                "Projects",
                "Json",
                "JsonUtilities"
            });

        PublicDefinitions.Add("CSV_PROFILER=1");
        PrivateDefinitions.Add("CSV_PROFILER=1");

        // Dynamic Gorgeous General System definitions (C++)
        var GeneralSystemsPath = Path.Combine(ModuleDirectory, "..", "GorgeousCoreRuntime", "Public", "GeneralSystems");
        if (Directory.Exists(GeneralSystemsPath))
        {
            foreach (var SystemPath in Directory.GetDirectories(GeneralSystemsPath))
            {
                var SystemName = Path.GetFileName(SystemPath).ToUpper();
                PublicDefinitions.Add($"GORGEOUS_SYSTEM_INSTALLED_{SystemName}=1");
            }
        }

        // Dynamic Gorgeous Blueprint System definitions (Content/Systems)
        var BlueprintSystemsPath = Path.Combine(ModuleDirectory, "..", "..", "..", "Content", "Systems");
        if (Directory.Exists(BlueprintSystemsPath))
        {
            foreach (var SystemPath in Directory.GetDirectories(BlueprintSystemsPath))
            {
                var SystemName = Path.GetFileName(SystemPath).ToUpper();
                var MacroName = $"GORGEOUS_SYSTEM_INSTALLED_{SystemName}=1";

                if (!PublicDefinitions.Contains(MacroName))
                {
                    PublicDefinitions.Add(MacroName);
                }
            }
        }

        // Dynamic Gorgeous Plugin definitions
        var PluginsDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", ".."));
        if (Directory.Exists(PluginsDir))
        {
            var AllUPlugins = Directory.GetFiles(PluginsDir, "*.uplugin", SearchOption.AllDirectories);
            foreach (var UPlugin in AllUPlugins)
            {
                var PluginName = Path.GetFileNameWithoutExtension(UPlugin);
                bool bIsGorgeousPlugin = false;

                if (PluginName.StartsWith("Gorgeous", System.StringComparison.OrdinalIgnoreCase))
                {
                    bIsGorgeousPlugin = true;
                }
                else
                {
                    var Content = File.ReadAllText(UPlugin);
                    if (Content.Contains("\"Simsalabim Studios\"") || Content.Contains("\"Gorgeous\""))
                    {
                        bIsGorgeousPlugin = true;
                    }
                }

                if (bIsGorgeousPlugin && !PluginName.Equals("GorgeousCore", System.StringComparison.OrdinalIgnoreCase))
                {
                    var CleanName = PluginName.Replace("Gorgeous-", "", System.StringComparison.OrdinalIgnoreCase)
                                              .Replace("Gorgeous", "", System.StringComparison.OrdinalIgnoreCase)
                                              .ToUpper();
                    if (!string.IsNullOrEmpty(CleanName))
                    {
                        PublicDefinitions.Add($"GORGEOUSTHINGS_WITH_{CleanName}=1");
                    }
                }
            }
        }

        // Auto-Build Gorgeous Installer (Skip if invoked by the installer itself or already built in this UBT run)
        if (System.Environment.GetEnvironmentVariable("GORGEOUS_SKIP_INSTALLER_BUILD") != "1" &&
            System.Environment.GetEnvironmentVariable("GORGEOUS_ALREADY_BUILT_INSTALLER") != "1")
        {
            try
            {
                System.Environment.SetEnvironmentVariable("GORGEOUS_ALREADY_BUILT_INSTALLER", "1");
                var InstallerDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty", "GorgeousInstaller"));
                var GoFiles = Directory.GetFiles(InstallerDir, "*.go", SearchOption.AllDirectories);
                var FilteredGoFiles = new System.Collections.Generic.List<string>();
                foreach (var F in GoFiles)
                {
                    if (!F.Replace('\\', '/').EndsWith("/internal/buildinfo/buildinfo.go"))
                    {
                        FilteredGoFiles.Add(F);
                        ExternalDependencies.Add(F);
                    }
                }

                var BinariesDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Binaries", Target.Platform.ToString()));
                var ExeName = Target.Platform == UnrealTargetPlatform.Win64 ? "gorgeous-installer.exe" : "gorgeous-installer";
                var ExeSrc = Path.Combine(InstallerDir, "build", ExeName);
                var ExeDst = Path.Combine(BinariesDir, ExeName);

                RuntimeDependencies.Add(ExeDst);

                if (!Directory.Exists(BinariesDir))
                {
                    Directory.CreateDirectory(BinariesDir);
                }

                var ProcInfo = new System.Diagnostics.ProcessStartInfo();
				ProcInfo.WorkingDirectory = InstallerDir;
				if (Target.Platform == UnrealTargetPlatform.Win64)
				{
					ProcInfo.FileName = "powershell.exe";
					ProcInfo.Arguments = $"-ExecutionPolicy Bypass -File \"{Path.Combine(InstallerDir, "build.ps1")}\" -SkipUPX";
				}
				else if (Target.Platform == UnrealTargetPlatform.Linux)
				{
					ProcInfo.FileName = "bash";
					ProcInfo.Arguments = $"\"{Path.Combine(InstallerDir, "build.sh")}\" --skip-upx";
				}
                else
                {
                    ProcInfo = null;
                }

                bool bNeedsBuild = false;
                if (ProcInfo != null)
                {
                    bNeedsBuild = !File.Exists(ExeDst);
                    if (!bNeedsBuild)
                    {
                        System.DateTime ExeTime = File.GetLastWriteTimeUtc(ExeDst);
                        foreach (var F in FilteredGoFiles)
                        {
                            if (File.GetLastWriteTimeUtc(F) > ExeTime)
                            {
                                bNeedsBuild = true;
                                break;
                            }
                        }
                    }
                }

                if (bNeedsBuild && ProcInfo != null)
                {
                    ProcInfo.UseShellExecute = false;
                    var Proc = System.Diagnostics.Process.Start(ProcInfo);
                    Proc.WaitForExit();

                    if (Proc.ExitCode != 0)
                    {
                        System.Console.WriteLine($"[Gorgeous] Installer build process exited with non-zero code: {Proc.ExitCode}");
                    }

                    if (File.Exists(ExeSrc))
                    {
                        try
                        {
                            File.Copy(ExeSrc, ExeDst, true);
                        }
                        catch (System.Exception CopyEx)
                        {
                            System.Console.WriteLine($"[Gorgeous] Failed to copy installer binary from {ExeSrc} to {ExeDst}: {CopyEx.Message}");
                        }

                        if (Target.Platform == UnrealTargetPlatform.Linux)
                        {
                            var DesktopSrc = Path.Combine(InstallerDir, "build", "gorgeous-installer.desktop");
                            var DesktopDst = Path.Combine(BinariesDir, "gorgeous-installer.desktop");
                            if (File.Exists(DesktopSrc))
                            {
                                try
                                {
                                    File.Copy(DesktopSrc, DesktopDst, true);
                                }
                                catch (System.Exception CopyEx)
                                {
                                    System.Console.WriteLine($"[Gorgeous] Failed to copy desktop entry: {CopyEx.Message}");
                                }
                            }
                        }
                    }
                    else
                    {
                        System.Console.WriteLine($"[Gorgeous] Installer source binary not found at expected path: {ExeSrc}");
                    }
                }
            }
            catch (System.Exception e)
            {
                System.Console.WriteLine("Failed to auto-build Gorgeous Installer: " + e.Message);
            }
        }

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