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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIFoundationSignalForwarder.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousUIFoundationSubsystem;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Signal Bridge Tag Forwarder
| Functional Name: UGorgeousSignalBridgeTagForwarder
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Signal Bridge Tag Forwarder.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousSignalBridgeTagForwarder",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousSignalBridgeTagForwarder : public UObject
{
	GENERATED_BODY()


	//<============================--- Variables ---============================>
	#pragma region Variables
public:
	// Signal Bridge tag forwarded by this object.
	UPROPERTY()
	FGameplayTag Tag;

	// UI Foundation subsystem that receives forwarded payloads.
	UPROPERTY()
	TObjectPtr<UGorgeousUIFoundationSubsystem> OwningSubsystem;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
public:

	/**
	 * Forwards a Signal Bridge payload to the owning UI Foundation subsystem.
	 *
	 * @param SignalTag The received signal tag.
	 * @param Payload The received signal payload.
	 */
	UFUNCTION()
	void OnBridgePayload(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks
};