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
			
			GT_W_LOG_FULL_EX("GT.InsightMatrix.Provider.Missing", 
				TEXT("Insight Matrix provider missing for plugin '%s', some functionality may not work as expected!"),
				5.0f,
				true,
				true,
				true,
				true,
				nullptr,
				nullptr,
				*GetBelongingPluginName().ToString()
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