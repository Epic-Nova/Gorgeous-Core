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
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetRegistry/AssetRegistryModule.h"
//<--------------------------=== Module Includes ===------------------------->
#include "AssetRegistry/AssetData.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousExtensionResourceGuard Implementation
//=============================================================================

bool UGorgeousExtensionResourceGuard::IsContentPresent() const
{
	// Content-less guards are always considered active.
	if (!bIsContentPackGuard)
	{
		return true;
	}

	if (OwningPluginName.IsNone() || ContentSubPath.IsEmpty())
	{
		return true;
	}

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(OwningPluginName.ToString());
	if (!Plugin.IsValid())
	{
		return false;
	}

	const FString FullContentPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Content"), ContentSubPath);
	return FPaths::DirectoryExists(FullContentPath);
}

bool UGorgeousExtensionResourceGuard::AddPluginDependencyToDescriptor(
	const FName& OwningPlugin, const FString& PluginName)
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(OwningPlugin.ToString());
	if (!Plugin.IsValid())
	{
		GT_W_LOG("GT.ExtensionResourceGuard",
			TEXT("Cannot add dependency: owning plugin '%s' not found."), *OwningPlugin.ToString());
		return false;
	}

	const FString& DescriptorPath = Plugin->GetDescriptorFileName();

	FPluginDescriptor Descriptor;
	FText FailReason;
	if (!Descriptor.Load(DescriptorPath, FailReason))
	{
		GT_E_LOG("GT.ExtensionResourceGuard",
			TEXT("Failed to load plugin descriptor '%s': %s"), *DescriptorPath, *FailReason.ToString());
		return false;
	}

	// Check if the dependency already exists
	for (FPluginReferenceDescriptor& Ref : Descriptor.Plugins)
	{
		if (Ref.Name == PluginName)
		{
			if (Ref.bEnabled)
			{
				return true; // Already present and enabled - no-op
			}
			Ref.bEnabled = true;

			if (!Descriptor.UpdatePluginFile(DescriptorPath, FailReason))
			{
				GT_E_LOG("GT.ExtensionResourceGuard",
					TEXT("Failed to update plugin descriptor '%s': %s"), *DescriptorPath, *FailReason.ToString());
				return false;
			}

			GT_S_LOG("GT.ExtensionResourceGuard",
				TEXT("Enabled existing dependency '%s' in plugin descriptor '%s'."),
				*PluginName, *OwningPlugin.ToString());
			return true;
		}
	}

	FPluginReferenceDescriptor NewRef;
	NewRef.Name = PluginName;
	NewRef.bEnabled = true;
	Descriptor.Plugins.Add(NewRef);

	if (!Descriptor.UpdatePluginFile(DescriptorPath, FailReason))
	{
		GT_E_LOG("GT.ExtensionResourceGuard",
			TEXT("Failed to update plugin descriptor '%s': %s"), *DescriptorPath, *FailReason.ToString());
		return false;
	}

	return true;
}

bool UGorgeousExtensionResourceGuard::RemovePluginDependencyFromDescriptor(
	const FName& OwningPlugin, const FString& PluginName)
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(OwningPlugin.ToString());
	if (!Plugin.IsValid())
	{
		return false;
	}

	const FString& DescriptorPath = Plugin->GetDescriptorFileName();

	FPluginDescriptor Descriptor;
	FText FailReason;
	if (!Descriptor.Load(DescriptorPath, FailReason))
	{
		GT_E_LOG("GT.ExtensionResourceGuard",
			TEXT("Failed to load plugin descriptor '%s': %s"), *DescriptorPath, *FailReason.ToString());
		return false;
	}

	const int32 RemovedCount = Descriptor.Plugins.RemoveAll(
		[&PluginName](const FPluginReferenceDescriptor& Ref)
		{
			return Ref.Name == PluginName;
		});

	if (RemovedCount == 0)
	{
		return true; // Nothing to remove - no-op
	}

	if (!Descriptor.UpdatePluginFile(DescriptorPath, FailReason))
	{
		GT_E_LOG("GT.ExtensionResourceGuard",
			TEXT("Failed to update plugin descriptor '%s': %s"), *DescriptorPath, *FailReason.ToString());
		return false;
	}

	GT_I_LOG("GT.ExtensionResourceGuard",
		TEXT("Removed stale dependency '%s' from plugin descriptor '%s'."),
		*PluginName, *OwningPlugin.ToString());
	return true;
}

