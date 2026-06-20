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
#include "GorgeousCoreRuntimeUtilitiesModule.h"
#include "Containers/Ticker.h"
#include "Helpers/GorgeousPluginHelper.h"

void FGorgeousCoreRuntimeUtilitiesModule::GorgeousStartupModule()
{
	// Schedule binary checksum generation 10 seconds after boot
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float DeltaTime)
	{
		UGorgeousPluginHelper::GenerateAndSaveBinaryChecksum();
		return false; // Run exactly once
	}), 10.0f);
}

IMPLEMENT_MODULE(FGorgeousCoreRuntimeUtilitiesModule, GorgeousCoreRuntimeUtilities)