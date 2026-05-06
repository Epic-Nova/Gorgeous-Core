// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousProgressBarProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonProgressBar.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "Containers/Ticker.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"

UGorgeousProgressBarProcessor::UGorgeousProgressBarProcessor()
{
	TargetWidgetClass = UGorgeousCommonProgressBar::StaticClass();
}

void UGorgeousProgressBarProcessor::OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload)
{
	UGorgeousCommonProgressBar* ProgressBar = Cast<UGorgeousCommonProgressBar>(Widget);
	if (!ProgressBar) return;

	const FGorgeousUIUpdatePayload* MasterPayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!MasterPayload) return;

	// @TODO:If an animation tag is present we don't currently support playing UMG animations
	// on native progress bar widgets; fall back to normal interpolation.

	bool bNeedsTick = false;

	for (const auto& Pair : MasterPayload->Updates)
	{
		if (Pair.Key == FName("Percent") || Pair.Key == FName("Value"))
		{
			float FloatOut = 0.0f;
			if (GorgeousUIInstanced::TryGetFloat(Pair.Value, FloatOut))
			{
				FGorgeousProgressBarInterpState& State = InterpStates.FindOrAdd(Widget);
				State.TargetPercent = FloatOut;
				State.bActive = !FMath::IsNearlyEqual(State.CurrentPercent, State.TargetPercent);
				if (State.bActive) bNeedsTick = true;
			}
		}
		else
		{
			// Universal reflection for everything else (FillColor, Visibility, etc.)
			ApplyPropertyToTarget(ProgressBar, Pair.Key, Pair.Value);
		}
	}

	if (bNeedsTick && !TickHandle.IsValid())
	{
		StartTicking();
	}
}

void UGorgeousProgressBarProcessor::BeginDestroy()
{
	StopTicking();
	Super::BeginDestroy();
}

void UGorgeousProgressBarProcessor::StartTicking()
{
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UGorgeousProgressBarProcessor::TickInterp));
}

void UGorgeousProgressBarProcessor::StopTicking()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

bool UGorgeousProgressBarProcessor::TickInterp(float DeltaTime)
{
	bool bAnyActive = false;

	for (auto It = InterpStates.CreateIterator(); It; ++It)
	{
		// Remove entries for destroyed widgets automatically
		if (!It->Key.IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		FGorgeousProgressBarInterpState& State = It->Value;
		if (!State.bActive) continue;

		UGorgeousCommonProgressBar* ProgressBar = Cast<UGorgeousCommonProgressBar>(It->Key.Get());
		if (!ProgressBar)
		{
			It.RemoveCurrent();
			continue;
		}

		if (!FMath::IsNearlyEqual(State.CurrentPercent, State.TargetPercent, 0.0001f))
		{
			State.CurrentPercent = FMath::FInterpTo(State.CurrentPercent, State.TargetPercent, DeltaTime, InterpSpeed);
			ProgressBar->SetPercent(State.CurrentPercent);
			bAnyActive = true;
		}
		else
		{
			// Snap and stop
			State.CurrentPercent = State.TargetPercent;
			ProgressBar->SetPercent(State.CurrentPercent);
			State.bActive = false;
		}
	}

	// Return false to stop the ticker when nothing is animating
	if (!bAnyActive)
	{
		TickHandle.Reset();
		return false;
	}

	return true;
}
