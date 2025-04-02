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

//@TODO: This is a WIP file. To learn more about what's to come and why this file exists. Check out the current roadmap of Gorgeous Things under our github repository: https://github.com/orgs/Epic-Nova/projects/12

/*=======================>
| Dependencies & Pragmas |
<=======================*/
using UnrealBuildTool;
using EpicGames.Core;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Collections.Generic;
using System.Diagnostics;

// Disable warnings about unused variables from ReSharper
// ReSharper disable InconsistentNaming

// Disable warning CS0618: 'Type' is obsolete: 'Type class is obsolete
#nullable enable

// Disable warning CS0618: '
#pragma warning disable CS0618

/*====================>
| Build Platform Enum |
<====================*/
public enum GorgeousBuildPlatform
{
	All,
	Windows,
	Linux,
	LinuxArm64,
	Mac,
	Android,
	IOS,
	TVOS,
	VisionOS,
	PlayStation,
	Xbox,
	Switch,
}

/*=================>
| Module Type Enum |
<=================*/
public enum GorgeousModuleType
{
	Game,
	Client,
	Server,
	Editor,
	Program
}

/*======================>
| Build Settings Struct |
<======================*/
public struct GorgeousBuildSettings
{
	public GorgeousModuleType TargetModuleType;
	public UnrealTargetPlatform TargetPlatform;
	public string[] ModulesToExclude;

	public List<string> AdditionalLibraries;
	public List<string> AdditionalDynamicLibraries;
}

/**
 * This standalone build file for Unreal Engine 4/5 is designed to provide a clean and well structured build file for all build products.
 *
 * It is designed to be used with any Unreal Engine plugin and project but was originally designed for the Gorgeous Things Plugin family.
 *
 * -------------------------------------------- Features -------------------------------------------
 * Provides pre configured UBT definitions, functions and settings to:
 * - Properly build Gorgeous Things with all available and eligible extensions.
 * - Provide a clean and well-structured build file for all build products.
 * - Easy configurable cross dependant modules.
 * - Provide a worry free opt in setup for external modules.
 * - Provide preprocessor macros to every gorgeous plugin to grant the ability for cross functionality sharing and extending.
 *
 * @author Nils Bergemann
 * @since Beginning
 * @version 0.0.1
 *
 * @todo boolean flag that can be set to specify that a gorgeous plugin includes an utilities module either for editor or runtime or both
 * @todo Every Module coming from a Gorgeous Things extension should depend on this build file.
 */
public class GorgeousCore : ModuleRules
{
	private static GorgeousPlatform GorgeousPlatformInstance;
	private static GorgeousConfiguration GorgeousConfigurationInstance;
	private static GorgeousBuildSettings GorgeousBuildSettingsInstance;
	
	private GorgeousPlatform GetGorgeousPlatformInstance()
	{
		var PlatformType = DoesCodingTypeExist("GorgeousPlatform_" + GetPlatform().ToString(), true);

		if (Activator.CreateInstance(PlatformType!) is GorgeousPlatform PlatformInstance) return PlatformInstance;
		
		Log.TraceError(GetLoggingCategory() + "Gorgeous Platform Instance not found for " + GetPlatform() + " platform.");
		throw new BuildException("Gorgeous Platform Instance not found for " + GetPlatform() + " platform.");
	}

	private GorgeousConfiguration GetGorgeousConfigurationInstance()
	{
		var ConfigurationType = DoesCodingTypeExist("GorgeousConfiguration_" + GetConfiguration().ToString(), true);

		if (Activator.CreateInstance(ConfigurationType!) is GorgeousConfiguration ConfigurationInstance) return ConfigurationInstance;
		
		Log.TraceError(GetLoggingCategory() + "Gorgeous Configuration Instance not found for " + GetConfiguration() + " configuration.");
		throw new BuildException("Gorgeous Configuration Instance not found for " + GetConfiguration() + " configuration.");
	}

	private bool ConfigurePlatform(ReadOnlyTargetRules Target)
	{
		return false;
	}
	
	private bool ConfigureConfiguration(UnrealTargetConfiguration Configuration)
	{
		return false;
	}

	public GorgeousCore(ReadOnlyTargetRules Target) : base(Target)
	{
	}
	
	/**
	 * Returns the third party path for the current module.
	 *
	 * @return The third party path.
	 */
	private string GetThirdPartyPath()
	{
		return Path.Combine(ModuleDirectory, "ThirdParty");
	}
	
	/**
	 * Returns the current build configuration for this module.
	 *
	 * @return The current build configuration. can be Shipping, Development, DebugGame, Debug, Test or Unknown.
	 */
	private UnrealTargetConfiguration GetConfiguration()
	{
		return Target.Configuration;
	}
	
	/**
	 * Returns the current build platform for this module.
	 *
	 * @return The current build platform. can be Windows, Linux, LinuxArm64, Mac, PlayStation, Xbox, Switch, IOS, Android, VisionOS or TVOS.
	 */
	private UnrealTargetPlatform GetPlatform()
	{
		return Target.Platform;
	}

	/**
	 * Checks if a specific coding type in form of a class, structure or enum exists.
	 *
	 * @return True if the coding type exists, false otherwise.
	 */
	private Type? DoesCodingTypeExist(string CodingType, bool bThrowExceptionIfNotFound = false)
	{
		var CodingTypeType = System.Type.GetType(CodingType);
		
		if (CodingTypeType != null) return CodingTypeType;
		if (!bThrowExceptionIfNotFound) return null;
		
		Log.TraceError(GetLoggingCategory() + "Gorgeous Type not found for " + CodingType + " coding type.");
		throw new BuildException("Gorgeous Type not found for " + CodingType + " coding type.");
	}
	
