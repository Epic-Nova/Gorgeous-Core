// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "Helpers/GorgeousRecompilationHelper.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/Macros/GorgeousExtensionHelperMacros.h"

bool UGorgeousRecompilationHelper::CheckRecompilationRequirement()
{
	const UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton();
	const TArray<FString> RecordedSystems = PluginHelper->GetRecordedInstalledSystems();
	const TArray<FString> ActiveSystems = GetActiveGorgeousSystems();

	// Check for differences
	bool bRecompilationNeeded = false;

	if (RecordedSystems.Num() != ActiveSystems.Num())
	{
		bRecompilationNeeded = true;
	}
	else
	{
		for (const FString& System : ActiveSystems)
		{
			if (!RecordedSystems.Contains(System))
			{
				bRecompilationNeeded = true;
				break;
			}
		}
	}

	if (bRecompilationNeeded)
	{
		GT_W_LOG("GT.Core.Recompilation", TEXT("Recompilation required: System configuration has changed."));
		
		// Record the new state to the persistent data file
		UGorgeousPluginHelper::GetSingleton()->RecordInstalledSystems(ActiveSystems);
	}

	return bRecompilationNeeded;
}

void UGorgeousRecompilationHelper::TriggerGorgeousRecompilation()
{
	// TODO: Invoke the gorgeous installer executable that will close the engine, recompile & install needed updates or requested extensions from the gorgeous library & then repopen the engine
	GT_E_LOG("GT.Core.Recompilation", TEXT("TriggerGorgeousRecompilation: Gorgeous Installer invocation is not yet implemented."));
}

TArray<FString> UGorgeousRecompilationHelper::GetActiveGorgeousSystems()
{
	TArray<FString> Systems;

	#ifdef GORGEOUS_SYSTEM_INSTALLED_COMMONUIFOUNDATION
		Systems.Add(TEXT("CommonUIFoundation"));
	#endif

	#ifdef GORGEOUS_SYSTEM_INSTALLED_DEBUGASSIST
		Systems.Add(TEXT("DebugAssist"));
	#endif

	#ifdef GORGEOUS_SYSTEM_INSTALLED_INTERACTIONFOUNDATION
		Systems.Add(TEXT("InteractionFoundation"));
	#endif

	#ifdef GORGEOUS_SYSTEM_INSTALLED_SIGNALBRIDGE
		Systems.Add(TEXT("SignalBridge"));
	#endif

	#ifdef GORGEOUS_SYSTEM_INSTALLED_STATSFOUNDATION
		Systems.Add(TEXT("StatsFoundation"));
	#endif

	#ifdef GORGEOUS_SYSTEM_INSTALLED_PLAYLIST
		Systems.Add(TEXT("Playlist"));
	#endif

	#ifdef GORGEOUS_SYSTEM_INSTALLED_TEAM
		Systems.Add(TEXT("Team"));
	#endif
	
	#ifdef GORGEOUS_SYSTEM_INSTALLED_VISUALDATAGEATHERING
		Systems.Add(TEXT("VisualDataGathering"));
	#endif
	
	// Plugins
	#ifdef GORGEOUSTHINGS_WITH_EVENTS
		Systems.Add(TEXT("Plugin:Events"));
	#endif

	#ifdef GORGEOUSTHINGS_WITH_INVENTORY
		Systems.Add(TEXT("Plugin:Inventory"));
	#endif

	#ifdef GORGEOUSTHINGS_WITH_ENTERTAINING
		Systems.Add(TEXT("Plugin:Entertaining"));
	#endif

	return Systems;
}
