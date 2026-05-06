// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GorgeousUIProcessor.generated.h"

class UObject;

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
};
