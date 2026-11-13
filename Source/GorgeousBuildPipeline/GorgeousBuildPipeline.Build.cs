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

using UnrealBuildTool;
using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;
using System.Text.RegularExpressions;
using EpicGames.Core;

#nullable enable

// A custom attribute that plugins use to tag their macro generator methods
[AttributeUsage(AttributeTargets.Method)]
public class GorgeousMacroProviderAttribute : Attribute { }

public class GorgeousBuildPipeline : ModuleRules
{
	public GorgeousBuildPipeline(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "Core" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		
		// This module acts purely as a carrier for the Build Rules.
		// It has no code to compile.
		bUsePrecompiled = false;
	}
}

public enum GorgeousModuleType
{
	Game,
	Client,
	Server,
	Editor,
	Program
}

public struct GorgeousBuildSettings
{
	public GorgeousModuleType TargetModuleType;
	public string[]? ModulesToExclude;
	public string[]? IncludePathsToExclude;
	public List<string>? AdditionalLibraries;
	public List<string>? AdditionalDynamicLibraries;
	// Disables include-based dependency inference for this module when true.
	public bool bDisableInferredDependencies;
}

/// <summary>Compile checks generated from explicitly exported, fingerprinted guard assets.</summary>
public static class GorgeousGuardBuildRequirements
{
    private sealed class Guard
    {
        public JsonObject Data = null!;
        public string Container = "";
        public string Id => Data.GetStringField("SystemIdentifier");
    }

    private static string Under(string root, string relative)
    {
        string prefix = Path.GetFullPath(root).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + Path.DirectorySeparatorChar;
        string full = Path.GetFullPath(Path.Combine(root, relative));
        if (!full.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("Guard manifest path leaves its plugin: " + relative);
        return full;
    }

    private static string Hash(string path)
    {
        using var sha = System.Security.Cryptography.SHA1.Create();
        using var stream = File.OpenRead(path);
        return BitConverter.ToString(sha.ComputeHash(stream)).Replace("-", "").ToLowerInvariant();
    }

    private static string Symbol(string id) => Regex.Replace(id.ToUpperInvariant(), "[^A-Z0-9_]", "_");
    private static string Message(string value) => value.Replace("\\", "/").Replace("\"", "'").Replace("\r", " ").Replace("\n", " ");

    public static void Register(ModuleRules module, ReadOnlyTargetRules target, string coreRoot, string engineRoot)
    {
        ProjectDescriptor? project = target.ProjectFile == null ? null : ProjectDescriptor.FromFile(target.ProjectFile);
        var additional = new List<DirectoryReference>();
        if (project != null && target.ProjectFile != null && JsonObject.TryRead(target.ProjectFile, out var projectJson) && projectJson.TryGetStringArrayField("AdditionalPluginDirectories", out var directories))
            additional.AddRange(directories.Select(path => new DirectoryReference(Path.GetFullPath(Path.Combine(target.ProjectFile.Directory.FullName, path)))));
        var plugins = Plugins.ToFilteredDictionary(Plugins.ReadAvailablePlugins(new DirectoryReference(engineRoot), target.ProjectFile?.Directory, additional))
            .ToDictionary(pair => pair.Key, pair => pair.Value.ChoiceVersion ?? pair.Value.KnownVersions.Last(), StringComparer.OrdinalIgnoreCase);
        var enabled = new HashSet<string>(plugins.Values.Where(p => Plugins.IsPluginEnabledForTarget(p, project!, target.Platform, target.Configuration, target.Type)).Select(p => p.Name), StringComparer.OrdinalIgnoreCase);
        bool ExplicitlyOff(string name) => target.DisablePlugins.Contains(name, StringComparer.OrdinalIgnoreCase) ||
            (project?.Plugins ?? Array.Empty<PluginReferenceDescriptor>()).Any(p => p.Name.Equals(name, StringComparison.OrdinalIgnoreCase) && !p.bEnabled);
        foreach (string name in target.EnablePlugins) if (!ExplicitlyOff(name)) enabled.Add(name);
        foreach (string name in target.DisablePlugins) enabled.Remove(name);
        bool changed;
        do
        {
            changed = false;
            foreach (string name in enabled.ToArray())
                if (plugins.TryGetValue(name, out var plugin))
                    foreach (var reference in plugin.Descriptor.Plugins ?? new List<PluginReferenceDescriptor>())
                        if (reference.bEnabled && reference.IsEnabledForPlatform(target.Platform) && reference.IsEnabledForTargetConfiguration(target.Configuration) && reference.IsEnabledForTarget(target.Type) && !ExplicitlyOff(reference.Name) && plugins.ContainsKey(reference.Name))
                            changed |= enabled.Add(reference.Name);
        } while (changed);

        var guards = new List<Guard>();
        var dependencies = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var plugin in plugins.Values.OrderBy(p => p.Name, StringComparer.Ordinal))
        {
            if (!enabled.Contains(plugin.Name)) continue;
            string manifest = Path.Combine(plugin.Directory.FullName, "Config", "GorgeousGuardRequirements.json");
            if (!File.Exists(manifest))
            {
                if (plugin.Directory.FullName.Equals(coreRoot, StringComparison.OrdinalIgnoreCase) && Directory.Exists(Path.Combine(coreRoot, "Content", "Data", "CoreData", "ExtensionResourceGuards")))
                    throw new InvalidOperationException("Missing Gorgeous guard requirements export. Run UnrealEditor-Cmd <project> -run=GorgeousAuditFixup -ExportGuardRequirements before building; distribute Config/GorgeousGuardRequirements.json with the plugin.");
                continue;
            }
            dependencies.Add(manifest);
            if (!JsonObject.TryRead(new FileReference(manifest), out var document) || document.GetIntegerField("SchemaVersion") != 1 || document.GetStringField("HashAlgorithm") != "SHA1" || !document.GetStringField("PluginName").Equals(plugin.Name, StringComparison.OrdinalIgnoreCase))
                throw new InvalidOperationException("Invalid/version-mismatched Gorgeous guard manifest: " + manifest);
            var sources = document.GetObjectArrayField("Sources");
            var recorded = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (var source in sources)
            {
                string relative = source.GetStringField("Path").Replace('\\', '/');
                string path = Under(plugin.Directory.FullName, relative);
                if (!recorded.Add(relative) || !File.Exists(path) || !Hash(path).Equals(source.GetStringField("Sha1"), StringComparison.OrdinalIgnoreCase))
                    throw new InvalidOperationException("Stale Gorgeous guard metadata: " + path + ". Save assets and rerun -run=GorgeousAuditFixup -ExportGuardRequirements -GuardPlugin=" + plugin.Name);
                dependencies.Add(path);
            }
            var onDisk = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (string folder in document.GetStringArrayField("GuardFolders"))
            {
                string path = Under(plugin.Directory.FullName, folder);
                dependencies.Add(path);
                if (Directory.Exists(path)) foreach (string asset in Directory.GetFiles(path, "*.uasset", SearchOption.AllDirectories))
                    onDisk.Add(Path.GetRelativePath(plugin.Directory.FullName, asset).Replace('\\', '/'));
            }
            if (!recorded.SetEquals(onDisk)) throw new InvalidOperationException("Added/removed guard source assets require a fresh export: " + manifest);
            foreach (var data in document.GetObjectArrayField("Guards"))
            {
                if (!recorded.Contains(data.GetStringField("SourceAsset")) || string.IsNullOrWhiteSpace(data.GetStringField("SystemIdentifier")) || string.IsNullOrWhiteSpace(data.GetStringField("GuardClass")))
                    throw new InvalidOperationException("Guard manifest lacks source/class/identifier provenance: " + manifest);
                guards.Add(new Guard { Data = data, Container = plugin.Name });
            }
        }
        foreach (var file in dependencies) module.ExternalDependencies.Add(file);
        bool Active(Guard guard)
        {
            if (!guard.Data.GetBoolField("IsContentPackGuard")) return true;
            string owner = guard.Data.GetStringField("OwningPluginName");
            string content = guard.Data.GetStringField("ContentSubPath");
            if (string.IsNullOrWhiteSpace(owner) || owner == "None" || string.IsNullOrWhiteSpace(content)) return true;
            return enabled.Contains(owner) && plugins.TryGetValue(owner, out var info) && Directory.Exists(Under(Path.Combine(info.Directory.FullName, "Content"), content.TrimStart('/', '\\')));
        }
        var ids = new Dictionary<string, Guard>(StringComparer.OrdinalIgnoreCase);
        var symbols = new HashSet<string>(StringComparer.Ordinal);
        foreach (var guard in guards)
            if (!ids.TryAdd(guard.Id, guard) || !symbols.Add(Symbol(guard.Id))) throw new InvalidOperationException("Duplicate/ambiguous compile guard identifier: " + guard.Id);
        var lines = new List<string> { "// Generated by Gorgeous UBT rules from fingerprinted guard data. Do not edit.", "#pragma once", "#define GORGEOUS_GUARD_REQUIREMENTS_SCHEMA_VERSION 1" };
        var runtimeTypes = new HashSet<string>(new[] { "Runtime", "RuntimeNoCommandlet", "RuntimeAndProgram", "CookedOnly", "ClientOnly", "ClientOnlyNoCommandlet", "ServerOnly" });
        foreach (var guard in guards.OrderBy(g => g.Id, StringComparer.Ordinal))
        {
            bool active = Active(guard);
            var mandatory = new List<string>(); var optional = new List<string>();
            foreach (var requirement in guard.Data.GetObjectArrayField("RequiredPlugins"))
            {
                string name = requirement.GetStringField("Name");
                if (target.Type != TargetType.Editor && plugins.TryGetValue(name, out var info) && info.Descriptor.Modules != null && info.Descriptor.Modules.Count > 0 && !info.Descriptor.Modules.Any(m => runtimeTypes.Contains(m.Type.ToString()))) continue;
                if (!enabled.Contains(name)) (requirement.GetBoolField("Optional") ? optional : mandatory).Add("plugin " + name);
            }
            foreach (var requirement in guard.Data.GetObjectArrayField("RequiredBlueprintPacks"))
            {
                string name = requirement.GetStringField("Name");
                if (!ids.TryGetValue(name, out var dependency) || !Active(dependency)) (requirement.GetBoolField("Optional") ? optional : mandatory).Add("pack " + name);
            }
            string symbol = "GORGEOUS_GUARD_" + Symbol(guard.Id);
            lines.Add($"#define {symbol}_ACTIVE {(active ? 1 : 0)}");
            lines.Add($"#define {symbol}_REQUIREMENTS_MET {(mandatory.Count == 0 ? 1 : 0)}");
            if (active && mandatory.Count > 0) lines.Add("#error \"Gorgeous guard " + Message(guard.Id) + " requires " + Message(string.Join(", ", mandatory)) + "\"");
            if (active && optional.Count > 0) lines.Add("#pragma message(\"Optional Gorgeous guard dependency unavailable: " + Message(guard.Id + ": " + string.Join(", ", optional)) + "\")");
        }
        string header = string.Join("\n", lines) + "\n";
        // UBT owns the generated include directory. No global forced includes or public PCH flags.
        module.GenerateHeaderFuncs.Add(("GorgeousGuardRequirements", (logger, directory) =>
        {
            string path = Path.Combine(directory.FullName, "GorgeousGuardRequirements.h");
            Directory.CreateDirectory(directory.FullName);
            if (!File.Exists(path) || File.ReadAllText(path) != header) File.WriteAllText(path, header, new System.Text.UTF8Encoding(false));
        }));
    }
}

