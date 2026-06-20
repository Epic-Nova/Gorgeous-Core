// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GorgeousRecompilationHelper.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/Macros/GorgeousExtensionHelperMacros.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "FileHelpers.h"

bool UGorgeousRecompilationHelper::bIsRecompilationRequired = false;
FSimpleMulticastDelegate UGorgeousRecompilationHelper::OnRecompilationRequirementChanged;

bool UGorgeousRecompilationHelper::IsRecompilationRequired()
{
	return bIsRecompilationRequired;
}

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

	// Always record the active systems if it's the first time running
	// or if we detected a change. This ensures the array is populated.
	if (bRecompilationNeeded || RecordedSystems.Num() == 0)
	{
		if (bRecompilationNeeded)
		{
			GT_W_LOG("GT.Core.Recompilation", TEXT("Recompilation required: System configuration has changed."));
			bIsRecompilationRequired = true;
			OnRecompilationRequirementChanged.Broadcast();

			if (GIsEditor && !IsRunningCommandlet())
			{
				GorgeousLogging::ShowToastNotification(
					TEXT("Recompilation Required"),
					TEXT("The installed Gorgeous plugins or systems have changed.\nPlease visit the Gorgeous Library to recompile the project."),
					3 // Error level
				);
			}
		}

		// Record the new state to the persistent data file
		const_cast<UGorgeousPluginHelper*>(PluginHelper)->RecordInstalledSystems(ActiveSystems);
	}

	return bRecompilationNeeded;
}

void UGorgeousRecompilationHelper::TriggerGorgeousRecompilation()
{
	if (EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("Triggering recompilation will prompt you to save your packages and then gracefully exit the Unreal Editor. Do you wish to continue?"))))
	{
		const bool bPromptUserToSave = true;
		const bool bSaveMapPackages = true;
		const bool bSaveContentPackages = true;
		const bool bFastSave = false;
		FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave);

		TSharedPtr<IPlugin> CorePlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
		if (CorePlugin.IsValid())
		{
			const FString InstallerPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
				CorePlugin->GetBaseDir(),
				TEXT("Binaries"),
				FPlatformProcess::GetBinariesSubdirectory(),
#if PLATFORM_WINDOWS
				TEXT("gorgeous-installer.exe")
#else
				TEXT("gorgeous-installer")
#endif
			));

			if (FPaths::FileExists(InstallerPath))
			{
				FString ProjectPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::GetProjectFilePath());
				uint32 PID = FPlatformProcess::GetCurrentProcessId();
				FString Args = FString::Printf(TEXT("--verify-compatibility --reopen-project --project \"%s\" --wait-for-pid %d"), *ProjectPath, PID);

				FPlatformProcess::CreateProc(*InstallerPath, *Args, true, false, false, nullptr, 0, nullptr, nullptr);

				FGenericPlatformMisc::RequestExit(false);
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Could not find Gorgeous Installer executable! Please build it first.")));
			}
		}
	}
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
