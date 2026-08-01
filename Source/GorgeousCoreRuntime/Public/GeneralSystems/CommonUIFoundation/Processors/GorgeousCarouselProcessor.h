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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCarouselProcessor.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Carousel Processor
| Functional Name: UGorgeousCarouselProcessor
| Parent Class: UGorgeousUIProcessor
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Processor for the Gorgeous Carousel widget. Handles navigation and child
| injection via signals.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/GorgeousCarouselProcessor",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousCarouselProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

	// Initializes the processor for carousel widgets.
	UGorgeousCarouselProcessor();

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Routes a signal payload to a carousel widget.
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
};