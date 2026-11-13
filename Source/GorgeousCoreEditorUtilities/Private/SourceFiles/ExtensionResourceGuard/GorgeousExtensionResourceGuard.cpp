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
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetRegistry/AssetRegistryModule.h"
//<--------------------------=== Module Includes ===------------------------->
#include "Editor.h"
#include "ModuleCore/GorgeousUsefulSettings.h"
#include "EditorUtilityObject.h"
#include "AssetRegistry/AssetData.h"
#include "Editor/Blutility/Classes/EditorUtilityBlueprint.h"
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
	if (IsRunningCommandlet() || GetDefault<UGorgeousUsefulSettings>()->bDisableExtensionResourceProtection ||
		!GetDefault<UGorgeousUsefulSettings>()->bAllowAutomaticPluginDescriptorChanges) return;
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

		if (Guard->OwningPluginName.IsNone() ||
			Guard->RequiredPlugins.IsEmpty())
		{
			continue;
		}

		if (Guard->IsContentPresent())
		{
			// Content exists - record all its required plugins as still-needed.
			TMap<FName, TArray<FString>>& ActiveMap = ActiveDependencies.FindOrAdd(Guard->OwningPluginName);
			const FString GuardSystem = Guard->SystemDisplayName.ToString();
			for (const TPair<FName, bool>& Requirement : Guard->RequiredPlugins)
			{
				if (!Requirement.Value)
				{
					ActiveMap.FindOrAdd(Requirement.Key).AddUnique(GuardSystem);
				}
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

	// Dependencies may also be required by native code; removing content never removes descriptor dependencies.
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
					return Ref.Name == RequiredPluginStr && Ref.bEnabled;
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
	
	// Pass 4 - asynchronously loads & executes any editor utility blueprints for active guards that specified them.
	/*for (const FAssetData& AssetData : AllGuardAssets)
	{
		const UGorgeousExtensionResourceGuard* Guard =
			Cast<UGorgeousExtensionResourceGuard>(AssetData.GetAsset());
		if (!Guard || !Guard->IsContentPresent() || Guard->UtilityBlueprintsToExecute.IsEmpty())
		{
			continue;
		}
		
		for (const TSoftClassPtr<UEditorUtilityObject>& SoftUtilityBlueprint : Guard->UtilityBlueprintsToExecute)
		{
			if (!SoftUtilityBlueprint.IsValid())
			{
				continue;
			}
			
			UEditorUtilityObject* UtilityObject = NewObject<UEditorUtilityObject>(GetTransientPackage(), SoftUtilityBlueprint.Get(), NAME_None, RF_Transient);
			
			// Search within this new created class for a variable called EditorWorldContext, if found then set this ptr value to the world editor context
			
			for (TFieldIterator<FObjectProperty> PropIt(UtilityObject->GetClass()); PropIt; ++PropIt)	
			{
				if (FObjectProperty* ObjectProp = *PropIt; ObjectProp->GetName() == TEXT("EditorWorldContext"))
				{
					UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
					ObjectProp->SetObjectPropertyValue_InContainer(UtilityObject, EditorWorld);
					break;
				}
			}
			
			UtilityObject->Run();
		}
	}	*/
}

TArray<FString> UGorgeousExtensionResourceGuard::GetPluginOptions() const
{
	TArray<FString> Options;
	TArray<TSharedRef<IPlugin>> DiscoveredPlugins = IPluginManager::Get().GetDiscoveredPlugins();
	for (const TSharedRef<IPlugin>& Plugin : DiscoveredPlugins)
	{
		Options.Add(Plugin->GetName());
	}
	Options.Sort();
	return Options;
}
