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
#include "UObject/NoExportTypes.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIProcessor.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UObject;
class UGorgeousUITheme_DA;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIProcessor
| Functional Name: UGorgeousUIProcessor
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
	| Base class for shared UI processors that apply incoming signal payloads to
	| concrete widget instances without retaining per-widget state.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, Blueprintable, Abstract,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIProcessor : public UObject
{
	GENERATED_BODY()

	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// The base widget/object class this processor handles. Used for automatic registration in the Subsystem.
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous UI")
	TSubclassOf<UObject> TargetWidgetClass;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:
	/**
	 * Applies an incoming signal payload to the supplied widget instance.
	 *
	 * @param Widget The widget that owns the received signal.
	 * @param SignalTag The tag that identifies the received signal.
	 * @param Payload The data carried by the received signal.
	 */
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/**
	 * Universally applies a value to a target object using reflection.
	 *
	 * @param Target The object that receives the resolved value.
	 * @param PropertyName The target property or setter name.
	 * @param Value The value to assign through reflection.
	 * @return True if the property was successfully applied.
	 */
	static bool ApplyPropertyToTarget(UObject* Target, FName PropertyName, const FInstancedStruct& Value);

	/**
	 * Applies a style value when the target allows the named property.
	 *
	 * @param Target The object that receives the style value.
	 * @param PropertyName The style property to resolve.
	 * @param Value The resolved style value to apply.
	 * @return True if the style property was applied.
	 */
	static bool ApplyStylePropertyToTarget(UObject* Target, FName PropertyName, const FInstancedStruct& Value);

	/**
	 * Determines whether a target accepts updates to a style property.
	 *
	 * @param Target The object whose style policy is checked.
	 * @param PropertyName The style property to test.
	 * @return True if the property may receive a styling update.
	 */
	static bool IsStylePropertyAllowed(const UObject* Target, FName PropertyName);

	/**
	 * Retrieves the project default theme from developer settings.
	 *
	 * @return The configured default theme, or null when none is configured.
	 */
	static const UGorgeousUITheme_DA* GetDefaultTheme();

	/**
	 * Applies resolved theme properties to a widget using an optional fallback.
	 *
	 * @param Target The widget or object that receives the theme values.
	 * @param PrimaryTheme The preferred source of theme values.
	 * @param FallbackTheme The secondary source used for unresolved values.
	 */
	static void ApplyThemeToWidgetInternal(UObject* Target, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr);

	/**
	 * Applies resolved theme properties through this processor instance.
	 *
	 * @param Widget The widget that receives the theme values.
	 * @param PrimaryTheme The preferred source of theme values.
	 * @param FallbackTheme The secondary source used for unresolved values.
	 */
	virtual void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr);

#if WITH_EDITOR
	/**
	 * Applies editor-time theme evaluation using project settings.
	 *
	 * @param Target The widget or object that receives the previewed theme.
	 */
	static void ApplyEditorThemeToWidget(UObject* Target);
#endif

private:
	/**
	 * Resolves a property value from a theme.
	 *
	 * @param Theme The theme to query.
	 * @param Key The property key to resolve.
	 * @param OutValue Receives the resolved property value.
	 * @return True if the theme contains the requested property.
	 */
	static bool ResolveThemeProperty(const UGorgeousUITheme_DA* Theme, FName Key, FInstancedStruct& OutValue);

	/**
	 * Resolves a property value from a primary theme with a fallback theme.
	 *
	 * @param PrimaryTheme The preferred theme to query.
	 * @param FallbackTheme The theme used when the primary lacks a value.
	 * @param BindingTag The widget binding tag used to scope the property.
	 * @param PropertyName The property name to resolve.
	 * @param OutValue Receives the resolved property value.
	 * @return True if either theme contains the requested property.
	 */
	static bool ResolveThemePropertyWithFallback(const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme, const FGameplayTag& BindingTag, FName PropertyName, FInstancedStruct& OutValue);

	/**
	 * Applies interpolated theme colors to a target object.
	 *
	 * @param Target The object receiving interpolated colors.
	 */
	static void ApplyInterpolatedThemeColors(UObject* Target);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};