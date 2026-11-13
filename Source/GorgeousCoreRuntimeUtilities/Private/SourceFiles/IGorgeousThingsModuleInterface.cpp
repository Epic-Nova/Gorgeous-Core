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
#include "IGorgeousThingsModuleInterface.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Features/IModularFeatures.h"
#include "UObject/UObjectBase.h"
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
		CompleteGorgeousStartup();
	}
}

void IGorgeousThingsModuleInterface::CompleteGorgeousStartup()
{
	if (!bGorgeousModuleStarted)
	{
		bGorgeousModuleStarted = true;
		GorgeousStartupModule();

		// Insight providers are optional. Ordinary runtime modules need no developer suite.

		if (InsightProvider)
		{
			IModularFeatures::Get().RegisterModularFeature(FName(TEXT("GorgeousInsightMatrixProvider")), InsightProvider);
		}
	}
}

void IGorgeousThingsModuleInterface::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
	
	if (bGorgeousModuleStarted && InsightProvider)
	{
		IModularFeatures::Get().UnregisterModularFeature(FName(TEXT("GorgeousInsightMatrixProvider")), InsightProvider);
	}

	if (bGorgeousModuleStarted)
	{
		bGorgeousModuleStarted = false;
		GorgeousShutdownModule();
	}
	if (UObjectInitialized() && !GExitPurge)
	{
		if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton()) Helper->UnregisterModule(this);
	}
}
