// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "GorgeousCoreCmdletHandler.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousCoreCmdletHandler Implementation
//=============================================================================

//@TODO: Hook up the documentaiton under "Help"

void UGorgeousCoreCmdletHandler::PostInitProperties()
{
	Super::PostInitProperties();
	
	RegisterConsoleCommands();
}

void UGorgeousCoreCmdletHandler::RegisterConsoleCommands()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("gorgeous.suppress"),
		TEXT("Suppresses log messages associated with the specified logging key."),
		FConsoleCommandWithArgsDelegate::CreateLambda([this](const TArray<FString>& Args)
		{
			SuppressLoggingKey(Args);
		}),
		ECVF_Default
	);
}

void UGorgeousCoreCmdletHandler::SuppressLoggingKey(const TArray<FString>& Args)
{
	if (Args.Num() == 0)
	{
		GT_W_LOG("GT.Suppress_Invalid", TEXT("No arguments key. Usage: gorgeous.suppress <LoggingKey> <ShouldSuppress>"));
		return;
	}

	const FString LoggingKey = Args[0];
	const bool bShouldSuppress = Args.Num() > 1 ? Args[1].ToBool() : true;
	GorgeousLogging::SetLoggingKeySuppressed(*LoggingKey, bShouldSuppress);
}
