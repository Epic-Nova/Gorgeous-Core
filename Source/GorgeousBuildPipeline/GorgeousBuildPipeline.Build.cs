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
	/// <summary>Configures private plugin availability without exposing flags to dependent modules.</summary>
	public static void ConfigureModuleDefinitions(ModuleRules Module, ReadOnlyTargetRules Target)
	{
		GorgeousBuildDiagnostics.PrepareModule(Module.ModuleDirectory, Target.Configuration);
		var inventory = GorgeousPluginAvailability.ForTarget(Target);
		if (!GorgeousPluginAvailability.CanInject(Module.Name, inventory)) return;

		foreach (string definition in inventory.AvailabilityDefinitions)
			if (!Module.PrivateDefinitions.Contains(definition)) Module.PrivateDefinitions.Add(definition);
	}

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

/// <summary>Keeps diagnostic reports out of Shipping/Test while retaining build-critical caches.</summary>
public static class GorgeousBuildDiagnostics
{
    public static bool IsEnabled(UnrealTargetConfiguration Configuration) =>
        Configuration != UnrealTargetConfiguration.Shipping && Configuration != UnrealTargetConfiguration.Test;

    /// <summary>Removes only known stale diagnostic reports from this module's generated directory.</summary>
    public static void PrepareModule(string ModuleDirectory, UnrealTargetConfiguration Configuration)
    {
        if (IsEnabled(Configuration) || string.IsNullOrWhiteSpace(ModuleDirectory)) return;
        string directory = Path.GetFullPath(Path.Combine(ModuleDirectory, "Intermediate"));
        foreach (string filename in new[] { "GorgeousMacroDebug.json", "GorgeousIncludeDebug.json", "GorgeousInferredDependencies.json" })
        {
            string path = Path.Combine(directory, filename);
            try { if (File.Exists(path)) File.Delete(path); }
            catch (IOException) { }
            catch (UnauthorizedAccessException) { }
        }
    }
}

/// <summary>One Unreal-discovered plugin inventory per rules target for availability flags.</summary>
public static class GorgeousPluginAvailability
{
	public sealed class TargetPlugins
	{
		public Dictionary<string, PluginInfo> Plugins = new(StringComparer.OrdinalIgnoreCase);
		public HashSet<string> Enabled = new(StringComparer.OrdinalIgnoreCase);
		public HashSet<string> EarlyModules = new(StringComparer.OrdinalIgnoreCase);
		public List<string> AvailabilityDefinitions = new();
	}
	private static readonly Dictionary<string, TargetPlugins> Cache = new(StringComparer.OrdinalIgnoreCase);

	/// <summary>Availability is a compile-time contract in every configuration.</summary>
	public static bool CanInject(string ModuleName, TargetPlugins Inventory) =>
		ModuleName.StartsWith("Gorgeous", StringComparison.Ordinal) &&
		!ModuleName.StartsWith("GorgeousLoadingscreen", StringComparison.Ordinal) &&
		!Inventory.EarlyModules.Contains(ModuleName) &&
		(ModuleName.EndsWith("Runtime", StringComparison.Ordinal) || ModuleName.EndsWith("Editor", StringComparison.Ordinal));

	public static string? DefinitionFor(string Name, string DescriptorPath)
	{
		string suffix = Name.StartsWith("Gorgeous-", StringComparison.OrdinalIgnoreCase) ? Name.Substring(9) :
			Name.StartsWith("Gorgeous", StringComparison.OrdinalIgnoreCase) ? Name.Substring(8) : "";
		if (suffix.Length > 0) return "GORGEOUS_WITH_" + Regex.Replace(suffix.ToUpperInvariant(), "[^A-Z0-9_]", "_") + "=1";
		if (JsonObject.TryRead(new FileReference(DescriptorPath), out var descriptor) &&
			descriptor.TryGetBoolField("bIsGorgeousImplementation", out bool implementation) && implementation)
			return "GORGEOUS_FOREIGN_IMPLEMENTATION_" + Regex.Replace(Name.ToUpperInvariant(), "[^A-Z0-9_]", "_") + "=1";
		return null;
	}

