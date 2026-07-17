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
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousProgressBarProcessor.generated.h"
//<-------------------------------------------------------------------------->

struct FGorgeousProgressBarInterpState
{
	float CurrentPercent = 0.0f;
	float TargetPercent  = 0.0f;
	bool  bActive        = false;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Progress Bar Processor
| Functional Name: UGorgeousProgressBarProcessor
| Parent Class: UGorgeousUIProcessor
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Processor for UGorgeousCommonProgressBar. Interpolation is driven by the
| ENGINE tick via FTicker, since this processor is shared (flyweight),
| per-widget state is tracked in a map keyed by widget pointer.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/GorgeousProgressBarProcessor",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousProgressBarProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

	// Initializes the processor for progress bar widgets.
	UGorgeousProgressBarProcessor();

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Routes a signal payload to a progress bar widget.
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;

	// Applies resolved theme values to a progress bar widget.
	virtual void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr) override;

	// Stops the ticker before the processor is destroyed.
	virtual void BeginDestroy() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// Interpolation speed applied to all managed progress bars.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Juicy")
	float InterpSpeed = 10.0f;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:
	// Starts ticking while interpolation work is pending.
	void StartTicking();

	// Stops ticking when no interpolation work remains.
	void StopTicking();

	/**
	 * Advances interpolation for each managed progress bar.
	 *
	 * @param DeltaTime The elapsed time since the previous ticker callback.
	 * @return True while the ticker should continue running.
	 */
	bool TickInterp(float DeltaTime);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
private:
	// Stores interpolation state per widget through weak pointers.
	TMap<TWeakObjectPtr<UObject>, FGorgeousProgressBarInterpState> InterpStates;

	// Holds the active core ticker registration.
	FTSTicker::FDelegateHandle TickHandle;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};