public abstract class GorgeousModuleRules : ModuleRules
{
	private static readonly string[] BasePublicDependencies = {
		"Core"
	};

	private static readonly string[] BasePrivateDependencies = {
		"CoreUObject",
		"Engine",
		"Slate",
		"SlateCore"
	};

	public GorgeousModuleRules(ReadOnlyTargetRules Target) : base(Target)
	{
	}

    private static void TryWriteAllLines(string path, IEnumerable<string> lines)
    {
        try
        {
            var dir = Path.GetDirectoryName(path);
            if (dir != null) Directory.CreateDirectory(dir);
            File.WriteAllLines(path, lines);
        }
        catch (IOException) { }
        catch (UnauthorizedAccessException) { }
    }

	protected void ApplyGorgeousBuildSettings(GorgeousBuildSettings Settings)
	{
		// Normalize settings
		Settings.ModulesToExclude ??= Array.Empty<string>();
		Settings.AdditionalLibraries ??= new List<string>();
		Settings.AdditionalDynamicLibraries ??= new List<string>();

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.Clear();
		PrivateDependencyModuleNames.Clear();

		AddPublicDependency(BasePublicDependencies);
		AddPrivateDependency(BasePrivateDependencies);
		if (Name == "GorgeousCoreRuntime")
		{
			GorgeousGuardBuildRequirements.Register(this, Target, Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..")), EngineDirectory);
		}

		var platformAsString = Target.Platform.ToString().ToUpper();
		PublicDefinitions.Add("IS_" + platformAsString + "_PLATFORM=1");

		var moduleTypeAsString = Settings.TargetModuleType.ToString().ToUpper();
		PublicDefinitions.Add("IS_GORGEOUS_" + moduleTypeAsString + "_MODULE=1");

		var moduleName = Name.ToUpper();
		PublicDefinitions.Add("GORGEOUS_WITH_" + moduleName + "_MODULE=1"); //@TODO: Do we really need this?

		ApplyAutoDependencies(Settings);
		// Make foreign macro providers available before parsing conditional includes.
		InjectForeignPluginMacros();
		if (IsInferredDependencyScanEnabled(Settings))
		{
			ApplyInferredDependencies(Settings);
		}
		else
		{
			// Opt-out freezes the last resolved dependency graph instead of removing
			// dependencies from Build.cs. A missing cache is generated exactly once.
		ApplyFrozenInferredDependencies(Settings);
		}
		ApplyAutoIncludePaths(Settings);
        PublicAdditionalLibraries.AddRange(Settings.AdditionalLibraries);
        foreach(var library in Settings.AdditionalDynamicLibraries)
        {
            PublicDelayLoadDLLs.Add(Path.GetFileName(library));
            RuntimeDependencies.Add(library.Contains("$(",StringComparison.Ordinal) || Path.IsPathRooted(library)
                ? library : Path.GetFullPath(Path.Combine(ModuleDirectory,library)));
        }

		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}

	/// <summary>
	/// Resolves whether the include-based dependency scan is enabled for this module and target.
	/// A module can set GorgeousBuildSettings.bDisableInferredDependencies to true. A target can
	/// disable it project-wide with GlobalDefinitions.Add("GORGEOUS_ENABLE_INFERRED_DEPENDENCIES=0").
	/// </summary>
	private bool IsInferredDependencyScanEnabled(GorgeousBuildSettings Settings)
	{
		if (Settings.bDisableInferredDependencies)
		{
			return false;
		}

		return GetDefinitionValue(Target.GlobalDefinitions, "GORGEOUS_ENABLE_INFERRED_DEPENDENCIES", true);
	}

	protected void AddPublicDependency(IEnumerable<string> Modules)
	{
		foreach (var module in Modules.Where(m => !string.IsNullOrWhiteSpace(m)))
		{
			if (!PublicDependencyModuleNames.Contains(module))
			{
				PublicDependencyModuleNames.Add(module);
			}
		}
	}

	private bool IsPathExcluded(string path, GorgeousBuildSettings Settings)
	{
		if (Settings.IncludePathsToExclude != null)
		{
			foreach (var exclude in Settings.IncludePathsToExclude)
			{
				if (path.IndexOf(exclude, StringComparison.OrdinalIgnoreCase) >= 0)
				{
					return true;
				}
			}
		}

		// Also support .noexpose marker file
		if (File.Exists(Path.Combine(path, ".noexpose")))
		{
			return true;
		}

		return false;
	}

	private void AddIncludePathsRecursively(string BaseDir, GorgeousBuildSettings Settings, bool bIsPublic)
	{
		if (!Directory.Exists(BaseDir)) return;

		var allDirs = Directory.GetDirectories(BaseDir, "*", SearchOption.AllDirectories);
		var validDirs = new List<string> { BaseDir }; // Always add the base Public/Private dir

		foreach (var dir in allDirs)
		{
			if (!IsPathExcluded(dir, Settings))
			{
				validDirs.Add(dir);
			}
		}

		if (bIsPublic)
		{
			PublicIncludePaths.AddRange(validDirs);
		}
		else
		{
			PrivateIncludePaths.AddRange(validDirs);
		}

		// Save for debugging
		var debugFile = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousIncludeDebug.json");
		var debugDir = Path.GetDirectoryName(debugFile);
		// Directory creation is handled by TryWriteAllLines.

		var dict = new Dictionary<string, List<string>>();
		if (File.Exists(debugFile))
		{
			try
			{
				var content = File.ReadAllText(debugFile);
				var match = Regex.Match(content, "\"PublicIncludePaths\"\\s*:\\s*\\[(.*?)\\],\\s*\"PrivateIncludePaths\"\\s*:\\s*\\[(.*?)\\]", RegexOptions.Singleline);
				if (match.Success)
				{
					dict["PublicIncludePaths"] = Regex.Matches(match.Groups[1].Value, "\"([^\"]+)\"").Cast<Match>().Select(m => m.Groups[1].Value).ToList();
					dict["PrivateIncludePaths"] = Regex.Matches(match.Groups[2].Value, "\"([^\"]+)\"").Cast<Match>().Select(m => m.Groups[1].Value).ToList();
				}
			}
			catch (Exception) { }
		}

		if (!dict.ContainsKey("PublicIncludePaths")) dict["PublicIncludePaths"] = new List<string>();
		if (!dict.ContainsKey("PrivateIncludePaths")) dict["PrivateIncludePaths"] = new List<string>();

		if (bIsPublic) dict["PublicIncludePaths"].AddRange(validDirs);
		else dict["PrivateIncludePaths"].AddRange(validDirs);

		dict["PublicIncludePaths"] = dict["PublicIncludePaths"].Distinct().ToList();
		dict["PrivateIncludePaths"] = dict["PrivateIncludePaths"].Distinct().ToList();

		var output = new List<string>();
		output.Add("{");
		output.Add("  \"PublicIncludePaths\": [");
		for (int i = 0; i < dict["PublicIncludePaths"].Count; i++)
			output.Add($"    \"{dict["PublicIncludePaths"][i].Replace("\\", "/")}\"{(i < dict["PublicIncludePaths"].Count - 1 ? "," : "")}");
		output.Add("  ],");
		output.Add("  \"PrivateIncludePaths\": [");
		for (int i = 0; i < dict["PrivateIncludePaths"].Count; i++)
			output.Add($"    \"{dict["PrivateIncludePaths"][i].Replace("\\", "/")}\"{(i < dict["PrivateIncludePaths"].Count - 1 ? "," : "")}");
		output.Add("  ]");
		output.Add("}");

		TryWriteAllLines(debugFile, output);
	}

	private void ApplyAutoIncludePaths(GorgeousBuildSettings Settings)
	{
		var publicPath = Path.Combine(ModuleDirectory, "Public");
		var privatePath = Path.Combine(ModuleDirectory, "Private");

		AddIncludePathsRecursively(publicPath, Settings, true);
		AddIncludePathsRecursively(privatePath, Settings, false);
	}

	protected void AddPrivateDependency(IEnumerable<string> Modules)
	{
		foreach (var module in Modules.Where(m => !string.IsNullOrWhiteSpace(m)))
		{
			if (!PrivateDependencyModuleNames.Contains(module))
			{
				PrivateDependencyModuleNames.Add(module);
			}
		}
	}

	private void ApplyAutoDependencies(GorgeousBuildSettings Settings)
	{
		var exclusions = new HashSet<string>(Settings.ModulesToExclude ?? Array.Empty<string>(), StringComparer.OrdinalIgnoreCase);
		var isEditor = Settings.TargetModuleType == GorgeousModuleType.Editor || Target.Type == TargetType.Editor;
		var isProgram = Settings.TargetModuleType == GorgeousModuleType.Program;
		var isServer = Settings.TargetModuleType == GorgeousModuleType.Server;
		var isGameLike = Settings.TargetModuleType == GorgeousModuleType.Game || Settings.TargetModuleType == GorgeousModuleType.Client;

		var publicModules = new List<string>();
		var privateModules = new List<string>();

		if (isGameLike)
		{
			publicModules.AddRange(new[] { "EnhancedInput" });
		}

		if (isServer)
		{
			privateModules.Add("OnlineSubsystemUtils");
		}

		if (isEditor)
		{
			privateModules.AddRange(new[] { "UnrealEd", "Blutility", "UnrealEdMessages", "PropertyEditor", "BlueprintGraph", "KismetCompiler", "Kismet" });
		}

        publicModules.AddRange(new[] { "InputCore", "GameplayTags" });

		if (!isProgram)
		{
			privateModules.Add("Projects");
		}

		AddPublicDependency(publicModules.Where(m => !exclusions.Contains(m)));
		AddPrivateDependency(privateModules.Where(m => !exclusions.Contains(m)));
	}

	// --------------------------------------------------------------------------------
	// MACRO INJECTION SYSTEM
	// --------------------------------------------------------------------------------

	protected void TryAddDefinition(string Def)
	{
		if (!PublicDefinitions.Contains(Def))
		{
			PublicDefinitions.Add(Def);
		}
	}

	/// <summary>
	/// Uses Reflection to find all methods tagged with [GorgeousMacroProvider] across all 
	/// loaded Build.cs files and injects their returned macros globally into this module.
	/// </summary>
	protected void InjectForeignPluginMacros()
	{
		try
		{
			var assembly = this.GetType().Assembly;
			foreach (var type in assembly.GetTypes())
			{
				var methods = type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
				foreach (var method in methods)
				{
					if (method.GetCustomAttribute<GorgeousMacroProviderAttribute>() != null)
					{
						// Method must return IEnumerable<string>
						var parameters = method.GetParameters();
						var result = parameters.Length switch
						{
							0 => method.Invoke(null, null) as IEnumerable<string>,
							1 when parameters[0].ParameterType == typeof(ReadOnlyTargetRules)
								=> method.Invoke(null, new object[] { Target }) as IEnumerable<string>,
							_ => null
						};
						if (result != null)
						{
							foreach (var macro in result)
							{
								TryAddDefinition(macro);
							}
						}
					}
				}
			}
		}
		catch (Exception ex)
		{
			Console.WriteLine($"[GorgeousBuildPipeline] Failed to inject foreign plugin macros: {ex.Message}");
		}
	}

	/// <summary>
	/// Writes all currently defined PublicDefinitions to a JSON debug file.
	/// Call this at the very end of your module's constructor.
	/// </summary>
	protected void DumpMacroDebug()
	{
		try
		{
			var debugPath = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousMacroDebug.json");
			var dir = Path.GetDirectoryName(debugPath);
			// Best-effort directory creation is performed by TryWriteAllLines.

			var lines = new List<string> { "{", $"  \"Module\": \"{Name}\",", "  \"PublicDefinitions\": [" };
			var macroList = PublicDefinitions.ToList();
			for (int i = 0; i < macroList.Count; i++)
			{
				var comma = i < macroList.Count - 1 ? "," : "";
				lines.Add($"    \"{macroList[i]}\"{comma}");
			}
			lines.Add("  ]");
			lines.Add("}");

			TryWriteAllLines(debugPath, lines);
		}
		catch (Exception) { /* Fail silently */ }
	}

	/// <summary>
	/// Generic helper for modules to scan a directory and format the results into macros.
	/// e.g. InjectCustomMacros(..., "*AdapterPack_*.cpp", name => $"GORGEOUS_INVENTORY_ADAPTER_{name}")
	/// </summary>
	public static IEnumerable<string> InjectCustomMacros(string SearchDir, string SearchPattern, Func<string, string> FormatDelegate)
	{
		var results = new List<string>();
		if (!Directory.Exists(SearchDir)) return results;

		try
		{
			var files = Directory.GetFiles(SearchDir, SearchPattern, SearchOption.AllDirectories);
			foreach (var file in files)
			{
				var name = Path.GetFileNameWithoutExtension(file);
				// If the pattern is *AdapterPack_*.cpp, extract the part after the last underscore
				if (name.Contains("_"))
				{
					name = name.Split('_').Last();
				}
				name = name.ToUpper();
				results.Add(FormatDelegate(name));
			}
		}
		catch (Exception) { /* Fail silently */ }

		return results;
	}

	/// <summary>
	/// Generic helper to find all plugins and inject macros.
	/// </summary>
	public static IEnumerable<string> InjectPluginMacros(string PluginsDirectory, string? ProjectDescriptorPath = null)
	{
		var results = new List<string>();
		if (!Directory.Exists(PluginsDirectory)) return results;

		try
		{
			var upluginFiles = Directory.GetFiles(PluginsDirectory, "*.uplugin", SearchOption.AllDirectories);
			foreach (var uplugin in upluginFiles)
			{
				var descriptorPluginName = Path.GetFileNameWithoutExtension(uplugin);
				if (!IsPluginDescriptorActive(descriptorPluginName, uplugin, PluginsDirectory, ProjectDescriptorPath))
				{
					continue;
				}

				var pluginName = descriptorPluginName.ToUpper();
				
				bool isNative = false;
				if (pluginName.StartsWith("GORGEOUS-")) 
				{
					pluginName = pluginName.Substring(9);
					isNative = true;
				}
				else if (pluginName.StartsWith("GORGEOUS")) 
				{
					pluginName = pluginName.Substring(8);
					isNative = true;
				}
				
				if (isNative)
				{
					results.Add($"GORGEOUS_WITH_{pluginName}=1");
				}
				else
				{
					try
					{
						var content = File.ReadAllText(uplugin);
						// Allow foreign plugins to declare they are a Gorgeous implementation
						if (content.Contains("\"bIsGorgeousImplementation\": true", StringComparison.OrdinalIgnoreCase) ||
						    content.Contains("\"bIsGorgeousImplementation\":true", StringComparison.OrdinalIgnoreCase))
						{
							results.Add($"GORGEOUS_FOREIGN_IMPLEMENTATION_{pluginName}=1");
						}
					}
					catch (Exception) { }
				}
			}
		}
		catch (Exception) { /* Fail silently */ }

		return results;
	}

	/// <summary>
	/// Applies the StructUtils module dependency only while the engine still exposes
	/// InstancedStruct through the legacy StructUtils plugin. UE 5.6 moved the
	/// required headers into the engine, so later versions must not load the
	/// deprecated plugin merely for those types.
	/// </summary>
	protected void ApplyLegacyStructUtilsRequirement(bool bRequiresStructUtilsEngine = false)
	{
		var bUsesLegacyStructUtils = Target.Version.MajorVersion < 5
			|| (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion < 6);

		PublicDefinitions.Add($"GORGEOUS_WITH_LEGACY_STRUCTUTILS={(bUsesLegacyStructUtils ? 1 : 0)}");
		if (!bUsesLegacyStructUtils)
		{
			return;
		}

		AddPublicDependency(new[] { "StructUtils" });
		if (bRequiresStructUtilsEngine)
		{
			AddPublicDependency(new[] { "StructUtilsEngine" });
		}
	}

	/// <summary>
	/// Returns whether a plugin is enabled by the project descriptor, by an enabled descriptor
	/// reference, or by its own EnabledByDefault setting. This intentionally distinguishes an
	/// installed plugin from one that UBT may load for the active target.
	/// </summary>
	private static bool IsPluginDescriptorActive(
		string PluginName,
		string PluginDescriptorPath,
		string PluginsDirectory,
		string? ProjectDescriptorPath)
	{
        // Explicit project references win, including false. Parse JSON objects rather
        // than allowing a regex to match the next plugin's Enabled field.
        bool? ReadReference(string path)
        {
            try
            {
                if (JsonObject.TryRead(new FileReference(path), out var doc) && doc.TryGetObjectArrayField("Plugins", out var refs))
                    foreach (var reference in refs)
                        if (reference.TryGetStringField("Name", out var name) &&
                            string.Equals(name, PluginName, StringComparison.OrdinalIgnoreCase) &&
                            reference.TryGetBoolField("Enabled", out var enabled))
                            return enabled;
            }
            catch (Exception) { }
            return null;
        }
        if (!string.IsNullOrWhiteSpace(ProjectDescriptorPath) && File.Exists(ProjectDescriptorPath))
        {
            var enabled = ReadReference(ProjectDescriptorPath);
            if (enabled.HasValue) return enabled.Value;
        }
        foreach (var path in Directory.GetFiles(PluginsDirectory, "*.uplugin", SearchOption.AllDirectories))
            if (ReadReference(path) == true) return true;
        try
        {
            return JsonObject.TryRead(new FileReference(PluginDescriptorPath), out var doc) && doc.TryGetBoolField("EnabledByDefault", out var enabled) && enabled;
        }
        catch (Exception) { return false; }
    }

	/// <summary>
	/// The current standard format for General System macros.
	/// </summary>
	public static IEnumerable<string> InjectGeneralSystemMacros(string SystemName, string ModuleDir, bool bWithBlueprintSystems = false)
	{
		var results = new List<string>();
		SystemName = SystemName.ToUpper();

		var publicGeneralPath = Path.Combine(ModuleDir, "Public", "GeneralSystems");
		if (Directory.Exists(publicGeneralPath))
		{
			var dirs = Directory.GetDirectories(publicGeneralPath);
			foreach (var dir in dirs)
			{
				var subSystem = Path.GetFileName(dir).ToUpper();
				results.Add($"GORGEOUS_SYSTEM_INSTALLED_{subSystem}=1");
			}
		}

		if (bWithBlueprintSystems)
		{
			var contentSystemsPath = Path.Combine(ModuleDir, "..", "..", "Content", "Systems");
			if (Directory.Exists(contentSystemsPath))
			{
				var dirs = Directory.GetDirectories(contentSystemsPath);
				foreach (var dir in dirs)
				{
					var subSystem = Path.GetFileName(dir).ToUpper();
					results.Add($"GORGEOUS_SYSTEM_INSTALLED_{subSystem}=1");
				}
			}
		}

		return results;
	}


	// --------------------------------------------------------------------------------
	// Deep Dependency Resolution
	// --------------------------------------------------------------------------------

	private void ApplyInferredDependencies(GorgeousBuildSettings Settings)
	{
		var preprocessorDefinitions = BuildPreprocessorDefinitions();
		var cache = LoadOrGenerateDependencyCache(Settings, preprocessorDefinitions);
		var exclusions = new HashSet<string>(Settings.ModulesToExclude ?? Array.Empty<string>(), StringComparer.OrdinalIgnoreCase);

		AddPublicDependency(cache.PublicDependencies.Where(dep => !exclusions.Contains(dep)));
		AddPrivateDependency(cache.PrivateDependencies.Where(dep => !exclusions.Contains(dep)));
	}

	/// <summary>
	/// Applies a stable dependency snapshot while inference is disabled. This prevents
	/// toggling a module/target opt-out from silently dropping required link inputs.
	/// The snapshot is not refreshed from source changes; delete its cache or re-enable
	/// inference deliberately when the module's include graph should change.
	/// </summary>
	private void ApplyFrozenInferredDependencies(GorgeousBuildSettings Settings)
	{
		var preprocessorDefinitions = BuildPreprocessorDefinitions();
		var cache = LoadOrCreateFrozenDependencyCache(Settings, preprocessorDefinitions);
		var exclusions = new HashSet<string>(Settings.ModulesToExclude ?? Array.Empty<string>(), StringComparer.OrdinalIgnoreCase);

		AddPublicDependency(cache.PublicDependencies.Where(dep => !exclusions.Contains(dep)));
		AddPrivateDependency(cache.PrivateDependencies.Where(dep => !exclusions.Contains(dep)));
	}

	private DependencyCache LoadOrGenerateDependencyCache(
		GorgeousBuildSettings Settings,
		IReadOnlyDictionary<string, long> PreprocessorDefinitions)
	{
		var cachePath = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousDepsCache.json");
		var sourceSignature = BuildSourceSignature();
		var preprocessorSignature = BuildPreprocessorSignature(PreprocessorDefinitions);
		var regenerate = ShouldForceDependencyRescan();

		if (!regenerate && File.Exists(cachePath))
		{
			try
			{
				var text = File.ReadAllText(cachePath);
				var cache = new DependencyCache();
				
				var pubMatch = Regex.Match(text, "\"PublicDependencies\"\\s*:\\s*\\[(.*?)\\]", RegexOptions.Singleline);
				if (pubMatch.Success)
				{
					var items = Regex.Matches(pubMatch.Groups[1].Value, "\"([^\"]+)\"");
					foreach (Match m in items) cache.PublicDependencies.Add(m.Groups[1].Value);
				}
				
				var privMatch = Regex.Match(text, "\"PrivateDependencies\"\\s*:\\s*\\[(.*?)\\]", RegexOptions.Singleline);
				if (privMatch.Success)
				{
					var items = Regex.Matches(privMatch.Groups[1].Value, "\"([^\"]+)\"");
					foreach (Match m in items) cache.PrivateDependencies.Add(m.Groups[1].Value);
				}

				var sourceSignatureMatch = Regex.Match(text, "\"SourceSignature\"\\s*:\\s*\"(?<value>[^\"]*)\"");
				cache.SourceSignature = sourceSignatureMatch.Success ? sourceSignatureMatch.Groups["value"].Value : string.Empty;
				var preprocessorSignatureMatch = Regex.Match(text, "\"PreprocessorSignature\"\\s*:\\s*\"(?<value>[^\"]*)\"");
				cache.PreprocessorSignature = preprocessorSignatureMatch.Success ? preprocessorSignatureMatch.Groups["value"].Value : string.Empty;
				
				if ((cache.PublicDependencies.Count > 0 || cache.PrivateDependencies.Count > 0)
					&& cache.SourceSignature == sourceSignature
					&& cache.PreprocessorSignature == preprocessorSignature)
					return cache;
			}
			catch (Exception) { /* Fall through to regenerate */ }
		}

		var dirName = Path.GetDirectoryName(cachePath);
		// Best-effort directory creation is performed by TryWriteAllLines.
		var generated = GenerateDependencyCache(Settings, PreprocessorDefinitions);
		generated.SourceSignature = sourceSignature;
		generated.PreprocessorSignature = preprocessorSignature;
		
		var output = new List<string>();
		output.Add("{");
		
		output.Add("  \"PublicDependencies\": [");
		for (int i = 0; i < generated.PublicDependencies.Count; i++)
		{
			var comma = (i < generated.PublicDependencies.Count - 1) ? "," : "";
			output.Add($"    \"{generated.PublicDependencies[i]}\"{comma}");
		}
		output.Add("  ],");

		output.Add("  \"PrivateDependencies\": [");
		for (int i = 0; i < generated.PrivateDependencies.Count; i++)
		{
			var comma = (i < generated.PrivateDependencies.Count - 1) ? "," : "";
			output.Add($"    \"{generated.PrivateDependencies[i]}\"{comma}");
		}
		output.Add("  ],");
		output.Add($"  \"SourceSignature\": \"{generated.SourceSignature}\",");
		output.Add($"  \"PreprocessorSignature\": \"{generated.PreprocessorSignature}\"");
		
		output.Add("}");
		TryWriteAllLines(cachePath, output);
		return generated;
	}

	/// <summary>
	/// Loads a dependency snapshot without checking signatures. If no snapshot exists,
	/// performs one initial scan and persists it for all later opt-out builds.
	/// </summary>
	private DependencyCache LoadOrCreateFrozenDependencyCache(
		GorgeousBuildSettings Settings,
		IReadOnlyDictionary<string, long> PreprocessorDefinitions)
	{
		var cachePath = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousDepsCache.json");
		if (File.Exists(cachePath))
		{
			try
			{
				var text = File.ReadAllText(cachePath);
				var cache = new DependencyCache();
				var pubMatch = Regex.Match(text, "\\\"PublicDependencies\\\"\\s*:\\s*\\[(.*?)\\]", RegexOptions.Singleline);
				if (pubMatch.Success)
				{
					foreach (Match match in Regex.Matches(pubMatch.Groups[1].Value, "\\\"([^\\\"]+)\\\"")) cache.PublicDependencies.Add(match.Groups[1].Value);
				}
				var privMatch = Regex.Match(text, "\\\"PrivateDependencies\\\"\\s*:\\s*\\[(.*?)\\]", RegexOptions.Singleline);
				if (privMatch.Success)
				{
					foreach (Match match in Regex.Matches(privMatch.Groups[1].Value, "\\\"([^\\\"]+)\\\"")) cache.PrivateDependencies.Add(match.Groups[1].Value);
				}

				// A valid cache may legitimately contain no inferred dependencies. Its
				// signature fields distinguish that state from an unreadable empty file.
				if (Regex.IsMatch(text, "\\\"SourceSignature\\\"\\s*:\\s*\\\"[^\\\"]*\\\""))
				{
					return cache;
				}
			}
			catch (Exception) { /* Build a first stable snapshot below. */ }
		}

		var generated = GenerateDependencyCache(Settings, PreprocessorDefinitions);
		generated.SourceSignature = BuildSourceSignature();
		generated.PreprocessorSignature = BuildPreprocessorSignature(PreprocessorDefinitions);
		var cacheDirectory = Path.GetDirectoryName(cachePath);
		// Best-effort directory creation is performed by TryWriteAllLines.
		WriteDependencyCache(cachePath, generated);
		Console.WriteLine($"Gorgeous inferred dependency scan is disabled for {Name}; created initial frozen cache at {cachePath}.");
		return generated;
	}

	private static void WriteDependencyCache(string CachePath, DependencyCache Cache)
	{
		var output = new List<string> { "{", "  \"PublicDependencies\": [" };
		for (int i = 0; i < Cache.PublicDependencies.Count; i++) output.Add($"    \"{Cache.PublicDependencies[i]}\"{(i < Cache.PublicDependencies.Count - 1 ? "," : "")}");
		output.Add("  ],");
		output.Add("  \"PrivateDependencies\": [");
		for (int i = 0; i < Cache.PrivateDependencies.Count; i++) output.Add($"    \"{Cache.PrivateDependencies[i]}\"{(i < Cache.PrivateDependencies.Count - 1 ? "," : "")}");
		output.Add("  ],");
		output.Add($"  \"SourceSignature\": \"{Cache.SourceSignature}\",");
		output.Add($"  \"PreprocessorSignature\": \"{Cache.PreprocessorSignature}\"");
		output.Add("}");
		TryWriteAllLines(CachePath, output);
	}

	private DependencyCache GenerateDependencyCache(
		GorgeousBuildSettings Settings,
		IReadOnlyDictionary<string, long> PreprocessorDefinitions)
	{
		var exclusions = new HashSet<string>(Settings.ModulesToExclude ?? Array.Empty<string>(), StringComparer.OrdinalIgnoreCase);
		var publicPath = Path.Combine(ModuleDirectory, "Public");
		var privatePath = Path.Combine(ModuleDirectory, "Private");

		var engineDir = Path.GetFullPath(Target.RelativeEnginePath);
		var projectDir = Target.ProjectFile != null ? Target.ProjectFile.Directory.FullName : Path.Combine(engineDir, "..", "Project");
		var headerMapCachePath = Path.Combine(projectDir, "Intermediate", "GorgeousHeaderMapCache.json");
		var headerMap = GetOrGenerateHeaderMap(headerMapCachePath);

		var diagnostics = new List<string>();
		var publicDeps = ScanForModules(publicPath, exclusions, headerMap, PreprocessorDefinitions, diagnostics).ToList();
		var privateDeps = ScanForModules(privatePath, exclusions, headerMap, PreprocessorDefinitions, diagnostics).ToList();
		WriteInferenceReport(publicDeps, privateDeps, diagnostics);

		return new DependencyCache
		{
			GeneratedUtc = DateTime.UtcNow,
			PublicDependencies = publicDeps.ToList(),
			PrivateDependencies = privateDeps.ToList()
		};
	}

	private IEnumerable<string> ScanForModules(
		string Root,
		HashSet<string> Exclusions,
		Dictionary<string, string> HeaderMap,
		IReadOnlyDictionary<string, long> PreprocessorDefinitions,
		List<string> Diagnostics)
	{
		if (!Directory.Exists(Root)) yield break;

		var files = Directory.EnumerateFiles(Root, "*.*", SearchOption.AllDirectories)
			.Where(file => file.EndsWith(".h", StringComparison.OrdinalIgnoreCase)
						 || file.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase)
						 || file.EndsWith(".inl", StringComparison.OrdinalIgnoreCase)
						 || file.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));

