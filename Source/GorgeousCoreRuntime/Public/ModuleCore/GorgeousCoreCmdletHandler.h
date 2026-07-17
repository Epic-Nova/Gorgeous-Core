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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCoreCmdletHandler.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Core Cmdlet Handler
| Functional Name: UGorgeousCoreCmdletHandler
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Handles console commands related to Gorgeous Core.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ModuleCore/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ModuleCore/GorgeousCoreCmdletHandler",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ModuleCore/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousCoreCmdletHandler : public UObject
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
#pragma region Overrides

public:

	// Called after the object's properties have been initialized. Used here to register console commands.
	virtual void PostInitProperties() override;
	//<------------------------------------------------------------------------->
#pragma endregion Overrides

private:

	/**
	 * Registers console commands related to Gorgeous Object Variables.
	 */
	void RegisterConsoleCommands();

	/**
	 * Lists Gorgeous Variables based on the provided arguments.
	 *
	 * @param Args The arguments provided to the console command.
	 */
	void SuppressLoggingKey(const TArray<FString>& Args);
};