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
		ApplyInferredDependencies(Settings);
		ApplyAutoIncludePaths(Settings);

		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
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
		var isEditor = Settings.TargetModuleType == GorgeousModuleType.Editor || Target.bBuildEditor;
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

		if (Target.bBuildDeveloperTools)
		{
			privateModules.Add("DerivedDataCache");
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
						var result = method.Invoke(null, null) as IEnumerable<string>;
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
			if (dir != null) Directory.CreateDirectory(dir);

			var lines = new List<string> { "{", $"  \"Module\": \"{Name}\",", "  \"PublicDefinitions\": [" };
			var macroList = PublicDefinitions.ToList();
			for (int i = 0; i < macroList.Count; i++)
			{
				var comma = i < macroList.Count - 1 ? "," : "";
				lines.Add($"    \"{macroList[i]}\"{comma}");
			}
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
	/// Generic helper to find all plugins and inject macros.
	/// </summary>
	public static IEnumerable<string> InjectPluginMacros(string PluginsDirectory)
	{
		var results = new List<string>();
		if (!Directory.Exists(PluginsDirectory)) return results;

		try
		{
			var upluginFiles = Directory.GetFiles(PluginsDirectory, "*.uplugin", SearchOption.AllDirectories);
			foreach (var uplugin in upluginFiles)
			{
				var pluginName = Path.GetFileNameWithoutExtension(uplugin).ToUpper();
				
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
		var cache = LoadOrGenerateDependencyCache(Settings);
		var exclusions = new HashSet<string>(Settings.ModulesToExclude ?? Array.Empty<string>(), StringComparer.OrdinalIgnoreCase);

		AddPublicDependency(cache.PublicDependencies.Where(dep => !exclusions.Contains(dep)));
		AddPrivateDependency(cache.PrivateDependencies.Where(dep => !exclusions.Contains(dep)));
	}

	private DependencyCache LoadOrGenerateDependencyCache(GorgeousBuildSettings Settings)
	{
		var cachePath = Path.Combine(ModuleDirectory, "Intermediate", "GorgeousDepsCache.json");
		var regenerate = Target.bGenerateProjectFiles;

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
				
				if (cache.PublicDependencies.Count > 0 || cache.PrivateDependencies.Count > 0)
					return cache;
			}
			catch (Exception) { /* Fall through to regenerate */ }
		}

		var dirName = Path.GetDirectoryName(cachePath);
		if (dirName != null) Directory.CreateDirectory(dirName);
		var generated = GenerateDependencyCache(Settings);
		
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
		output.Add("  ]");
		
		output.Add("}");
		File.WriteAllLines(cachePath, output);
		return generated;
	}

	private DependencyCache GenerateDependencyCache(GorgeousBuildSettings Settings)
	{
		var exclusions = new HashSet<string>(Settings.ModulesToExclude ?? Array.Empty<string>(), StringComparer.OrdinalIgnoreCase);
		var publicPath = Path.Combine(ModuleDirectory, "Public");
		var privatePath = Path.Combine(ModuleDirectory, "Private");

		var engineDir = Path.GetFullPath(Target.RelativeEnginePath);
		var projectDir = Target.ProjectFile != null ? Target.ProjectFile.Directory.FullName : Path.Combine(engineDir, "..", "Project");
		var headerMapCachePath = Path.Combine(projectDir, "Intermediate", "GorgeousHeaderMapCache.json");
		var headerMap = GetOrGenerateHeaderMap(headerMapCachePath);

		var publicDeps = ScanForModules(publicPath, exclusions, headerMap);
		var privateDeps = ScanForModules(privatePath, exclusions, headerMap);

		return new DependencyCache
		{
			GeneratedUtc = DateTime.UtcNow,
			PublicDependencies = publicDeps.ToList(),
			PrivateDependencies = privateDeps.ToList()
		};
	}

	private IEnumerable<string> ScanForModules(string Root, HashSet<string> Exclusions, Dictionary<string, string> HeaderMap)
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
			foreach (var line in File.ReadLines(file))
			{
				var headerName = ExtractHeaderCandidate(line);
				if (headerName == null) continue;

				if (HeaderMap.TryGetValue(headerName, out var moduleName))
				{
					if (moduleName == "COLLISION" || Exclusions.Contains(moduleName) || string.Equals(moduleName, Name, StringComparison.OrdinalIgnoreCase)) continue;
					discovered.Add(moduleName);
				}
			}
		}

		foreach (var module in discovered) yield return module;
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
		var regenerate = Target.bGenerateProjectFiles;
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

	private void ScanDirectoryForHeaders(string RootDir, Dictionary<string, string> Map)
	{
		if (!Directory.Exists(RootDir)) return;

		var directories = Directory.EnumerateDirectories(RootDir, "*", SearchOption.AllDirectories);
		foreach (var dir in directories)
		{
			// Check if this directory contains a .Build.cs file
			var buildCsFiles = Directory.GetFiles(dir, "*.Build.cs");
			if (buildCsFiles.Length > 0)
			{
				var moduleName = Path.GetFileNameWithoutExtension(Path.GetFileNameWithoutExtension(buildCsFiles[0]));
				
				// Scan all headers inside this module (Public/Private/Classes)
				foreach (var subdir in new[] { "Public", "Private", "Classes" })
				{
					var subPath = Path.Combine(dir, subdir);
					if (Directory.Exists(subPath))
					{
						var headers = Directory.EnumerateFiles(subPath, "*.h", SearchOption.AllDirectories);
						foreach (var header in headers)
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
		}
	}

	private sealed class DependencyCache
	{
		public DateTime GeneratedUtc { get; set; }
		public List<string> PublicDependencies { get; set; } = new List<string>();
		public List<string> PrivateDependencies { get; set; } = new List<string>();
	}
}
