// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

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

/*==================================================================================================>
| This standalone build file for Unreal Engine 4/5 is designed to provide a clean and well 			|
| structured build file for all build products. It is designed to be used with any Unreal Engine 	|
| plugin and project but was originally designed for the Gorgeous Things Plugin.					|
| -------------------------------------------- Features ------------------------------------------- |
| Provides pre configured UBT definitions, functions and settings to:								|
|	- Properly build Gorgeous Things with all available and eligible extensions.					|
|	- Provide a clean and well-structured build file for all build products.						|
| 	- Easy configurable cross dependant modules.													|
|	- Provide a worry free opt in setup for external modules.										|
|	- Provide preprocessor macros to every gorgeous plugin to grant the ability for cross			|
|		functionality sharing and extending.														|
<==================================================================================================*/
//@TODO: boolean flag that can be set to specify that a gorgeous plugin includes an utilities module either for editor or runtime or both
//@TODO: Every Module coming from a Gorgeous Things extension should depend on this build file.