	/**
	 *  Returns the logging category with the option to log as the Core module if needed.
	 */
	private string GetLoggingCategory(bool bStaticLoggingCategory = false)
	{
		return bStaticLoggingCategory ? "[GorgeousCore]: " : "[" + this.Name + "]: ";
	}
}

public abstract class GorgeousPlatform
{
	//Everything above here is for setting up the build file on the specific platform

	public abstract GorgeousBuildPlatform Platform { get; }
	public abstract List<string> Architectures { get; }

	//Everything under here is for including third party lib files and managing dll stuff
	public virtual string GetConfigurationDirectory(UnrealTargetConfiguration Configuration)
	{
		return Configuration switch
		{
			UnrealTargetConfiguration.Shipping => "Release",
			UnrealTargetConfiguration.Development => "Development",
			UnrealTargetConfiguration.DebugGame or UnrealTargetConfiguration.Debug or UnrealTargetConfiguration.Test
				or UnrealTargetConfiguration.Unknown => "Debug",
			_ => throw new ArgumentOutOfRangeException()
		};
	}

	public abstract string LibrariesPath { get; }
	public abstract string LibraryPrefix { get; }
	public abstract string LibraryExtension { get; }

	public abstract string DynamicLibrariesPath { get; }
	public abstract string DynamicLibraryPrefix { get; }
	public abstract string DynamicLibraryExtension { get; }

}

public abstract class GorgeousConfiguration
{
	public abstract UnrealTargetConfiguration Configuration { get; }
}

public class GorgeousPlatform_Win64 : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.Windows;
	public override List<string> Architectures => ["x64", "Arm64", "Arm64EC"];
	public override string LibrariesPath => "Win64";
	public override string LibraryPrefix => "";
	public override string LibraryExtension => ".lib";

	public override string DynamicLibrariesPath => "Win64";
	public override string DynamicLibraryPrefix => "";
	public override string DynamicLibraryExtension => ".dll";
}

public class GorgeousPlatform_Linux : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.Linux;
	public override List<string> Architectures => ["x64"];

	public override string LibrariesPath => "Linux";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "Linux";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".so";
}

public class GorgeousPlatform_LinuxArm64 : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.Linux;
	public override List<string> Architectures => ["Arm64"];

	public override string LibrariesPath => "Linux";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "Linux";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".so";
}

public class GorgeousPlatform_Mac : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.Mac;
	public override List<string> Architectures => ["Arm64", "x64"];

	public override string LibrariesPath => "Mac";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "Mac";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".dylib";
}

public class GorgeousPlatform_Android : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.Android;
	public override List<string> Architectures => ["Arm64-v8a", "Armeabi-v7a", "x86", "x86_64"];

	public override string LibrariesPath => "Android";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "Android";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".so";
}

public class GorgeousPlatform_IOS : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.IOS;
	public override List<string> Architectures => ["Arm64", "x64"];

	public override string LibrariesPath => "IOS";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "IOS";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".dylib";
}

public class GorgeousPlatform_TVOS : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.IOS;
	public override List<string> Architectures => ["Arm64"];

	public override string LibrariesPath => "TVOS";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "TVOS";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".dylib";
}

public class GorgeousPlatform_VisionOS : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.IOS;
	public override List<string> Architectures => ["Arm64"];

	public override string LibrariesPath => "VisionOS";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "VisionOS";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".dylib";
}

public class GorgeousPlatform_Playstation : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.PlayStation;
	public override List<string> Architectures => ["x64"];

	public override string LibrariesPath => "Playstation";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "Playstation";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".so";
}

public class GorgeousPlatform_Xbox : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.Xbox;
	public override List<string> Architectures => ["x64"];

	public override string LibrariesPath => "Xbox";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "Xbox";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".so";
}

public class GorgeousPlatform_Switch : GorgeousPlatform
{
	public override GorgeousBuildPlatform Platform => GorgeousBuildPlatform.Switch;
	public override List<string> Architectures => ["x64"];

	public override string LibrariesPath => "Switch";
	public override string LibraryPrefix => "lib";
	public override string LibraryExtension => ".a";

	public override string DynamicLibrariesPath => "Switch";
	public override string DynamicLibraryPrefix => "lib";
	public override string DynamicLibraryExtension => ".so";
}


public class GorgeousConfiguration_Shipping : GorgeousConfiguration
{
	public override UnrealTargetConfiguration Configuration => UnrealTargetConfiguration.Shipping;
}

public class GorgeousConfiguration_Development : GorgeousConfiguration
{
	public override UnrealTargetConfiguration Configuration => UnrealTargetConfiguration.Development;
}

public class GorgeousConfiguration_DebugGame : GorgeousConfiguration
{
	public override UnrealTargetConfiguration Configuration => UnrealTargetConfiguration.DebugGame;
}

public class GorgeousConfiguration_Debug : GorgeousConfiguration
{
	public override UnrealTargetConfiguration Configuration => UnrealTargetConfiguration.Debug;
}

public class GorgeousConfiguration_Test : GorgeousConfiguration
{
	public override UnrealTargetConfiguration Configuration => UnrealTargetConfiguration.Test;
}

public class GorgeousConfiguration_Unknown : GorgeousConfiguration
{
	public override UnrealTargetConfiguration Configuration => UnrealTargetConfiguration.Unknown;
}