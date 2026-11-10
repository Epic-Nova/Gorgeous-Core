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
#ifdef GT_DURATION
#undef GT_DURATION
#endif
#define GT_DURATION 15.0f
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "FileHelpers.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/MessageDialog.h"
#include "ModuleCore/GorgeousUsefulSettings.h"
#include "Interfaces/IPluginManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousExtensionResourceGuardEnforcer Implementation
//=============================================================================

void UGorgeousExtensionResourceGuardEnforcer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (IsRunningCommandlet()) return;
	ShowProtectionBanner();

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
	if (ProtectionBanner.IsValid()) ProtectionBanner->ExpireAndFadeout();
	ProtectionBanner.Reset();
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
	const FString& ResourceName, const FString& SystemDisplayName, bool bIsBlueprintPack)
{
	if (IsRunningCommandlet() || GetDefault<UGorgeousUsefulSettings>()->bDisableExtensionResourceProtection) return;
	const FString Key = FString::Printf(TEXT("%s:%s"), bIsBlueprintPack ? TEXT("BlueprintPack") : TEXT("Plugin"), *ResourceName);
	if (ActiveEnforcements.Contains(Key))
	{
		return; // Already enforcing this plugin
	}

	// Check if any guard that requires this plugin actually has its content present.
	// If no content pack referencing this plugin exists on disk, skip enforcement.
	{
		bool bAnyContentPresent = false;
		for (TObjectIterator<UGorgeousExtensionResourceGuard> It; It; ++It)
		{
			const bool bRequired = bIsBlueprintPack
				? It->RequiredBlueprintPacks.Contains(FName(*ResourceName)) && !It->RequiredBlueprintPacks.FindChecked(FName(*ResourceName))
				: It->RequiredPlugins.Contains(FName(*ResourceName)) && !It->RequiredPlugins.FindChecked(FName(*ResourceName));
			if (bRequired && It->IsContentPresent())
			{
				bAnyContentPresent = true;
				break;
			}
		}

		if (!bAnyContentPresent)
		{
			GT_I_LOG("GT.ExtensionResourceGuard",
				TEXT("Skipping enforcement for required resource '%s' - no content packs requiring it are present."),
				*ResourceName);
			return;
		}
	}

	const double Now = FPlatformTime::Seconds();

	FEnforcementEntry Entry;
	Entry.ResourceName = ResourceName;
	Entry.SystemDisplayName = SystemDisplayName;
	Entry.bIsBlueprintPack = bIsBlueprintPack;
	Entry.StartTimestamp = Now;
	Entry.LastToastTimestamp = Now;
	Entry.bShutdownWarningShown = false;

	ActiveEnforcements.Add(Key, MoveTemp(Entry));
}

