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
#include "IGorgeousThingsModuleInterface.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousPluginHelper IGorgeousThingsModuleInterface
//=============================================================================

EGorgeousModuleLoadFailureHandling IGorgeousThingsModuleInterface::GetLoadFailureHandling() const
{
	return EGorgeousModuleLoadFailureHandling::ContinueOnFailure;
}

bool IGorgeousThingsModuleInterface::ProvidesCoreFunctionality() const
{
	return false;
}

void IGorgeousThingsModuleInterface::StartupModule()
{
	IModuleInterface::StartupModule();
	
	if (UGorgeousPluginHelper::GetSingleton()->RegisterModule(this))
	{
		GorgeousStartupModule();

		if (GetModuleFunctionality() == EGorgeousModuleFunctionality::Runtime && !GetInsightProvider())
		{
			const FString Message = FString::Printf(TEXT("Insight Matrix provider missing for plugin '%s'."), *GetBelongingPluginName().ToString());
			GT_W_LOG_MESSAGE_FULL_EX(
				Message,
				"GT.InsightMatrix.Provider.Missing",
				5.0f,
				true,
				true,
				true,
				true,
				nullptr,
				nullptr
			);
		}
	}
}

void IGorgeousThingsModuleInterface::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
	GorgeousShutdownModule();
	UGorgeousPluginHelper::GetSingleton()->UnregisterModule(this);
}