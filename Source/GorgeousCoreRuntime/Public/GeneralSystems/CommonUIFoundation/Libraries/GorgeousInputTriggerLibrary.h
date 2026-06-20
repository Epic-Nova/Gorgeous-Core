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
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<--------------------------=== Module Includes ===------------------------->
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousInputTriggerLibrary.generated.h"
//<-------------------------------------------------------------------------->

class UInputTrigger;
class UInputTriggerHold;
class UInputTriggerTap;
class UInputTriggerPulse;

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Input Trigger Library
| Functional Name: UGorgeousInputTriggerLibrary
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Function Library providing static helper functions to modify Enhanced Input Trigger parameters at runtime.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInputTriggerLibrary : public UGorgeous
{
	GENERATED_BODY()

public:

	/**
	 * Sets the hold time threshold for an input trigger. Useful for dynamically adjusting input responsiveness based on game state or player preferences.
	 * 
	 * @param Trigger The input trigger to modify. Safely handles UInputTriggerHold and UInputTriggerHoldAndRelease.
	 * @param HoldTimeThreshold The new hold time threshold in seconds. Must be non-negative
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Extensions|Input")
	static void SetInputTriggerHoldTimeThreshold(UInputTrigger* Trigger, const float HoldTimeThreshold);

	/**
	 * Sets whether a Hold trigger should fire only once (OneShot) or repeatedly.
	 * 
	 * @param Trigger The input trigger to modify. Must be a UInputTriggerHold.
	 * @param bIsOneShot If true, the trigger fires only once when the hold time is met.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Extensions|Input")
	static void SetInputTriggerIsOneShot(UInputTriggerHold* Trigger, const bool bIsOneShot);

	/**
	 * Sets the maximum duration a Tap trigger can be held before it is considered canceled.
	 * 
	 * @param Trigger The input trigger to modify. Must be a UInputTriggerTap.
	 * @param TapReleaseTimeThreshold The max time the key can be held down and still be considered a tap.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Extensions|Input")
	static void SetInputTriggerTapReleaseTimeThreshold(UInputTriggerTap* Trigger, const float TapReleaseTimeThreshold);

	/**
	 * Sets the interval and trigger limit for a Pulse trigger.
	 * 
	 * @param Trigger The input trigger to modify. Must be a UInputTriggerPulse.
	 * @param Interval How often the pulse fires (in seconds).
	 * @param TriggerLimit How many times it can trigger before completing. 0 = unlimited.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Extensions|Input")
	static void SetInputTriggerPulseIntervalAndLimit(UInputTriggerPulse* Trigger, const float Interval, const int32 TriggerLimit);

	/**
	 * Extracts the FInputActionValue from a given FInputActionInstance.
	 * 
	 * @param Instance The input action instance, typically passed from HandleGorgeousInputAdvanced.
	 * @return The evaluated input action value (e.g. Bool, Axis1D, Axis2D) for this frame.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Extensions|Input")
	static struct FInputActionValue GetActionValueFromInstance(const struct FInputActionInstance& Instance);
};

using UGT_InputTrigger_FL = UGorgeousInputTriggerLibrary;
