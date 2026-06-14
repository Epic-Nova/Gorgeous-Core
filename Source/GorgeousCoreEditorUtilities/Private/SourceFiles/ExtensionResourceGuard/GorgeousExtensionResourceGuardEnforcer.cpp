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
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuardEnforcer.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#define GT_DURATION 15.0f
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "FileHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousExtensionResourceGuardEnforcer Implementation
//=============================================================================

void UGorgeousExtensionResourceGuardEnforcer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Ticker runs every 10 seconds - lightweight check.
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ThisClass::OnTick),
		10.0f);

	// Proactively check all known guards on startup.
	// Defer until the Asset Registry has finished its initial scan -
	// at Initialize() time the content scan is still in progress.
	IAssetRegistry& AssetRegistryRef = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry")).Get();

	if (AssetRegistryRef.IsLoadingAssets())
	{
		AssetRegistryRef.OnFilesLoaded().AddUObject(this, &ThisClass::PerformStartupValidation);
	}
	else
	{
		PerformStartupValidation();
	}
}

void UGorgeousExtensionResourceGuardEnforcer::Deinitialize()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	// Remove the Asset Registry callback if it hasn't fired yet.
	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry")))
	{
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
			TEXT("AssetRegistry")).Get();
		AssetRegistry.OnFilesLoaded().RemoveAll(this);
	}

	ActiveEnforcements.Empty();

	Super::Deinitialize();
}

void UGorgeousExtensionResourceGuardEnforcer::BeginEnforcement(
	const FString& PluginName, const FString& SystemDisplayName)
{
	if (ActiveEnforcements.Contains(PluginName))
	{
		return; // Already enforcing this plugin
	}

	// Check if any guard that requires this plugin actually has its content present.
	// If no content pack referencing this plugin exists on disk, skip enforcement.
	{
		bool bAnyContentPresent = false;
		for (TObjectIterator<UGorgeousExtensionResourceGuard> It; It; ++It)
		{
			if (It->RequiredPlugins.Contains(FName(*PluginName)) && It->IsContentPresent())
			{
				bAnyContentPresent = true;
				break;
			}
		}

		if (!bAnyContentPresent)
		{
			GT_I_LOG("GT.ExtensionResourceGuard",
				TEXT("Skipping enforcement for plugin '%s' - no content packs requiring it are present."),
				*PluginName);
			return;
		}
	}

	const double Now = FPlatformTime::Seconds();

	FEnforcementEntry Entry;
	Entry.PluginName = PluginName;
	Entry.SystemDisplayName = SystemDisplayName;
	Entry.StartTimestamp = Now;
	Entry.LastToastTimestamp = Now;
	Entry.bShutdownWarningShown = false;

	ActiveEnforcements.Add(PluginName, MoveTemp(Entry));
}

void UGorgeousExtensionResourceGuardEnforcer::StopEnforcement(const FString& PluginName)
{
	if (ActiveEnforcements.Remove(PluginName) > 0)
	{
		GT_S_LOG("GT.ExtensionResourceGuard",
			TEXT("Enforcement stopped for plugin '%s'."), *PluginName);
	}
}

bool UGorgeousExtensionResourceGuardEnforcer::IsEnforcementActive() const
{
	return ActiveEnforcements.Num() > 0;
}

bool UGorgeousExtensionResourceGuardEnforcer::OnTick(float DeltaTime)
{
	if (ActiveEnforcements.IsEmpty())
	{
		return true; // Keep ticking, nothing to do
	}

	const double Now = FPlatformTime::Seconds();
	TArray<FString> ToRemove;

	for (auto& Pair : ActiveEnforcements)
	{
		auto& [PluginName, SystemDisplayName, StartTimestamp, LastToastTimestamp, bShutdownWarningShown] = Pair.Value;
		const double ElapsedSeconds = Now - StartTimestamp;

		//Check if the plugin was enabled externally
		if (IPluginManager::Get().FindEnabledPlugin(PluginName).IsValid())
		{
			ToRemove.Add(Pair.Key);
			GT_S_LOG("GT.ExtensionResourceGuard",
				TEXT("Plugin '%s' is now enabled. Enforcement lifted."), *PluginName);
			continue;
		}

		//Check if the content pack was removed while running
		{
			bool bStillNeeded = false;
			for (TObjectIterator<UGorgeousExtensionResourceGuard> It; It; ++It)
			{
				if (It->RequiredPlugins.Contains(FName(*PluginName)) && It->IsContentPresent())
				{
					bStillNeeded = true;
					break;
				}
			}

			if (!bStillNeeded)
			{
				ToRemove.Add(Pair.Key);
				GT_I_LOG("GT.ExtensionResourceGuard",
					TEXT("Content for plugin '%s' is no longer present. Enforcement lifted."),
					*PluginName);
				continue;
			}
		}

		// Hard deadline: 10 minutes → save all & shutdown
		if (ElapsedSeconds >= ShutdownDeadlineSeconds)
		{
			GT_E_LOG("GT.ExtensionResourceGuard",
				TEXT("10-minute deadline reached for plugin '%s'. Saving all assets and shutting down the editor."),
				*PluginName);

			SafeShutdownEditor();
			return true; // Won't actually reach here after shutdown
		}

		// Shutdown warning at 8 minutes
		if (!bShutdownWarningShown && ElapsedSeconds >= (ShutdownDeadlineSeconds - 120.0))
		{
			bShutdownWarningShown = true;

			// Show as toast and log via Gorgeous system
			FGorgeousLogHyperlink ShutdownWarningLink;
			ShutdownWarningLink.ActionName = FName("GT.ExtensionGuard.EnablePlugin");
			ShutdownWarningLink.ActionPayload = PluginName;
			ShutdownWarningLink.LinkText = FString::Printf(TEXT("Restart now"));

			GT_E_LOG_FULL_EX("GT.ExtensionResourceGuard",
				TEXT("Plugin '%s' has been automatically added as a dependency but is not yet active. "
					"The editor will save all assets and shut down in approximately 2 minutes. "
					"Restart now to apply the changes."),
				GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG,
				false, false, nullptr, &ShutdownWarningLink,
				*PluginName);
		}

		// Periodic toast every 2 minutes
		if (!bShutdownWarningShown && (Now - LastToastTimestamp) >= ToastIntervalSeconds)
		{
			LastToastTimestamp = Now;

			const int32 MinutesRemaining = FMath::CeilToInt(
				static_cast<float>(ShutdownDeadlineSeconds - ElapsedSeconds) / 60.0f);

			FGorgeousLogHyperlink PeriodicToastLink;
			PeriodicToastLink.ActionName = FName("GT.ExtensionGuard.EnablePlugin");
			PeriodicToastLink.ActionPayload = PluginName;
			PeriodicToastLink.LinkText = FString::Printf(TEXT("Restart now"));

			GT_W_LOG_FULL_EX("GT.ExtensionResourceGuard",
				TEXT("Plugin '%s' was automatically added as a dependency for the '%s' system. "
					"A restart is required to activate it. ~%d minute(s) remaining before the editor shuts down."),
				GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG,
				false, false, nullptr, &PeriodicToastLink,
				*PluginName, *SystemDisplayName, MinutesRemaining);
		}
	}

	// Clean up resolved entries
	for (const FString& Key : ToRemove)
	{
		ActiveEnforcements.Remove(Key);
	}

	return true; // Keep ticking
}