		var discovered = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
		foreach (var file in files)
		{
			var definitions = new Dictionary<string, long>(PreprocessorDefinitions, StringComparer.OrdinalIgnoreCase);
			var conditionStack = new Stack<PreprocessorConditionFrame>();
            bool inBlockComment=false;
			foreach (var rawLine in File.ReadLines(file))
			{
                var line=StripSourceComments(rawLine, ref inBlockComment);
				if (TryProcessPreprocessorDirective(line, definitions, conditionStack))
				{
					continue;
				}

				if (!IsPreprocessorBranchActive(conditionStack))
				{
					continue;
				}

				var headerName = ExtractHeaderCandidate(ResolveVersionInclude(line, definitions));
				if (headerName == null)
				{
					if (line.Contains("#include", StringComparison.Ordinal)) Diagnostics.Add($"Skipped macro or malformed include: {file}: {line.Trim()}");
					continue;
				}

				if (HeaderMap.TryGetValue(headerName, out var moduleName))
				{
					if (moduleName == "COLLISION")
					{
						Diagnostics.Add($"Skipped '{headerName}' because its header name collides between modules.");
						continue;
					}
					if (Exclusions.Contains(moduleName) || string.Equals(moduleName, Name, StringComparison.OrdinalIgnoreCase)) continue;
					if (!IsModuleCompatibleWithTarget(moduleName))
					{
						Diagnostics.Add($"Skipped '{moduleName}' because its name is editor-classed for target '{Target.Name}'.");
						continue;
					}
					if (WouldCreateInferredCycle(moduleName))
					{
						Diagnostics.Add($"Skipped '{moduleName}' because inferred dependency would create a cycle back to '{Name}'.");
						continue;
					}
					discovered.Add(moduleName);
				}
			}
		}

