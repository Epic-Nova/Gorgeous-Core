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
#include "GameFramework/CheatManager.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousSignalBridgeCheatManagerExtension.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Signal Bridge Cheat Manager Extension
| Functional Name: UGorgeousSignalBridgeCheatManagerExtension
| Parent Class: UCheatManagerExtension
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides developer console commands for inspecting and locally testing the
| Signal Bridge system.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/GorgeousSignalBridgeCheatManagerExtension",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousSignalBridgeCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:

	// Lists the current Signal Bridge listener and dispatch counters.
	UFUNCTION(Exec)
	void Cheat_ListSignalBridge();

	/**
	 * Registers a networked public signal rule on the authoritative bridge.
	 *
	 * @param SignalTag The gameplay tag that identifies the signal to register.
	 */
	UFUNCTION(Exec)
	void Cheat_RegisterPublicSignal(FString SignalTag);

	/**
	 * Dispatches an empty payload to local listeners without networking.
	 *
	 * @param SignalTag The gameplay tag to dispatch locally.
	 */
	UFUNCTION(Exec)
	void Cheat_DispatchLocalSignal(FString SignalTag);

	/**
	 * Removes this player's local and remote listeners for a signal.
	 *
	 * @param SignalTag The gameplay tag whose listeners should be cleared.
	 */
	UFUNCTION(Exec)
	void Cheat_ClearSignalListeners(FString SignalTag);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};