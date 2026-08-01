// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Libraries/GorgeousInputTriggerLibrary.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "InputTriggers.h"
#include "InputAction.h"
//<-------------------------------------------------------------------------->

void UGorgeousInputTriggerLibrary::SetInputTriggerHoldTimeThreshold(UInputTrigger* Trigger, const float HoldTimeThreshold)
{
	if (!Trigger)
	{
		return;
	}

	if (UInputTriggerHold* HoldTrigger = Cast<UInputTriggerHold>(Trigger))
	{
		HoldTrigger->HoldTimeThreshold = FMath::Max(0.0f, HoldTimeThreshold);
	}
	else if (UInputTriggerHoldAndRelease* HoldReleaseTrigger = Cast<UInputTriggerHoldAndRelease>(Trigger))
	{
		HoldReleaseTrigger->HoldTimeThreshold = FMath::Max(0.0f, HoldTimeThreshold);
	}
}

void UGorgeousInputTriggerLibrary::SetInputTriggerIsOneShot(UInputTriggerHold* Trigger, const bool bIsOneShot)
{
	if (Trigger)
	{
		Trigger->bIsOneShot = bIsOneShot;
	}
}

void UGorgeousInputTriggerLibrary::SetInputTriggerTapReleaseTimeThreshold(UInputTriggerTap* Trigger, const float TapReleaseTimeThreshold)
{
	if (Trigger)
	{
		Trigger->TapReleaseTimeThreshold = FMath::Max(0.0f, TapReleaseTimeThreshold);
	}
}

void UGorgeousInputTriggerLibrary::SetInputTriggerPulseIntervalAndLimit(UInputTriggerPulse* Trigger, const float Interval, const int32 TriggerLimit)
{
	if (Trigger)
	{
		Trigger->Interval = FMath::Max(0.0f, Interval);
		Trigger->TriggerLimit = FMath::Max(0, TriggerLimit);
	}
}

FInputActionValue UGorgeousInputTriggerLibrary::GetActionValueFromInstance(const FInputActionInstance& Instance)
{
	return Instance.GetValue();
}