void UGorgeousExtensionResourceGuardEnforcer::StopEnforcement(const FString& PluginName)
{
	const int32 Removed = ActiveEnforcements.Remove(FString::Printf(TEXT("Plugin:%s"), *PluginName)) +
		ActiveEnforcements.Remove(FString::Printf(TEXT("BlueprintPack:%s"), *PluginName));
	if (Removed > 0)
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
	ShowProtectionBanner();
	if (GetDefault<UGorgeousUsefulSettings>()->bDisableExtensionResourceProtection)
	{
		ActiveEnforcements.Empty();
		return true;
	}
	if (ActiveEnforcements.IsEmpty())
	{
		return true; // Keep ticking, nothing to do
	}

	const double Now = FPlatformTime::Seconds();
	TArray<FString> ToRemove;

	for (auto& Pair : ActiveEnforcements)
	{
		FEnforcementEntry& Entry = Pair.Value;
		const FString& ResourceName = Entry.ResourceName;
		const FString& SystemDisplayName = Entry.SystemDisplayName;
		const double StartTimestamp = Entry.StartTimestamp;
		double& LastToastTimestamp = Entry.LastToastTimestamp;
		bool& bShutdownWarningShown = Entry.bShutdownWarningShown;
		const double ElapsedSeconds = Now - StartTimestamp;

		bool bResolved = false;
		if (!Entry.bIsBlueprintPack)
		{
			bResolved = IPluginManager::Get().FindEnabledPlugin(ResourceName).IsValid();
		}
		else
		{
			for (TObjectIterator<UGorgeousExtensionResourceGuard> It; It; ++It)
			{
				if (It->IsContentPresent() && It->SystemIdentifier == FName(*ResourceName))
				{
					bResolved = true;
					break;
				}
			}
		}
		if (bResolved)
		{
			ToRemove.Add(Pair.Key);
			GT_S_LOG("GT.ExtensionResourceGuard",
				TEXT("Required resource '%s' is now present. Enforcement lifted."), *ResourceName);
			continue;
		}

		//Check if the content pack was removed while running
		{
			bool bStillNeeded = false;
			for (TObjectIterator<UGorgeousExtensionResourceGuard> It; It; ++It)
			{
				const bool bRequired = Entry.bIsBlueprintPack
					? It->RequiredBlueprintPacks.Contains(FName(*ResourceName)) && !It->RequiredBlueprintPacks.FindChecked(FName(*ResourceName))
					: It->RequiredPlugins.Contains(FName(*ResourceName)) && !It->RequiredPlugins.FindChecked(FName(*ResourceName));
				if (bRequired && It->IsContentPresent())
				{
					bStillNeeded = true;
					break;
				}
			}

			if (!bStillNeeded)
			{
				ToRemove.Add(Pair.Key);
				GT_I_LOG("GT.ExtensionResourceGuard",
					TEXT("Content for required resource '%s' is no longer present. Enforcement lifted."),
					*ResourceName);
				continue;
			}
		}

		// Hard deadline: 10 minutes → save all & shutdown
		if (ElapsedSeconds >= ShutdownDeadlineSeconds)
		{
			GT_E_LOG("GT.ExtensionResourceGuard",
				TEXT("10-minute deadline reached for required resource '%s'. Saving all assets and shutting down the editor."),
				*ResourceName);

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
			ShutdownWarningLink.ActionPayload = ResourceName;
			ShutdownWarningLink.LinkText = FString::Printf(TEXT("Restart now"));

			GT_E_LOG_FULL_EX("GT.ExtensionResourceGuard",
				TEXT("Required resource '%s' is not yet active. "
					"The editor will save all assets and shut down in approximately 2 minutes. "
					"Restart now to apply the changes."),
				GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG,
				false, false, nullptr, Entry.bIsBlueprintPack ? nullptr : &ShutdownWarningLink,
				*ResourceName);
		}

		// Periodic toast every 2 minutes
		if (!bShutdownWarningShown && (Now - LastToastTimestamp) >= ToastIntervalSeconds)
		{
			LastToastTimestamp = Now;

			const int32 MinutesRemaining = FMath::CeilToInt(
				static_cast<float>(ShutdownDeadlineSeconds - ElapsedSeconds) / 60.0f);

			FGorgeousLogHyperlink PeriodicToastLink;
			PeriodicToastLink.ActionName = FName("GT.ExtensionGuard.EnablePlugin");
			PeriodicToastLink.ActionPayload = ResourceName;
			PeriodicToastLink.LinkText = FString::Printf(TEXT("Restart now"));

			GT_W_LOG_FULL_EX("GT.ExtensionResourceGuard",
				TEXT("Required resource '%s' is required by the '%s' system. "
					"Restore or activate the missing resource. ~%d minute(s) remaining before the editor shuts down."),
				GT_DURATION, GT_PRINT_TO_SCREEN, GT_PRINT_TO_LOG,
				false, false, nullptr, Entry.bIsBlueprintPack ? nullptr : &PeriodicToastLink,
				*ResourceName, *SystemDisplayName, MinutesRemaining);
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
#if WITH_DEV_AUTOMATION_TESTS
	const bool bSaved = TestSaveDirtyPackages ? TestSaveDirtyPackages()
		: FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages);
#else
	const bool bSaved = FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages);
#endif
	if (!bSaved)
	{
		GT_E_LOG("GT.ExtensionResourceGuard", TEXT("Saving assets failed. Automatic shutdown cancelled to protect unsaved work."));
		ActiveEnforcements.Empty();
		return;
	}

	GT_W_LOG("GT.ExtensionResourceGuard",
		TEXT("All assets saved. Shutting down editor due to missing required plugins."));

	// Full shutdown - not a restart. The user must resolve the plugin issue before relaunching.
#if WITH_DEV_AUTOMATION_TESTS
	if (TestRequestExit) { TestRequestExit(); return; }
#endif
	FPlatformMisc::RequestExit(/*bForce=*/ false);
}

