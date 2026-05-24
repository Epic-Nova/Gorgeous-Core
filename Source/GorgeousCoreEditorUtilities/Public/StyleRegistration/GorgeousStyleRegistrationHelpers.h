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

#include "Fonts/CompositeFont.h"
#include "Fonts/SlateFontInfo.h"

namespace GorgeousStyleRegistration
{
	// Shorthand constants for icon sizes (16px)
	const FVector2D Icon16(16.0f, 16.0f);
	
	// Shorthand constants for thumbnail sizes (128px)
	const FVector2D Thumb128(128.0f, 128.0f);
	
	// The shared pointer to the style set that will hold all the brushes for Gorgeous Core editor utilities.
	static TSharedPtr<FSlateStyleSet> GGorgeousStyleSet;
	
	/**
	 * Helper function to set both icon and thumbnail brushes for a given key and class name, using the same file name for both.
	 *
	 * @param Style The style set to which the brushes will be added.
	 * @param Key A unique key that identifies the asset or class these brushes represent.
	 * @param FileName The base file name (without extension) of the image to be used for both the icon and thumbnail.
	 * @param ClassName The name of the class for which the brushes are being set, used for class icon and thumbnail registration.
	 */
	FORCEINLINE void SetBrushes(const TSharedPtr<FSlateStyleSet>& Style, const FString& Key, const FString& FileName, const FString& ClassName)
	{
		Style->Set(*FString::Printf(TEXT("GorgeousCore.%s.Icon"), *Key),
			new FSlateImageBrush(Style->RootToContentDir(FileName, TEXT(".png")), Icon16));
		Style->Set(*FString::Printf(TEXT("GorgeousCore.%s.Thumbnail"), *Key),
			new FSlateImageBrush(Style->RootToContentDir(FileName, TEXT(".png")), Thumb128));

		Style->Set(*FString::Printf(TEXT("ClassIcon.%s"), *ClassName),
			new FSlateImageBrush(Style->RootToContentDir(FileName, TEXT(".png")), Icon16));
		Style->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *ClassName),
			new FSlateImageBrush(Style->RootToContentDir(FileName, TEXT(".png")), Thumb128));
	};

	/**
	 * Helper function to set both icon and thumbnail brushes for a native class, using the same file name for both.
	 *
	 * @param Style The style set to which the brushes will be added.
	 * @param FileName The base file name (without extension) of the image to be used for both the icon and thumbnail.
	 * @param NativeClassName The name of the native class for which the brushes are being set, used for class icon and thumbnail registration.
	 */
	FORCEINLINE void SetNativeClassBrushes(const TSharedPtr<FSlateStyleSet>& Style, const FString& FileName, const FString& NativeClassName)
	{
		Style->Set(*FString::Printf(TEXT("ClassIcon.%s"), *NativeClassName),
			new FSlateImageBrush(Style->RootToContentDir(FileName, TEXT(".png")), Icon16));
		Style->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *NativeClassName),
			new FSlateImageBrush(Style->RootToContentDir(FileName, TEXT(".png")), Thumb128));
	};

	/**
	 * Helper function to set both icon and thumbnail brushes for a native class, using the same file name for both, with a key prefix for better organization.
	 *
	 * @param Style The style set to which the brushes will be added.
	 * @param ClassName The name of the class for which the brushes are being set, used for class icon and thumbnail registration.
	 * @param ParentClassName The name of the parent class from which to copy the brushes, used to retrieve the brushes for both the icon and thumbnail.
	 */
	FORCEINLINE void MapParentClassBrushes(const TSharedPtr<FSlateStyleSet>& Style, const FString& ClassName, const FString& ParentClassName)
	{
		Style->Set(*FString::Printf(TEXT("ClassIcon.%s"), *ClassName),
			const_cast<FSlateBrush*>(FAppStyle::Get().GetBrush(*FString::Printf(TEXT("ClassIcon.%s"), *ParentClassName))));
		Style->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *ClassName),
			const_cast<FSlateBrush*>(FAppStyle::Get().GetBrush(*FString::Printf(TEXT("ClassThumbnail.%s"), *ParentClassName))));
	};

	/**
	 * Helper function to set both icon and thumbnail brushes for a class, using an SVG file from the engine content directory.
	 *
	 * @param Style The style set to which the brushes will be added.
	 * @param ClassName The name of the class for which the brushes are being set, used for class icon and thumbnail registration.
	 * @param RelativeSvgPath The relative path (from the engine content directory) to the SVG file to be used for both the icon and thumbnail.
	 */
	FORCEINLINE void MapEngineSvgBrushes(const TSharedPtr<FSlateStyleSet>& Style, const FString& ClassName, const FString& RelativeSvgPath)
	{
		const FString SvgPath = FPaths::Combine(FPaths::EngineContentDir(), TEXT("Editor/Slate/Starship"), RelativeSvgPath);
		Style->Set(*FString::Printf(TEXT("ClassIcon.%s"), *ClassName),
			new FSlateVectorImageBrush(SvgPath, Icon16));
		Style->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *ClassName),
			new FSlateVectorImageBrush(SvgPath, Thumb128));
	};

	/**
	 * Helper function to register a TrueType / OpenType font file into a Slate style set.
	 * The font is stored under FontKey and can be retrieved via
	 * Style->GetFontStyle(FontKey) or ISlateStyle::GetFontStyle.
	 *
	 * @param Style          The style set to register the font into.
	 * @param FontKey        The style key used to look up the font (e.g. "GorgeousCore.EmojiFont").
	 * @param RelativePath   Path to the font file relative to the style set's content root,
	 *                       WITHOUT the .ttf / .otf extension.
	 * @param DefaultSize    Point size stored with the font info. Callers can override it at use-site.
	 */
	FORCEINLINE void RegisterFont(const TSharedPtr<FSlateStyleSet>& Style, const FString& FontKey, const FString& RelativePath, int32 DefaultSize = 14)
	{
		const FString FontPath = Style->RootToContentDir(RelativePath, TEXT(".ttf"));
		// Build a composite font so we avoid the deprecated string-path FSlateFontInfo constructor.
		const TSharedRef<FCompositeFont> CompositeFont = MakeShared<FCompositeFont>();
		FTypefaceEntry& Entry = CompositeFont->DefaultTypeface.Fonts.AddDefaulted_GetRef();
		Entry.Name = TEXT("Regular");
		Entry.Font = FFontData(FontPath, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
		const FSlateFontInfo FontInfo(TSharedPtr<const FCompositeFont>(CompositeFont), static_cast<float>(DefaultSize));
		Style->Set(*FontKey, FontInfo);
	};
}