	public static TargetPlugins ForTarget(ReadOnlyTargetRules Target)
	{
		string engineRoot = Path.GetFullPath(Target.RelativeEnginePath);
		// This target policy was added after UE 5.4; reflection keeps the rules source compatible.
		bool allowEngineDefaults = Target.GetType().GetProperty("bAllowEnginePluginsEnabledByDefault")?.GetValue(Target) as bool? ?? true;
		string key = string.Join("|", engineRoot, Target.ProjectFile?.FullName ?? "", Target.Name, Target.Platform,
			Target.Configuration, Target.Type, allowEngineDefaults, Target.bCompileAgainstEngine, Target.bCompileWithPluginSupport, Target.bIncludePluginsForTargetPlatforms,
			string.Join(",", Target.EnablePlugins.OrderBy(n => n)), string.Join(",", Target.DisablePlugins.OrderBy(n => n)), string.Join(",", Target.OptionalPlugins.OrderBy(n => n)),
			string.Join(",", Target.BuildPlugins.OrderBy(n => n)));
		lock (Cache)
		{
			if (Cache.TryGetValue(key, out var cached)) return cached;
			ProjectDescriptor? project = Target.ProjectFile == null ? null : ProjectDescriptor.FromFile(Target.ProjectFile);
			var additional = new List<DirectoryReference>();
			if (Target.ProjectFile != null && JsonObject.TryRead(Target.ProjectFile, out var json) && json.TryGetStringArrayField("AdditionalPluginDirectories", out var directories))
				additional.AddRange(directories.Select(path => new DirectoryReference(Path.GetFullPath(Path.Combine(Target.ProjectFile.Directory.FullName, path)))));
			var result = new TargetPlugins();
			// UBT stops discovery at a plugin descriptor instead of traversing plugin Source,
			// Binaries, Intermediate, Saved, repositories or bundled third-party dependency trees.
			result.Plugins = Plugins.ToFilteredDictionary(Plugins.ReadAvailablePlugins(new DirectoryReference(engineRoot), Target.ProjectFile?.Directory, additional))
				.ToDictionary(pair => pair.Key, pair => pair.Value.ChoiceVersion ?? pair.Value.KnownVersions.Last(), StringComparer.OrdinalIgnoreCase);
			result.Enabled = ResolveEnabledPlugins(result.Plugins, project, Target.Platform, Target.Configuration, Target.Type,
				allowEngineDefaults, Target.EnablePlugins, Target.DisablePlugins, Target.OptionalPlugins,
				Target.bCompileAgainstEngine || Target.bCompileWithPluginSupport, Target.bIncludePluginsForTargetPlatforms);
			// A BuildPlugins-only plugin is compiled without becoming enabled. Its
			// early-module declarations still apply, but it receives no availability flag.
			var gorgeousPlugins = result.Plugins.Values.Where(plugin => result.Enabled.Contains(plugin.Name) || Target.BuildPlugins.Contains(plugin.Name, StringComparer.OrdinalIgnoreCase))
				.Select(plugin => new { Plugin = plugin, Definition = DefinitionFor(plugin.Name, plugin.File.FullName) })
				.Where(item => item.Definition != null).ToList();
			foreach (var item in gorgeousPlugins)
				foreach (var module in item.Plugin.Descriptor.Modules ?? new List<ModuleDescriptor>())
					if (module.LoadingPhase is ModuleLoadingPhase.EarliestPossible or ModuleLoadingPhase.PostConfigInit
						or ModuleLoadingPhase.PostSplashScreen or ModuleLoadingPhase.PreEarlyLoadingScreen or ModuleLoadingPhase.PreLoadingScreen)
						result.EarlyModules.Add(module.Name);
			result.AvailabilityDefinitions = gorgeousPlugins.Where(item => result.Enabled.Contains(item.Plugin.Name)).Select(item => item.Definition!).ToList();
			Cache.Add(key, result);
			return result;
		}
	}
	/// <summary>Matches UBT's target > project > default precedence, followed by dependency closure.</summary>
	public static HashSet<string> ResolveEnabledPlugins(Dictionary<string, PluginInfo> PluginsByName, ProjectDescriptor? Project,
		UnrealTargetPlatform Platform, UnrealTargetConfiguration Configuration, TargetType Type, bool AllowEngineDefaults,
		IEnumerable<string> Enable, IEnumerable<string> Disable, IEnumerable<string> Optional, bool CompileWithPlugins = true, bool IncludePluginsForTargetPlatforms = false)
	{
		var referenced = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
		var blocked = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
		var enabled = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
		void Apply(PluginReferenceDescriptor reference)
		{
			if (!referenced.Add(reference.Name)) return;
			if (reference.bEnabled && reference.IsEnabledForPlatform(Platform) && reference.IsEnabledForTargetConfiguration(Configuration) && reference.IsEnabledForTarget(Type) &&
				PluginsByName.TryGetValue(reference.Name, out var plugin) && (IncludePluginsForTargetPlatforms || (reference.IsSupportedTargetPlatform(Platform) && plugin.Descriptor.SupportsTargetPlatform(Platform)))) enabled.Add(reference.Name);
			else blocked.Add(reference.Name);
		}
		foreach (string name in Enable) Apply(new PluginReferenceDescriptor(name, null, true));
		foreach (string name in Disable) Apply(new PluginReferenceDescriptor(name, null, false));
		foreach (string name in Optional) Apply(new PluginReferenceDescriptor(name, null, true));
		foreach (var reference in Project?.Plugins ?? Array.Empty<PluginReferenceDescriptor>()) Apply(reference);
		if (CompileWithPlugins)
			foreach (var plugin in PluginsByName.Values)
				if (!referenced.Contains(plugin.Name) && plugin.IsEnabledByDefault(AllowEngineDefaults && !(Project?.DisableEnginePluginsByDefault ?? false)))
					Apply(new PluginReferenceDescriptor(plugin.Name, null, true));
		bool changed;
		do
		{
			changed = false;
			foreach (string name in enabled.ToArray())
				if (PluginsByName.TryGetValue(name, out var plugin))
					foreach (var reference in plugin.Descriptor.Plugins ?? new List<PluginReferenceDescriptor>())
						if (reference.bEnabled && reference.IsEnabledForPlatform(Platform) && reference.IsEnabledForTargetConfiguration(Configuration) && reference.IsEnabledForTarget(Type) &&
							!blocked.Contains(reference.Name) && PluginsByName.TryGetValue(reference.Name, out var dependency) && (IncludePluginsForTargetPlatforms || (reference.IsSupportedTargetPlatform(Platform) && dependency.Descriptor.SupportsTargetPlatform(Platform))))
							changed |= enabled.Add(reference.Name);
		} while (changed);
		return enabled;
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

		var platformAsString = Target.Platform.ToString().ToUpper();
		PublicDefinitions.Add("IS_" + platformAsString + "_PLATFORM=1");

		var moduleTypeAsString = Settings.TargetModuleType.ToString().ToUpper();
		PublicDefinitions.Add("IS_GORGEOUS_" + moduleTypeAsString + "_MODULE=1");

		var moduleName = Name.ToUpper();
		PublicDefinitions.Add("GORGEOUS_WITH_" + moduleName + "_MODULE=1"); //@TODO: Do we really need this?

		ApplyAutoDependencies(Settings);
		GorgeousBuildPipeline.ConfigureModuleDefinitions(this, Target);
		// Resolve system and adapter providers before parsing conditional includes.
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

		if (!GorgeousBuildDiagnostics.IsEnabled(Target.Configuration)) return;

		// Save for debugging
		var debugFile = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousIncludeDebug.json");
		var debugDir = Path.GetDirectoryName(debugFile);
		if (debugDir != null) Directory.CreateDirectory(debugDir);

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

		File.WriteAllLines(debugFile, output);
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
	/// Writes public and private definitions to a JSON debug file.
	/// Call this at the very end of your module's constructor.
	/// </summary>
	protected void DumpMacroDebug()
	{
		if (!GorgeousBuildDiagnostics.IsEnabled(Target.Configuration)) return;

		try
		{
			var debugPath = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousMacroDebug.json");
			var dir = Path.GetDirectoryName(debugPath);
			if (dir != null) Directory.CreateDirectory(dir);

			var lines = new List<string> { "{", $"  \"Module\": \"{Name}\",", "  \"PublicDefinitions\": [" };
			var macroList = PublicDefinitions.ToList();
			for (int i = 0; i < macroList.Count; i++)
			{
				var comma = i < macroList.Count - 1 ? "," : "";
				lines.Add($"    \"{macroList[i]}\"{comma}");
			}
			lines.Add("  ],");
			lines.Add("  \"PrivateDefinitions\": [");
			lines.AddRange(PrivateDefinitions.Select((value, index) =>
				$"    \"{value}\"{(index + 1 < PrivateDefinitions.Count ? "," : "")}"));
			lines.Add("  ]");
			lines.Add("}");

			File.WriteAllLines(debugPath, lines);
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
		IReadOnlyDictionary<string, bool> PreprocessorDefinitions)
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
		if (dirName != null) Directory.CreateDirectory(dirName);
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
		File.WriteAllLines(cachePath, output);
		return generated;
	}

	/// <summary>
	/// Loads a dependency snapshot without checking signatures. If no snapshot exists,
	/// performs one initial scan and persists it for all later opt-out builds.
	/// </summary>
	private DependencyCache LoadOrCreateFrozenDependencyCache(
		GorgeousBuildSettings Settings,
		IReadOnlyDictionary<string, bool> PreprocessorDefinitions)
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
		if (cacheDirectory != null) Directory.CreateDirectory(cacheDirectory);
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
		File.WriteAllLines(CachePath, output);
	}

