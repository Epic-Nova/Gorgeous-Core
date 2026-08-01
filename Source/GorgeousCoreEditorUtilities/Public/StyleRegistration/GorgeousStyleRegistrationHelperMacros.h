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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Styling/SlateStyleRegistry.h"
#include "StyleRegistration/GorgeousStyleRegistrationHelpers.h"
//<-------------------------------------------------------------------------->

/**
 * Macro to register a new style set for the Gorgeous Core editor utilities, which will hold all the brushes for the editor icons and thumbnails.
 *
 * @param GGorgeousStyle The shared pointer variable that will hold the created style set instance, which should be defined in the module implementation file.
 * @param StyleName The name of the style set, which will be used as a prefix for all brushes registered to this style set.
 * @param PluginName The name of the plugin that this style set belongs to, used to locate the resources for the style set.
 * @param StyleScopeCalls A block of code containing calls to the helper macros for setting brushes, which will be executed within the scope of the style set registration,
 *							allowing the brushes to be registered to the style set before it is registered to the Slate style registry.
 */
#define GORGEOUS_REGISTER_STYLE_SET(GGorgeousStyle, StyleName, PluginName, StyleScopeCalls) \
{ \
	if(GGorgeousStyle.IsValid()) { return; } \
	GGorgeousStyle = MakeShared<FSlateStyleSet>(StyleName); \
	GGorgeousStyle->SetContentRoot(FPaths::Combine(IPluginManager::Get().FindPlugin(PluginName)->GetBaseDir(), TEXT("Resources"))); \
	GorgeousStyleRegistration::GGorgeousStyleSet = GGorgeousStyle; \
	StyleScopeCalls; \
	FSlateStyleRegistry::RegisterSlateStyle(*GGorgeousStyle); \
}

/**
 * Macro to set both icon and thumbnail brushes for a given key and class name, using the same file name for both, within the scope of a style set registration.
 *
 * @param Key A unique key that identifies the asset or class these brushes represent.
 * @param FileName The base file name (without extension) of the image to be used for both the icon and thumbnail.
 * @param ClassName The name of the class for which the brushes are being set, used for class icon and thumbnail registration.
 */
#define GORGEOUS_STYLE_SET_BRUSHES(Key, FileName, ClassName) \
{ \
	GorgeousStyleRegistration::SetBrushes(GorgeousStyleRegistration::GGorgeousStyleSet, Key, FileName, ClassName); \
}

/**
 * Macro to set both icon and thumbnail brushes for a native class, using the same file name for both, within the scope of a style set registration.
 *
 * @param FileName The base file name (without extension) of the image to be used for both the icon and thumbnail.
 * @param NativeClassName The name of the native class for which the brushes are being set, used for class icon and thumbnail registration.
 */
#define GORGEOUS_STYLE_SET_NATIVE_CLASS_BRUSHES(FileName, NativeClassName) \
{ \
	GorgeousStyleRegistration::SetNativeClassBrushes(GorgeousStyleRegistration::GGorgeousStyleSet, FileName, NativeClassName); \
}

/**
 * Macro to map the brushes of a parent class to a child class, which allows the child class to use the same icon and thumbnail as the parent class without needing to set new brushes,
 * within the scope of a style set registration.
 *
 * @param ClassName The name of the child class for which the brushes are being mapped, used for class icon and thumbnail registration.
 * @param ParentClassName The name of the parent class from which to map the brushes, used for class icon and thumbnail lookup.
 */
#define GORGEOUS_STYLE_MAP_PARENT_CLASS_BRUSHES(ClassName, ParentClassName) \
{ \
	GorgeousStyleRegistration::MapParentClassBrushes(GorgeousStyleRegistration::GGorgeousStyleSet, ClassName, ParentClassName); \
}

/**
 * Macro to set both icon and thumbnail brushes for a class, using an SVG file from the engine content directory, within the scope of a style set registration.
 *
 * @param ClassName The name of the class for which the brushes are being set, used for class icon and thumbnail registration.
 * @param RelativeSvgPath The relative path to the SVG file within the engine content directory, without the file extension, used to locate the SVG file for both the icon and thumbnail.
 */
#define GORGEOUS_STYLE_MAP_ENGINE_SVG_BRUSHES(ClassName, RelativeSvgPath) \
{ \
	GorgeousStyleRegistration::MapEngineSvgBrushes(GorgeousStyleRegistration::GGorgeousStyleSet, ClassName, RelativeSvgPath); \
}

/**
 * Registers a TrueType / OpenType font file into the current Gorgeous style set.
 * The font can be retrieved at runtime via ISlateStyle::GetFontStyle(FontKey)
 * or by calling FSlateApplication::Get().GetRenderer()->GetFontAtlas().
 *
 * @param FontKey      Style key used to look up the font (e.g. "GorgeousCore.EmojiFont").
 * @param RelativePath Path to the font file relative to the plugin Resources directory, WITHOUT extension.
 * @param DefaultSize  Default point size stored in the FSlateFontInfo (e.g. 14).
 */
#define GORGEOUS_STYLE_REGISTER_FONT(FontKey, RelativePath, DefaultSize) \
{ \
	GorgeousStyleRegistration::RegisterFont(GorgeousStyleRegistration::GGorgeousStyleSet, FontKey, RelativePath, DefaultSize); \
}

#define GORGEOUS_UNREGISTER_STYLE_SET(GGorgeousStyle) \
{ \
	if (!GGorgeousStyle.IsValid()) { return; } \
	FSlateStyleRegistry::UnRegisterSlateStyle(*GGorgeousStyle); \
	GGorgeousStyle.Reset(); \
	GorgeousStyleRegistration::GGorgeousStyleSet.Reset(); \
}