void UGorgeousExtensionResourceGuard::ReconcilePluginDependencies()
{
	// At module startup, data assets may not be in memory yet.
	// Use the AssetRegistry to discover and synchronously load all guards.
	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry")).Get();

	TArray<FAssetData> AllGuardAssets;
	AssetRegistry.GetAssetsByClass(
		UGorgeousExtensionResourceGuard::StaticClass()->GetClassPathName(),
		AllGuardAssets,
		/*bSearchSubClasses=*/ true);

	// Pass 1 - collect all plugins that are still actively required by guards
	// whose content IS present, keyed by owning plugin.
	// Key: OwningPluginName → Map of RequiredPlugin → SystemDisplayName(s) that requested it.
	TMap<FName, TMap<FName, TArray<FString>>> ActiveDependencies;

	// Also collect candidates for removal (guards whose content is gone).
	struct FStaleCandidate
	{
		const UGorgeousExtensionResourceGuard* Guard;
	};
	TArray<FStaleCandidate> StaleCandidates;

	for (const FAssetData& AssetData : AllGuardAssets)
	{
		const UGorgeousExtensionResourceGuard* Guard =
			Cast<UGorgeousExtensionResourceGuard>(AssetData.GetAsset());
		if (!Guard)
		{
			continue;
		}

		if (Guard->OwningPluginName.IsNone() || Guard->RequiredPlugins.IsEmpty())
		{
			continue;
		}

		if (Guard->IsContentPresent())
		{
			// Content exists - record all its required plugins as still-needed.
			TMap<FName, TArray<FString>>& ActiveMap = ActiveDependencies.FindOrAdd(Guard->OwningPluginName);
			const FString GuardSystem = Guard->SystemDisplayName.ToString();
			for (const FName& RequiredPlugin : Guard->RequiredPlugins)
			{
				ActiveMap.FindOrAdd(RequiredPlugin).AddUnique(GuardSystem);
			}
		}
		else
		{
			GT_W_LOG("GT.ExtensionResourceGuard",
				TEXT("Guard '%s' (system '%s') found but its content is not present on disk - marking as stale."),
				*AssetData.AssetName.ToString(), *Guard->SystemIdentifier.ToString());
			StaleCandidates.Add({ Guard });
		}
	}

	// Pass 2 - for each stale guard, only remove dependencies that are NOT
	// still claimed by another guard with present content on the same owning plugin.
	for (const FStaleCandidate& Candidate : StaleCandidates)
	{
		const UGorgeousExtensionResourceGuard* Guard = Candidate.Guard;
		const TMap<FName, TArray<FString>>* ActiveMap = ActiveDependencies.Find(Guard->OwningPluginName);

		bool bRemovedAny = false;
		for (const FName& RequiredPlugin : Guard->RequiredPlugins)
		{
			if (ActiveMap && ActiveMap->Contains(RequiredPlugin))
			{
				if (!IPluginManager::Get().FindPlugin(Guard->OwningPluginName.ToString())->GetDescriptor().Plugins.ContainsByPredicate(
					[&RequiredPlugin](const FPluginReferenceDescriptor& Ref) { return Ref.Name == RequiredPlugin.ToString(); }))
				{
					// This is a rare edge case: the guard's content is gone, but another guard on the same plugin still needs this dependency, which is now missing from the descriptor. The safe move is to leave it in place to avoid breaking the other guard, even though it will be reported as an error until fixed.
					continue;
				}
				
				// Another active guard on the same owning plugin still needs this dependency - skip.
				GT_I_LOG("GT.ExtensionResourceGuard",
					TEXT("Keeping dependency '%s' in plugin '%s' - still required by another active guard."),
					*RequiredPlugin.ToString(), *Guard->OwningPluginName.ToString());
				continue;
			}

			RemovePluginDependencyFromDescriptor(Guard->OwningPluginName, RequiredPlugin.ToString());
			bRemovedAny = true;
		}

		if (bRemovedAny)
		{
			GT_I_LOG("GT.ExtensionResourceGuard",
				TEXT("Swept stale dependencies for removed system '%s' from plugin '%s'."),
				*Guard->SystemIdentifier.ToString(), *Guard->OwningPluginName.ToString());
		}
	}

	// Pass 3 - for each active guard, ensure all required plugins are present
	// in the owning plugin's .uplugin descriptor. If any are missing, add them.
	for (const auto& Pair : ActiveDependencies)
	{
		const FName& OwningPlugin = Pair.Key;
		const TMap<FName, TArray<FString>>& RequiredMap = Pair.Value;

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(OwningPlugin.ToString());
		if (!Plugin.IsValid())
		{
			continue;
		}

		const TArray<FPluginReferenceDescriptor>& DescriptorPlugins = Plugin->GetDescriptor().Plugins;

		for (const auto& DepPair : RequiredMap)
		{
			const FName& RequiredPlugin = DepPair.Key;
			const TArray<FString>& RequestingSystems = DepPair.Value;
			const FString RequiredPluginStr = RequiredPlugin.ToString();

			const bool bAlreadyInDescriptor = DescriptorPlugins.ContainsByPredicate(
				[&RequiredPluginStr](const FPluginReferenceDescriptor& Ref)
				{
					return Ref.Name == RequiredPluginStr;
				});

			if (!bAlreadyInDescriptor)
			{
				const FString SystemList = FString::Join(RequestingSystems, TEXT(", "));
				GT_I_LOG("GT.ExtensionResourceGuard",
					TEXT("Adding missing dependency '%s' to plugin '%s' (requested by: %s)."),
					*RequiredPluginStr, *OwningPlugin.ToString(), *SystemList);
				AddPluginDependencyToDescriptor(OwningPlugin, RequiredPluginStr);
			}
		}
	}
}