		foreach (var module in discovered) yield return module;
	}

	private bool IsModuleCompatibleWithTarget(string ModuleName)
	{
		if (Target.Type == TargetType.Editor) return true;
		return !ModuleName.Contains("Editor", StringComparison.OrdinalIgnoreCase)
			&& !string.Equals(ModuleName, "UnrealEd", StringComparison.OrdinalIgnoreCase);
	}

	private bool WouldCreateInferredCycle(string CandidateModule)
	{
		var projectDir = Target.ProjectFile != null ? Target.ProjectFile.Directory.FullName : Directory.GetCurrentDirectory();
		var graph = new Dictionary<string, List<string>>(StringComparer.OrdinalIgnoreCase);
		foreach (var cachePath in Directory.GetFiles(projectDir, "GorgeousDepsCache.json", SearchOption.AllDirectories))
		{
			try
			{
				var moduleDir = Directory.GetParent(Directory.GetParent(cachePath)!.FullName)!.Name;
				var dependencies = Regex.Matches(File.ReadAllText(cachePath), "\"(?:Public|Private)Dependencies\"\\s*:\\s*\\[(?<items>.*?)\\]", RegexOptions.Singleline)
					.Cast<Match>().SelectMany(match => Regex.Matches(match.Groups["items"].Value, "\"([^\"]+)\"").Cast<Match>().Select(item => item.Groups[1].Value)).ToList();
				graph[moduleDir] = dependencies;
			}
			catch (Exception) { }
		}

		var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
		var pending = new Stack<string>();
		pending.Push(CandidateModule);
		while (pending.Count > 0)
		{
			var current = pending.Pop();
			if (!visited.Add(current)) continue;
			if (string.Equals(current, Name, StringComparison.OrdinalIgnoreCase)) return true;
			if (graph.TryGetValue(current, out var dependencies))
			{
				foreach (var dependency in dependencies) pending.Push(dependency);
			}
		}
		return false;
	}

	private void WriteInferenceReport(IEnumerable<string> PublicDependencies, IEnumerable<string> PrivateDependencies, IEnumerable<string> Diagnostics)
	{
		var reportPath = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousInferredDependencies.json");
		// Best-effort directory creation is performed by TryWriteAllLines.
		var lines = new List<string> { "{", $"  \"Module\": \"{Name}\",", "  \"PublicDependencies\": [" };
		var publicItems = PublicDependencies.OrderBy(item => item).ToList();
		for (var index = 0; index < publicItems.Count; ++index) lines.Add($"    \"{publicItems[index]}\"{(index + 1 < publicItems.Count ? "," : string.Empty)}");
		lines.Add("  ],");
		lines.Add("  \"PrivateDependencies\": [");
		var privateItems = PrivateDependencies.OrderBy(item => item).ToList();
		for (var index = 0; index < privateItems.Count; ++index) lines.Add($"    \"{privateItems[index]}\"{(index + 1 < privateItems.Count ? "," : string.Empty)}");
		lines.Add("  ],");
		lines.Add("  \"Diagnostics\": [");
		var diagnostics = Diagnostics.Distinct().OrderBy(item => item).ToList();
		for (var index = 0; index < diagnostics.Count; ++index)
		{
			var escapedDiagnostic = diagnostics[index].Replace("\\", "\\\\").Replace("\"", "\\\"");
			lines.Add($"    \"{escapedDiagnostic}\"{(index + 1 < diagnostics.Count ? "," : string.Empty)}");
		}
		lines.Add("  ]");
		lines.Add("}");
		TryWriteAllLines(reportPath, lines);
	}

	private sealed class PreprocessorConditionFrame
	{
		public bool bParentActive;
		public bool bAnyBranchMatched;
		public bool bCurrentBranchActive;
	}

	private static bool IsPreprocessorBranchActive(IEnumerable<PreprocessorConditionFrame> ConditionStack)
	{
		return ConditionStack.All(Frame => Frame.bCurrentBranchActive);
	}

	private static bool TryProcessPreprocessorDirective(
		string Line,
		Dictionary<string, long> Definitions,
		Stack<PreprocessorConditionFrame> ConditionStack)
	{
		var directiveMatch = Regex.Match(Line, @"^\s*#\s*(?<directive>if|ifdef|ifndef|elif|else|endif|define|undef)\b(?<argument>.*)$");
		if (!directiveMatch.Success)
		{
			return false;
		}

		var directive = directiveMatch.Groups["directive"].Value;
		var argument = StripPreprocessorComment(directiveMatch.Groups["argument"].Value).Trim();
		switch (directive)
		{
			case "if":
			case "ifdef":
			case "ifndef":
			{
				var parentActive = IsPreprocessorBranchActive(ConditionStack);
				var condition = directive switch
				{
					"ifdef" => Definitions.ContainsKey(argument),
					"ifndef" => !Definitions.ContainsKey(argument),
					_ => EvaluatePreprocessorExpression(argument, Definitions)
				};
				ConditionStack.Push(new PreprocessorConditionFrame
				{
					bParentActive = parentActive,
					bAnyBranchMatched = condition,
					bCurrentBranchActive = parentActive && condition
				});
				return true;
			}
			case "elif":
				if (ConditionStack.Count > 0)
				{
					var frame = ConditionStack.Peek();
					var condition = !frame.bAnyBranchMatched && EvaluatePreprocessorExpression(argument, Definitions);
					frame.bAnyBranchMatched |= condition;
					frame.bCurrentBranchActive = frame.bParentActive && condition;
				}
				return true;
			case "else":
				if (ConditionStack.Count > 0)
				{
					var frame = ConditionStack.Peek();
					frame.bCurrentBranchActive = frame.bParentActive && !frame.bAnyBranchMatched;
					frame.bAnyBranchMatched = true;
				}
				return true;
			case "endif":
				if (ConditionStack.Count > 0) ConditionStack.Pop();
				return true;
			case "define":
				if (IsPreprocessorBranchActive(ConditionStack))
				{
					var definition = Regex.Match(argument, @"^(?<name>[A-Za-z_]\w*)(?:\s+(?<value>\S+))?");
					if (definition.Success)
					{
						Definitions[definition.Groups["name"].Value] = definition.Groups["value"].Success ? ParsePreprocessorInteger(definition.Groups["value"].Value) : 1;
					}
				}
				return true;
			case "undef":
				if (IsPreprocessorBranchActive(ConditionStack)) Definitions.Remove(argument);
				return true;
			default:
				return false;
		}
	}


    private static long ParsePreprocessorInteger(string value)
    {
        value=value.Trim().TrimEnd('u','U','l','L');
        if (value.StartsWith("0x",StringComparison.OrdinalIgnoreCase))
            return long.TryParse(value.Substring(2),System.Globalization.NumberStyles.HexNumber,System.Globalization.CultureInfo.InvariantCulture,out var hex)?hex:0;
        if (long.TryParse(value,out var number)) return number;
        return string.Equals(value,"true",StringComparison.OrdinalIgnoreCase)?1:0;
    }

    private static bool EvaluatePreprocessorExpression(string expression, IReadOnlyDictionary<string,long> definitions)
    {
        expression=Regex.Replace(expression,@"UE_VERSION_(NEWER_THAN|OLDER_THAN|AT_LEAST)\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)",m=>{
            long Get(string name)=>definitions.TryGetValue(name,out var v)?v:0;
            var installed=new[]{Get("ENGINE_MAJOR_VERSION"),Get("ENGINE_MINOR_VERSION"),Get("ENGINE_PATCH_VERSION")};
            var required=new[]{long.Parse(m.Groups[2].Value),long.Parse(m.Groups[3].Value),long.Parse(m.Groups[4].Value)};
            int compare=0; for(int i=0;i<3 && compare==0;++i) compare=installed[i].CompareTo(required[i]);
            bool yes=m.Groups[1].Value=="NEWER_THAN"?compare>0:m.Groups[1].Value=="OLDER_THAN"?compare<0:compare>=0;
            return yes?"1":"0";
        });
        expression=Regex.Replace(expression,@"defined\s*\(\s*([A-Za-z_]\w*)\s*\)",m=>definitions.ContainsKey(m.Groups[1].Value)?"1":"0");
        expression=Regex.Replace(expression,@"defined\s+([A-Za-z_]\w*)",m=>definitions.ContainsKey(m.Groups[1].Value)?"1":"0");
        expression=Regex.Replace(expression,@"\b[A-Za-z_]\w*\b",m=>definitions.TryGetValue(m.Value,out var v)?v.ToString(System.Globalization.CultureInfo.InvariantCulture):"0");
        try { return new PreprocessorExpressionParser(expression).Parse(); } catch(FormatException) { return false; } catch(OverflowException) { return false; }
    }

    // C integer-expression precedence, sufficient for build/version feature guards.
    private sealed class PreprocessorExpressionParser
    {
        private readonly string text; private int position;
        public PreprocessorExpressionParser(string expression) { text=expression; }
        public bool Parse() { long v=Or(); Space(); return position==text.Length && v!=0; }
        private long Or() { long v=And(); while(Take("||")) {long r=And();v=v!=0||r!=0?1:0;} return v; }
        private long And() { long v=BitOr(); while(Take("&&")) {long r=BitOr();v=v!=0&&r!=0?1:0;} return v; }
        private long BitOr() { long v=BitXor(); while(Single("|","||"))v|=BitXor(); return v; }
        private long BitXor() { long v=BitAnd(); while(Take("^"))v^=BitAnd(); return v; }
        private long BitAnd() { long v=Equal(); while(Single("&","&&"))v&=Equal(); return v; }
        private long Equal() { long v=Relation(); while(true) {if(Take("=="))v=v==Relation()?1:0;else if(Take("!="))v=v!=Relation()?1:0;else return v;} }
        private long Relation() { long v=Shift(); while(true) {if(Take("<="))v=v<=Shift()?1:0;else if(Take(">="))v=v>=Shift()?1:0;else if(Take("<"))v=v<Shift()?1:0;else if(Take(">"))v=v>Shift()?1:0;else return v;} }
        private long Shift() { long v=Add(); while(true) {if(Take("<<"))v<<=(int)Add();else if(Take(">>"))v>>=(int)Add();else return v;} }
        private long Add() { long v=Multiply(); while(true) {if(Take("+"))v+=Multiply();else if(Take("-"))v-=Multiply();else return v;} }
        private long Multiply() { long v=Unary(); while(true) {if(Take("*"))v*=Unary();else if(Take("/")){long r=Unary();v=r==0?0:v/r;}else if(Take("%")){long r=Unary();v=r==0?0:v%r;}else return v;} }
        private long Unary()
        {
            if(Take("!"))return Unary()==0?1:0; if(Take("~"))return ~Unary(); if(Take("-"))return -Unary(); if(Take("+"))return Unary();
            if(Take("(")){long v=Or();if(!Take(")"))throw new FormatException();return v;}
            Space();int start=position;while(position<text.Length && char.IsLetterOrDigit(text[position]))++position;
            if(start==position)throw new FormatException();return ParsePreprocessorInteger(text.Substring(start,position-start));
        }
        private void Space(){while(position<text.Length && char.IsWhiteSpace(text[position]))++position;}
        private bool Take(string token){Space();if(!text.AsSpan(position).StartsWith(token,StringComparison.Ordinal))return false;position+=token.Length;return true;}
        private bool Single(string token,string doubled){Space();return !text.AsSpan(position).StartsWith(doubled,StringComparison.Ordinal)&&Take(token);}
    }

	private Dictionary<string, long> BuildPreprocessorDefinitions()
	{
		var definitions = new Dictionary<string, long>(StringComparer.OrdinalIgnoreCase);
		AddDefinitions(definitions, Target.GlobalDefinitions);
		AddDefinitions(definitions, PublicDefinitions);
		AddDefinitions(definitions, PrivateDefinitions);
        definitions["ENGINE_MAJOR_VERSION"]=Target.Version.MajorVersion;
        definitions["ENGINE_MINOR_VERSION"]=Target.Version.MinorVersion;
        definitions["ENGINE_PATCH_VERSION"]=Target.Version.PatchVersion;
        definitions["WITH_EDITOR"]=Target.bBuildEditor?1:0;
        definitions["WITH_EDITORONLY_DATA"]=Target.bBuildWithEditorOnlyData?1:0;
		return definitions;
	}

	private static void AddDefinitions(Dictionary<string, long> Definitions, IEnumerable<string> RawDefinitions)
	{
		foreach (var rawDefinition in RawDefinitions)
		{
			var match = Regex.Match(rawDefinition, @"^(?<name>[A-Za-z_]\w*)(?:=(?<value>.*))?$");
			if (!match.Success) continue;
			Definitions[match.Groups["name"].Value] = match.Groups["value"].Success ? ParsePreprocessorInteger(match.Groups["value"].Value) : 1;
		}
	}

	private static bool GetDefinitionValue(IEnumerable<string> RawDefinitions, string Name, bool DefaultValue)
	{
		var definitions = new Dictionary<string, long>(StringComparer.OrdinalIgnoreCase);
		AddDefinitions(definitions, RawDefinitions);
		return definitions.TryGetValue(Name, out var value) ? value != 0 : DefaultValue;
	}

	private string BuildSourceSignature()
	{
		var files = new[] { Path.Combine(ModuleDirectory, "Public"), Path.Combine(ModuleDirectory, "Private") }
			.Where(Directory.Exists)
			.SelectMany(root => Directory.EnumerateFiles(root, "*.*", SearchOption.AllDirectories))
			.Where(file => file.EndsWith(".h", StringComparison.OrdinalIgnoreCase)
				|| file.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase)
				|| file.EndsWith(".inl", StringComparison.OrdinalIgnoreCase)
				|| file.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase))
			.OrderBy(file => file, StringComparer.OrdinalIgnoreCase)
			.Select(file => $"{file}|{File.GetLastWriteTimeUtc(file).Ticks}");
		return Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(System.Text.Encoding.UTF8.GetBytes(string.Join("\n", files))));
	}

	private static string BuildPreprocessorSignature(IReadOnlyDictionary<string, long> Definitions)
	{
		var text = string.Join("\n", Definitions.OrderBy(pair => pair.Key, StringComparer.OrdinalIgnoreCase)
			.Select(pair => $"{pair.Key}={pair.Value}"));
		return Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(System.Text.Encoding.UTF8.GetBytes(text)));
	}

	private static string StripPreprocessorComment(string Value)
	{
		var lineComment = Value.IndexOf("//", StringComparison.Ordinal);
		return lineComment >= 0 ? Value.Substring(0, lineComment) : Value;
	}


    private static string StripSourceComments(string line, ref bool block)
    {
        var result=new System.Text.StringBuilder(); bool quoted=false, escaped=false;
        for(int i=0;i<line.Length;++i)
        {
            char c=line[i], next=i+1<line.Length?line[i+1]:'\0';
            if(block){if(c=='*'&&next=='/'){block=false;++i;result.Append(' ');}continue;}
            if(!quoted&&c=='/'&&next=='*'){block=true;++i;continue;}
            if(!quoted&&c=='/'&&next=='/')break;
            result.Append(c);
            if(c=='"'&&!escaped)quoted=!quoted;
            escaped=c=='\\'&&!escaped;
        }
        return result.ToString();
    }
    private static string ResolveVersionInclude(string line, IReadOnlyDictionary<string,long> definitions)
    {
        return Regex.Replace(line,@"GORGEOUS_(\d)(\d+)_SWITCH\s*\(\s*""([^""]+)""\s*,\s*""([^""]+)""\s*\)",m=>{
            definitions.TryGetValue("ENGINE_MAJOR_VERSION",out var major);
            definitions.TryGetValue("ENGINE_MINOR_VERSION",out var minor);
            long requiredMajor=long.Parse(m.Groups[1].Value), requiredMinor=long.Parse(m.Groups[2].Value);
            return "\""+m.Groups[major>requiredMajor||(major==requiredMajor&&minor>=requiredMinor)?4:3].Value+"\"";
        });
    }
	private string? ExtractHeaderCandidate(string Line)
	{
		if (!Regex.IsMatch(Line, @"^\s*#\s*include\b")) return null;
		var match = Regex.Match(Line, "#include\\s*[<\"](?<path>[^>\"]+)[>\"]");
		if (!match.Success) return null;

		var includePath = match.Groups["path"].Value;
		var separatorIndex = includePath.LastIndexOf('/');
		var candidate = separatorIndex >= 0 ? includePath.Substring(separatorIndex + 1) : includePath;
		return candidate.Trim();
	}

    private static readonly Dictionary<string,Dictionary<string,string>> HeaderMapsThisRun = new(StringComparer.OrdinalIgnoreCase);
    private static string BuildHeaderSearchSignature(IEnumerable<string> roots)
    {
        var entries=new List<string>();
        try
        {
            foreach(var root in roots.Where(Directory.Exists))
            {
                entries.Add(root+"|"+Directory.GetLastWriteTimeUtc(root).Ticks);
                foreach(var rules in Directory.EnumerateFiles(root,"*.Build.cs",SearchOption.AllDirectories))
                {
                    entries.Add(rules);
                    var module=Path.GetDirectoryName(rules)!;
                    foreach(var folder in new[]{"Public","Private","Classes"})
                    {
                        var path=Path.Combine(module,folder); if(!Directory.Exists(path))continue;
                        entries.Add(path+"|"+Directory.GetLastWriteTimeUtc(path).Ticks);
                        foreach(var dir in Directory.EnumerateDirectories(path,"*",SearchOption.AllDirectories))
                            entries.Add(dir+"|"+Directory.GetLastWriteTimeUtc(dir).Ticks);
                    }
                }
            }
        }
        catch(IOException) { return Guid.NewGuid().ToString(); }
        catch(UnauthorizedAccessException) { return Guid.NewGuid().ToString(); }
        entries.Sort(StringComparer.OrdinalIgnoreCase);
        return Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(System.Text.Encoding.UTF8.GetBytes(string.Join("\n",entries))));
    }
    private static bool HeaderSignatureMatches(string path,string signature)
    {
        try { return File.Exists(path)&&File.ReadAllText(path).Trim()==signature; }
        catch(IOException) { return false; }
        catch(UnauthorizedAccessException) { return false; }
    }

	private Dictionary<string, string> GetOrGenerateHeaderMap(string CachePath)
	{
        var regenerate=ShouldForceDependencyRescan();
        if(!regenerate && HeaderMapsThisRun.TryGetValue(CachePath,out var existing)) return existing;
        var engineRoot=Path.GetFullPath(Target.RelativeEnginePath);
        var projectRoot=Target.ProjectFile?.Directory.FullName ?? Path.Combine(engineRoot,"..","Project");
        var roots=new[]{Path.Combine(engineRoot,"Source","Runtime"),Path.Combine(engineRoot,"Source","Editor"),Path.Combine(engineRoot,"Source","Developer"),Path.Combine(engineRoot,"Plugins"),Path.Combine(projectRoot,"Plugins"),Path.Combine(projectRoot,"Source")};
        var signature=BuildHeaderSearchSignature(roots);
        var signatureFile=CachePath+".signature";
		if (!regenerate && File.Exists(CachePath) && HeaderSignatureMatches(signatureFile,signature))
		{
			try
			{
				var mapLoaded = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
				var text = File.ReadAllText(CachePath);
				var matches = Regex.Matches(text, "\"([^\"]+)\"\\s*:\\s*\"([^\"]+)\"");
				foreach (Match match in matches)
				{
					mapLoaded[match.Groups[1].Value] = match.Groups[2].Value;
				}
				if (mapLoaded.Count > 0) { HeaderMapsThisRun[CachePath]=mapLoaded; return mapLoaded; }
			}
			catch (Exception) { }
		}

		var map = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
		var engineDir = Path.GetFullPath(Target.RelativeEnginePath);
		var projectDir = Target.ProjectFile != null ? Target.ProjectFile.Directory.FullName : Path.Combine(engineDir, "..", "Project");

		// We will scan Engine/Source/Runtime, Engine/Plugins, and the Project plugins
		ScanDirectoryForHeaders(Path.Combine(engineDir, "Source", "Runtime"), map);
		ScanDirectoryForHeaders(Path.Combine(engineDir, "Source", "Editor"), map);
		ScanDirectoryForHeaders(Path.Combine(engineDir, "Source", "Developer"), map);
		ScanDirectoryForHeaders(Path.Combine(engineDir, "Plugins"), map);
		ScanDirectoryForHeaders(Path.Combine(projectDir, "Plugins"), map);
		ScanDirectoryForHeaders(Path.Combine(projectDir, "Source"), map);

		var cacheDirName = Path.GetDirectoryName(CachePath);
		// Best-effort directory creation is performed by TryWriteAllLines.
		
		var output = new List<string>();
		output.Add("{");
		int i = 0;
		foreach (var kvp in map)
		{
			var comma = (i < map.Count - 1) ? "," : "";
			output.Add($"  \"{kvp.Key}\": \"{kvp.Value}\"{comma}");
			i++;
		}
		output.Add("}");
		TryWriteAllLines(CachePath, output);
        TryWriteAllLines(signatureFile,new[]{signature});
        HeaderMapsThisRun[CachePath]=map;

		return map;
	}

	/// <summary>
	/// Project-file generation uses the normal signature-validated dependency and
	/// header-map caches. Define GORGEOUS_FORCE_DEPENDENCY_RESCAN=1 on a target,
	/// or set the environment variable to 1, to deliberately rebuild both caches.
	/// </summary>
	private bool ShouldForceDependencyRescan()
	{
		return GetDefinitionValue(Target.GlobalDefinitions, "GORGEOUS_FORCE_DEPENDENCY_RESCAN", false)
			|| string.Equals(Environment.GetEnvironmentVariable("GORGEOUS_FORCE_DEPENDENCY_RESCAN"), "1", StringComparison.Ordinal);
	}
	private void ScanDirectoryForHeaders(string RootDir, Dictionary<string, string> Map)
	{
		if (!Directory.Exists(RootDir)) return;

		try
		{
			// Discover module roots in one pass. The previous implementation enumerated
			// every directory and then probed each one for a Build.cs file, which makes
			// an explicit refresh especially expensive for engine and plugin trees.
			foreach (var buildCsFile in Directory.EnumerateFiles(RootDir, "*.Build.cs", SearchOption.AllDirectories))
			{
				var moduleDirectory = Path.GetDirectoryName(buildCsFile);
				if (string.IsNullOrEmpty(moduleDirectory)) continue;

				var moduleName = Path.GetFileNameWithoutExtension(Path.GetFileNameWithoutExtension(buildCsFile));
				foreach (var subdir in new[] { "Public", "Private", "Classes" })
				{
					var subPath = Path.Combine(moduleDirectory, subdir);
					if (!Directory.Exists(subPath)) continue;

					foreach (var header in Directory.EnumerateFiles(subPath, "*.h", SearchOption.AllDirectories))
					{
						var headerName = Path.GetFileName(header);
						if (Map.TryGetValue(headerName, out var existingModule))
						{
							if (existingModule != moduleName && existingModule != "COLLISION")
							{
								Map[headerName] = "COLLISION";
							}
						}
						else
						{
							Map[headerName] = moduleName;
						}
					}
				}
			}
		}
		catch (Exception ex)
		{
			Console.WriteLine($"[GorgeousBuildPipeline] Header-map scan failed for {RootDir}: {ex.Message}");
		}
	}

	private sealed class DependencyCache
	{
		public DateTime GeneratedUtc { get; set; }
		public List<string> PublicDependencies { get; set; } = new List<string>();
		public List<string> PrivateDependencies { get; set; } = new List<string>();
		public string SourceSignature { get; set; } = string.Empty;
		public string PreprocessorSignature { get; set; } = string.Empty;
	}
}
