// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousSingleton.h"
//<=============================--- Includes ---=============================>
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousCoreCmdletHandler.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Handles console commands related to Gorgeous Core.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCoreCmdletHandler : public UObject
{
	GENERATED_BODY()
	
public:
	
	virtual void PostInitProperties() override;
	
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