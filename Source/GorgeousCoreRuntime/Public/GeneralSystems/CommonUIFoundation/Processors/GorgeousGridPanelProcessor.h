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
#include "GorgeousGridPanelProcessor.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Grid Panel Processor
| Functional Name: UGorgeousGridPanelProcessor
| Parent Class: UGorgeousUIProcessor
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Advanced processor for Grid Panels. Can handle dynamic slot management and
| child injection.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/GorgeousGridPanelProcessor",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Processors/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousGridPanelProcessor : public UGorgeousUIProcessor
{
	GENERATED_BODY()

	// Initializes the processor for grid panel widgets.
	UGorgeousGridPanelProcessor();

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Routes a signal payload to a grid panel widget.
	virtual void OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
};