	private DependencyCache GenerateDependencyCache(
		GorgeousBuildSettings Settings,
		IReadOnlyDictionary<string, bool> PreprocessorDefinitions)
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
		IReadOnlyDictionary<string, bool> PreprocessorDefinitions,
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
			var definitions = new Dictionary<string, bool>(PreprocessorDefinitions, StringComparer.OrdinalIgnoreCase);
			var conditionStack = new Stack<PreprocessorConditionFrame>();
			foreach (var line in File.ReadLines(file))
			{
				if (TryProcessPreprocessorDirective(line, definitions, conditionStack))
				{
					continue;
				}

				if (!IsPreprocessorBranchActive(conditionStack))
				{
					continue;
				}

				var headerName = ExtractHeaderCandidate(line);
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
		if (!GorgeousBuildDiagnostics.IsEnabled(Target.Configuration)) return;

		var reportPath = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousInferredDependencies.json");
		Directory.CreateDirectory(Path.GetDirectoryName(reportPath)!);
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
		File.WriteAllLines(reportPath, lines);
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
		Dictionary<string, bool> Definitions,
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
					"ifdef" => Definitions.TryGetValue(argument, out var ifdefValue) && ifdefValue,
					"ifndef" => !Definitions.TryGetValue(argument, out var ifndefValue) || !ifndefValue,
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
						Definitions[definition.Groups["name"].Value] = !definition.Groups["value"].Success
							|| !string.Equals(definition.Groups["value"].Value, "0", StringComparison.Ordinal);
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

	private static bool EvaluatePreprocessorExpression(string Expression, IReadOnlyDictionary<string, bool> Definitions)
	{
		Expression = Regex.Replace(Expression, @"defined\s*\(\s*(?<name>[A-Za-z_]\w*)\s*\)", Match =>
			Definitions.TryGetValue(Match.Groups["name"].Value, out var value) && value ? "1" : "0");
		Expression = Regex.Replace(Expression, @"defined\s+(?<name>[A-Za-z_]\w*)", Match =>
			Definitions.TryGetValue(Match.Groups["name"].Value, out var value) && value ? "1" : "0");
		Expression = Regex.Replace(Expression, @"\b[A-Za-z_]\w*\b", Match =>
			Definitions.TryGetValue(Match.Value, out var value) && value ? "1" : "0");

		return new PreprocessorExpressionParser(Expression).Parse();
	}

	private sealed class PreprocessorExpressionParser
	{
		private readonly string Expression;
		private int Position;

		public PreprocessorExpressionParser(string InExpression) => Expression = InExpression;

		public bool Parse()
		{
			var value = ParseOr();
			SkipWhitespace();
			return Position == Expression.Length && value;
		}

		private bool ParseOr()
		{
			var value = ParseAnd();
			while (TryConsume("||")) value |= ParseAnd();
			return value;
		}

		private bool ParseAnd()
		{
			var value = ParseUnary();
			while (TryConsume("&&")) value &= ParseUnary();
			return value;
		}

		private bool ParseUnary()
		{
			if (TryConsume("!")) return !ParseUnary();
			if (TryConsume("("))
			{
				var value = ParseOr();
				return TryConsume(")") && value;
			}

			SkipWhitespace();
			var start = Position;
			while (Position < Expression.Length && char.IsDigit(Expression[Position])) ++Position;
			return start != Position && !string.Equals(Expression.Substring(start, Position - start), "0", StringComparison.Ordinal);
		}

		private bool TryConsume(string Token)
		{
			SkipWhitespace();
			if (!Expression.AsSpan(Position).StartsWith(Token, StringComparison.Ordinal)) return false;
			Position += Token.Length;
			return true;
		}

		private void SkipWhitespace()
		{
			while (Position < Expression.Length && char.IsWhiteSpace(Expression[Position])) ++Position;
		}
	}

	private Dictionary<string, bool> BuildPreprocessorDefinitions()
	{
		var definitions = new Dictionary<string, bool>(StringComparer.OrdinalIgnoreCase);
		AddDefinitions(definitions, Target.GlobalDefinitions);
		AddDefinitions(definitions, PublicDefinitions);
		AddDefinitions(definitions, PrivateDefinitions);
		return definitions;
	}

	private static void AddDefinitions(Dictionary<string, bool> Definitions, IEnumerable<string> RawDefinitions)
	{
		foreach (var rawDefinition in RawDefinitions)
		{
			var match = Regex.Match(rawDefinition, @"^(?<name>[A-Za-z_]\w*)(?:=(?<value>.*))?$");
			if (!match.Success) continue;
			Definitions[match.Groups["name"].Value] = !match.Groups["value"].Success
				|| !string.Equals(match.Groups["value"].Value, "0", StringComparison.Ordinal);
		}
	}

	private static bool GetDefinitionValue(IEnumerable<string> RawDefinitions, string Name, bool DefaultValue)
	{
		var definitions = new Dictionary<string, bool>(StringComparer.OrdinalIgnoreCase);
		AddDefinitions(definitions, RawDefinitions);
		return definitions.TryGetValue(Name, out var value) ? value : DefaultValue;
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

	private static string BuildPreprocessorSignature(IReadOnlyDictionary<string, bool> Definitions)
	{
		var text = string.Join("\n", Definitions.OrderBy(pair => pair.Key, StringComparer.OrdinalIgnoreCase)
			.Select(pair => $"{pair.Key}={Convert.ToInt32(pair.Value)}"));
		return Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(System.Text.Encoding.UTF8.GetBytes(text)));
	}

	private static string StripPreprocessorComment(string Value)
	{
		var lineComment = Value.IndexOf("//", StringComparison.Ordinal);
		return lineComment >= 0 ? Value.Substring(0, lineComment) : Value;
	}

	private string? ExtractHeaderCandidate(string Line)
	{
		if (!Line.Contains("#include", StringComparison.Ordinal)) return null;
		var match = Regex.Match(Line, "#include\\s*[<\"](?<path>[^>\"]+)[>\"]");
		if (!match.Success) return null;

		var includePath = match.Groups["path"].Value;
		var separatorIndex = includePath.LastIndexOf('/');
		var candidate = separatorIndex >= 0 ? includePath.Substring(separatorIndex + 1) : includePath;
		return candidate.Trim();
	}

	private Dictionary<string, string> GetOrGenerateHeaderMap(string CachePath)
	{
		var regenerate = ShouldForceDependencyRescan();
		if (!regenerate && File.Exists(CachePath))
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
				if (mapLoaded.Count > 0) return mapLoaded;
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
		if (cacheDirName != null) Directory.CreateDirectory(cacheDirName);
		
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
		File.WriteAllLines(CachePath, output);

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