void UGorgeousExtensionResourceGuardEnforcer::SafeShutdownEditor()
{
	GT_W_LOG("GT.ExtensionResourceGuard",
		TEXT("Extension Resource Guard: Saving all assets before editor shutdown..."));

	// Save all dirty packages - this ensures no user data is lost.
	constexpr bool bPromptUserToSave = false;
	constexpr bool bSaveMapPackages = true;
	constexpr bool bSaveContentPackages = true;
	FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages);

	GT_W_LOG("GT.ExtensionResourceGuard",
		TEXT("All assets saved. Shutting down editor due to missing required plugins."));

	// Full shutdown - not a restart. The user must resolve the plugin issue before relaunching.
	FPlatformMisc::RequestExit(/*bForce=*/ false);
}

void UGorgeousExtensionResourceGuardEnforcer::PerformStartupValidation()
{
	// Discover all Extension Resource Guard data assets (native C++ subclasses
	// AND Blueprint-derived guards) via the Asset Registry.
	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry")).Get();

	TArray<FAssetData> AllGuardAssets;
	AssetRegistry.GetAssetsByClass(
		UGorgeousExtensionResourceGuard::StaticClass()->GetClassPathName(),
		AllGuardAssets,
		/*bSearchSubClasses=*/ true);

	if (AllGuardAssets.IsEmpty())
	{
		return;
	}

	// Perform the checks directly rather than going through
	// UEditorValidatorSubsystem - at startup the DataValidation pipeline
	// may not be fully initialized yet (Blueprint validators not registered),
	// which causes spurious warnings that get attributed as asset errors.
	int32 IssueCount = 0;

	// Collect which systems are affected per missing plugin so we can
	// show a single consolidated toast per plugin.
	TMap<FString, TArray<FString>> PluginToSystems;

	for (const FAssetData& AssetData : AllGuardAssets)
	{
		const UGorgeousExtensionResourceGuard* Guard =
			Cast<UGorgeousExtensionResourceGuard>(AssetData.GetAsset());
		if (!Guard)
		{
			continue;
		}

		// Skip guards whose content pack is not present on disk.
		if (!Guard->IsContentPresent())
		{
			continue;
		}

		for (const FName& PluginName : Guard->RequiredPlugins)
		{
			if (PluginName.IsNone())
			{
				continue;
			}

			const bool bPluginIsEnabled = IPluginManager::Get()
				.FindEnabledPlugin(PluginName.ToString()).IsValid();

			if (!bPluginIsEnabled)
			{
				++IssueCount;

				TArray<FString>& Systems = PluginToSystems.FindOrAdd(PluginName.ToString());
				Systems.AddUnique(Guard->SystemDisplayName.ToString());

				BeginEnforcement(PluginName.ToString(), Guard->SystemDisplayName.ToString());
			}
		}
	}

	// Show one toast per missing plugin, listing all affected systems.
	for (const auto& Pair : PluginToSystems)
	{
		const FString& PluginName = Pair.Key;
		const TArray<FString>& Systems = Pair.Value;

		const FString SystemList = FString::Join(Systems, TEXT(", "));

		FGorgeousLogHyperlink EnableLink;
		EnableLink.ActionName = FName("GT.ExtensionGuard.EnablePlugin");
		EnableLink.ActionPayload = PluginName;
		EnableLink.LinkText = FString::Printf(TEXT("Restart now"));

		GT_W_LOG_FULL_EX("GT.ExtensionResourceGuard",
			TEXT("Plugin '%s' is required by %s and has been automatically added as a dependency. "
				"A restart is needed to activate it."),
			GT_DURATION, false, GT_PRINT_TO_LOG,
			false, false, nullptr, &EnableLink,
			*PluginName, *SystemList);
	}

	if (IssueCount > 0)
	{
		GT_W_LOG("GT.ExtensionResourceGuard",
			TEXT("Startup validation found %d Extension Resource Guard issue(s)."),
			IssueCount);
	}
}