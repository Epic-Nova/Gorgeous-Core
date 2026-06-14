// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GorgeousUIProcessor.generated.h"

class UObject;
class UGorgeousUITheme_DA;

/**
 * Base class for all UI Processor objects.
 *
 * Processors follow the FLYWEIGHT pattern — one shared instance per type exists for
 * the entire game session. The Subsystem routes each incoming signal to the correct
 * shared processor together with the concrete widget context.
 *
 * Key rules for subclasses:
 *   - Do NOT store per-widget state. Every method receives the widget as a parameter.
 *   - Override OnSignalReceived to handle your widget type's payload.
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class GORGEOUSCORERUNTIME_API UGorgeousUIProcessor : public UObject
{
	GENERATED_BODY()

public:
	/** 
	 * The base widget/object class this processor handles.
	 * Used for automatic registration in the Subsystem.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous UI")
	TSubclassOf<UObject> TargetWidgetClass;

public:
	/**
	 * Called when a signal arrives for a widget managed by this processor.
	 * The Widget context is always passed in — DO NOT cache it.
	 */
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload);

	/**
	 * Universally applies a value to a target object using reflection.
	 *
	 * Resolution order:
	 *   1. Direct UPROPERTY match by name.
	 *   2. UFUNCTION match with a "Set" prefix (e.g. "Percent" -> SetPercent).
	 *   3. Boolean prefix strip ('b') and retry.
	 *
	 * @return true if the property was successfully applied.
	 */
	static bool ApplyPropertyToTarget(UObject* Target, FName PropertyName, const FInstancedStruct& Value);

	/** Applies a style value to a target object using the style allow list. */
	static bool ApplyStylePropertyToTarget(UObject* Target, FName PropertyName, const FInstancedStruct& Value);

	/** Returns true if the property is allowed for styling updates. */
	static bool IsStylePropertyAllowed(const UObject* Target, FName PropertyName);

	/** Returns the project default theme from developer settings. */
	static const UGorgeousUITheme_DA* GetDefaultTheme();

	/** Applies resolved theme properties to a widget (uses binding tag prefix + fallback). */
	static void ApplyThemeToWidgetInternal(UObject* Target, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr);

	/** Applies resolved theme properties to a widget via a processor instance. */
	virtual void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr);

#if WITH_EDITOR
	/** Applies editor-time theme evaluation using project settings. */
	static void ApplyEditorThemeToWidget(UObject* Target);
#endif

private:
	static bool ResolveThemeProperty(const UGorgeousUITheme_DA* Theme, FName Key, FInstancedStruct& OutValue);
	static bool ResolveThemePropertyWithFallback(const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme, const FGameplayTag& BindingTag, FName PropertyName, FInstancedStruct& OutValue);
	static void ApplyInterpolatedThemeColors(UObject* Target);
};