void UGorgeousExtensionResourceGuardEnforcer::PerformStartupValidation()
{
	if (IsRunningCommandlet() || GetDefault<UGorgeousUsefulSettings>()->bDisableExtensionResourceProtection) return;
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

		for (const TPair<FName, bool>& Requirement : Guard->RequiredPlugins)
		{
			const FName& PluginName = Requirement.Key;
			if (Requirement.Value)
			{
				if (!IPluginManager::Get().FindEnabledPlugin(PluginName.ToString()).IsValid())
				{
					GT_W_LOG("GT.ExtensionResourceGuard.OptionalPlugin",
						TEXT("Optional plugin '%s' is not enabled for system '%s'."),
						*PluginName.ToString(), *Guard->SystemDisplayName.ToString());
				}
				continue;
			}
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

		for (const TPair<FName, bool>& Requirement : Guard->RequiredBlueprintPacks)
		{
			if (Requirement.Key.IsNone()) continue;
			bool bPackPresent = false;
			for (TObjectIterator<UGorgeousExtensionResourceGuard> Other; Other; ++Other)
			{
				if (Other->SystemIdentifier == Requirement.Key && Other->IsContentPresent()) { bPackPresent = true; break; }
			}
			if (bPackPresent) continue;
			if (Requirement.Value)
			{
				GT_W_LOG("GT.ExtensionResourceGuard.OptionalBlueprintPack", TEXT("Optional blueprint pack '%s' is missing for system '%s'."), *Requirement.Key.ToString(), *Guard->SystemDisplayName.ToString());
				continue;
			}
			GT_E_LOG_FULL_EX("GT.ExtensionResourceGuard", TEXT("Blueprint pack '%s' is required by '%s'. Restore that content or remove the dependent content. Restarting alone cannot restore a missing pack."), GT_DURATION, false, true, true, true, nullptr, nullptr, *Requirement.Key.ToString(), *Guard->SystemDisplayName.ToString());
			BeginEnforcement(Requirement.Key.ToString(), Guard->SystemDisplayName.ToString(), true);
			++IssueCount;
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
			TEXT("Plugin '%s' is required by %s. Enable it in the Plugins settings and restart. "
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
void UGorgeousExtensionResourceGuardEnforcer::ShowProtectionBanner()
{
	if (IsRunningCommandlet() || !FSlateApplication::IsInitialized()) return;
	const bool bDisabled = GetDefault<UGorgeousUsefulSettings>()->bDisableExtensionResourceProtection;
	if (ProtectionBanner.IsValid() && bBannerProtectionDisabled == bDisabled) return;
	if (ProtectionBanner.IsValid()) ProtectionBanner->ExpireAndFadeout();
	bBannerProtectionDisabled = bDisabled;
	FNotificationInfo Info(FText::FromString(bDisabled
		? TEXT("Gorgeous resource protection is OFF. Saving Blueprints with missing required content can permanently lose references and damage assets.")
		: TEXT("Gorgeous resource protection is ON. Missing required content can damage Blueprint references when saved. The guard requests shutdown after ten minutes only if saving succeeds.")));
	Info.bFireAndForget = false;
	Info.bUseLargeFont = false;
	Info.ExpireDuration = 0.0f;
	Info.HyperlinkText = FText::FromString(TEXT("Why resource protection matters"));
	Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { GetMutableDefault<UGorgeousUsefulSettings>()->OpenExtensionResourceProtectionDocumentation(); });
	Info.ButtonDetails.Add(FNotificationButtonInfo(
		FText::FromString(bDisabled ? TEXT("Restore Protection") : TEXT("Disable Forced Shutdown")),
		FText::FromString(TEXT("Change the project protection setting. Disabling requires acknowledgement of the risk to asset references.")),
		FSimpleDelegate::CreateWeakLambda(this, [this, bDisabled]()
		{
			if (!bDisabled && FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("Disable resource protection? Saving assets while required systems or content are missing may permanently remove references and damage Blueprints. You can restore protection from this banner or Core Useful Settings."))) != EAppReturnType::Yes) return;
			UGorgeousUsefulSettings* Settings = GetMutableDefault<UGorgeousUsefulSettings>();
			Settings->bDisableExtensionResourceProtection = !bDisabled;
			Settings->SaveConfig();
			if (ProtectionBanner.IsValid()) ProtectionBanner->ExpireAndFadeout();
			ProtectionBanner.Reset();
			ShowProtectionBanner();
			if (bDisabled) PerformStartupValidation();
		}), SNotificationItem::CS_None));
	ProtectionBanner = FSlateNotificationManager::Get().AddNotification(Info);
}
