// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "GorgeousProgressBarProcessor.generated.h"

struct FGorgeousProgressBarInterpState
{
	float CurrentPercent = 0.0f;
	float TargetPercent  = 0.0f;
	bool  bActive        = false;
};

/**
 * Processor for UGorgeousCommonProgressBar.
 *
 * Interpolation is driven by the ENGINE tick via FTicker — since this processor
 * is shared (flyweight), per-widget state is tracked in a map keyed by widget pointer.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousProgressBarProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

public:
	UGorgeousProgressBarProcessor();
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;

	virtual void BeginDestroy() override;

	/** Interpolation speed applied to all managed progress bars. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Juicy")
	float InterpSpeed = 10.0f;

private:
	void StartTicking();
	void StopTicking();
	bool TickInterp(float DeltaTime);

	/** Per-widget interpolation state — keyed by weak pointer to avoid leaks. */
	TMap<TWeakObjectPtr<UObject>, FGorgeousProgressBarInterpState> InterpStates;

	FTSTicker::FDelegateHandle TickHandle;
};
