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
#include "GorgeousButtonProcessor.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Button Processor
| Functional Name: UGorgeousButtonProcessor
| Parent Class: UGorgeousUIProcessor
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Processor for updating buttons.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/GorgeousButtonProcessor",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousButtonProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

	// Initializes the processor for button widgets.
	UGorgeousButtonProcessor();

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Routes a signal payload to a button widget.
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;

	// Applies resolved theme values to a button widget.
	virtual void ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme = nullptr) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
};