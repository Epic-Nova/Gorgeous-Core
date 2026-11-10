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
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystem_AC.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "PluginDescriptor.h"
#include "ModuleDescriptor.h"
#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#endif
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/AssetManager.h"
#include "GeneralSystems/GorgeousGeneralSystemAction.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuardRuntimeInterface.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// Helpers
//=============================================================================
namespace
{
const IGorgeousExtensionResourceGuardRuntimeInterface* FindGuardForSystem(const FName SystemName)
{
#if !WITH_EDITOR
    return nullptr;
#else
    FString SystemIdentifier = SystemName.ToString();
    if (SystemIdentifier.StartsWith(TEXT("PDA_")))
    {
        SystemIdentifier.RightChopInline(4);
    }

    if (SystemIdentifier.IsEmpty())
    {
        return nullptr;
    }

    IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    TArray<FAssetData> Assets;
    AssetRegistry.GetAllAssets(Assets, true);

    const FTopLevelAssetPath GuardBase(TEXT("/Script/GorgeousCoreEditorUtilities"), TEXT("GorgeousExtensionResourceGuard"));
    TArray<FTopLevelAssetPath> Bases = { GuardBase };
    TSet<FTopLevelAssetPath> DerivedGuardClasses;
    AssetRegistry.GetDerivedClassNames(Bases, TSet<FTopLevelAssetPath>(), DerivedGuardClasses);
    for (const FAssetData& Asset : Assets)
    {
        if (Asset.AssetClassPath != GuardBase && !DerivedGuardClasses.Contains(Asset.AssetClassPath))
        {
            continue;
        }

        UObject* GuardObject = Asset.GetAsset();
        if (GuardObject && GuardObject->GetClass()->ImplementsInterface(UGorgeousExtensionResourceGuardRuntimeInterface::StaticClass()))
        {
            const IGorgeousExtensionResourceGuardRuntimeInterface* Guard =
                Cast<IGorgeousExtensionResourceGuardRuntimeInterface>(GuardObject);
            if (Guard && Guard->GetGuardSystemIdentifier() == FName(*SystemIdentifier) && Guard->IsGuardContentPresent())
            {
                return Guard;
            }
        }
    }

    return nullptr;
#endif
}

bool AreGuardRequirementsAvailable(const TMap<FName, bool>& Plugins, const TMap<FName, bool>& Systems)
{
    for (const TPair<FName, bool>& RequiredPlugin : Plugins)
    {
        if (RequiredPlugin.Value)
        {
            continue;
        }

        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(RequiredPlugin.Key.ToString());
        if (!Plugin.IsValid() || !Plugin->IsEnabled())
        {
            return false;
        }
    }

    for (const TPair<FName, bool>& RequiredSystem : Systems)
    {
        if (!RequiredSystem.Value && !UGeneralSystemConfiguration_PDA::IsSystemPresent(RequiredSystem.Key))
        {
            return false;
        }
    }

    return true;
}
}

//=============================================================================
// UGeneralSystemConfiguration_PDA Implementation
//=============================================================================
TArray<UGeneralSystemConfiguration_PDA*> UGeneralSystemConfiguration_PDA::GetAllSystemConfigurations()
{
    TArray<UGeneralSystemConfiguration_PDA*> Results;

    TArray<FAssetData> AssetDataList;

    if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
    {
        AssetManager->GetPrimaryAssetDataList(TEXT("GorgeousBlueprintSystems"), AssetDataList);
    }

    if (AssetDataList.IsEmpty())
    {
        IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
        AssetRegistry.GetAssetsByClass(UGeneralSystemConfiguration_PDA::StaticClass()->GetClassPathName(), AssetDataList, true);
    }

    for (const FAssetData& Data : AssetDataList)
    {
        if (UGeneralSystemConfiguration_PDA* PDA = Cast<UGeneralSystemConfiguration_PDA>(Data.GetAsset()))
        {
            Results.AddUnique(PDA);
        }
        else if (UGeneralSystemConfiguration_PDA* LoadedPDA = LoadObject<UGeneralSystemConfiguration_PDA>(nullptr, *Data.GetSoftObjectPath().ToString()))
        {
            Results.AddUnique(LoadedPDA);
        }
    }

    return Results;
}

TArray<FName> UGeneralSystemConfiguration_PDA::GetAllSystemNames()
{
    TArray<FName> Results;

    for (const UGeneralSystemConfiguration_PDA* SystemConfiguration : GetAllSystemConfigurations())
    {
        if (IsValid(SystemConfiguration))
        {
            const FName SystemName = SystemConfiguration->GetPDAAssetName();
            if (!SystemName.IsNone())
            {
                Results.AddUnique(SystemName);
            }
        }
    }

    return Results;
}

bool UGeneralSystemConfiguration_PDA::IsSystemPresent(const FName SystemName)
{
    if (SystemName.IsNone())
    {
        return false;
    }

    for (const UGeneralSystemConfiguration_PDA* SystemConfiguration : GetAllSystemConfigurations())
    {
        if (!IsValid(SystemConfiguration))
            continue;

        // First check the obvious system name by grabbing the PDA filename. If no match occurred, then try to match the system name to one of the synonyms present in the component tags.
        FString NormalizedSystemName = SystemName.ToString();
        NormalizedSystemName.RemoveFromStart(TEXT("PDA_"));
        const FName ExpectedSystemAssetName(*FString::Printf(TEXT("PDA_%s"), *NormalizedSystemName));
        const FName ExpectedComponentTag(*FString::Printf(TEXT("%s_System"), *NormalizedSystemName));
        const UGeneralSystem_AC* AuthoritativeComponent = SystemConfiguration->AuthoritativeComponent
            ? SystemConfiguration->AuthoritativeComponent->GetDefaultObject<UGeneralSystem_AC>()
            : nullptr;

        if (SystemConfiguration->GetPDAAssetName() == ExpectedSystemAssetName ||
            (AuthoritativeComponent && AuthoritativeComponent->ComponentTags.Contains(ExpectedComponentTag)))
        {
            return true;
        }
    }

    return false;
}

bool UGeneralSystemConfiguration_PDA::ActivateSystem()
{
    TSet<const UGeneralSystemConfiguration_PDA*> ActivationStack;
    return ActivateSystemInternal(ActivationStack);
}

bool UGeneralSystemConfiguration_PDA::ActivateSystemInternal(TSet<const UGeneralSystemConfiguration_PDA*>& ActivationStack)
{
    if (bIsSystemActive)
    {
        return true;
    }

    if (ActivationStack.Contains(this))
    {
        GT_E_LOG("GT.GeneralSystemConfiguration", TEXT("Circular General System dependency detected while activating '%s'."), *GetName());
        return false;
    }

    ActivationStack.Add(this);
    const IGorgeousExtensionResourceGuardRuntimeInterface* Guard = FindGuardForSystem(GetPDAAssetName());
    const TMap<FName, bool>& RequiredPlugins = Guard ? Guard->GetGuardRequiredPlugins() : RuntimeRequiredPlugins;
    const TMap<FName, bool>& RequiredSystems = Guard ? Guard->GetGuardRequiredBlueprintPacks() : RuntimeRequiredSystems;
    {
        for (const TPair<FName, bool>& RequiredSystem : RequiredSystems)
        {
            if (RequiredSystem.Value)
            {
                continue;
            }

            FString DependencyName = RequiredSystem.Key.ToString();
            DependencyName.RemoveFromStart(TEXT("PDA_"));
            const FName RequiredPDAName(*FString::Printf(TEXT("PDA_%s"), *DependencyName));
            UGeneralSystemConfiguration_PDA* Dependency = nullptr;
            for (UGeneralSystemConfiguration_PDA* Candidate : GetAllSystemConfigurations())
            {
                if (Candidate && Candidate->GetPDAAssetName() == RequiredPDAName)
                {
                    Dependency = Candidate;
                    break;
                }
            }

            if (!Dependency || !Dependency->ActivateSystemInternal(ActivationStack))
            {
                GT_E_LOG("GT.GeneralSystemConfiguration",
                    TEXT("General system '%s' has an unresolved required system '%s'."),
                    *GetName(), *RequiredSystem.Key.ToString());
                ActivationStack.Remove(this);
                return false;
            }
        }
    }

    if (!AreGuardRequirementsAvailable(RequiredPlugins, RequiredSystems))
    {
        GT_E_LOG("GT.GeneralSystemConfiguration",
            TEXT("General system '%s' has unavailable extension resource guard requirements."), *GetName());
        ActivationStack.Remove(this);
        return false;
    }

    ActivationStack.Remove(this);
    for (UGorgeousGeneralSystemAction* Action : Actions)
    {
        if (IsValid(Action))
        {
            if (Action->CanExecuteInCurrentSession() && Action->PredictAction(this))
            {
                Action->OnSystemActivated(this);
                ActivatedActions.Add(Action);
            }
            else
            {
                GT_I_LOG("GT.GeneralSystemConfiguration",
                    TEXT("Skipped action '%s' for general system '%s' because an action-level required system is unavailable."),
                    *GetNameSafe(Action), *GetName());
            }
        }
    }

    bIsSystemActive = true;
    return true;
}

void UGeneralSystemConfiguration_PDA::DeactivateSystem()
{
    if (!bIsSystemActive)
    {
        return;
    }

    for (int32 ActionIndex = Actions.Num() - 1; ActionIndex >= 0; --ActionIndex)
    {
        if (UGorgeousGeneralSystemAction* Action = Actions[ActionIndex]; ActivatedActions.Contains(Action))
        {
            Action->OnSystemDeactivated(this);
        }
    }

    ActivatedActions.Reset();
    bIsSystemActive = false;
}

void UGeneralSystemConfiguration_PDA::ActivateAllSystemConfigurations()
{
    const TArray<UGeneralSystemConfiguration_PDA*> SystemConfigurations = GetAllSystemConfigurations();
    GT_I_LOG("GT.GeneralSystemConfiguration",
        TEXT("Activating %d Gorgeous general system configuration(s)."), SystemConfigurations.Num());

    for (UGeneralSystemConfiguration_PDA* SystemConfiguration : SystemConfigurations)
    {
        if (IsValid(SystemConfiguration))
        {
            SystemConfiguration->ActivateSystem();
        }
    }
}
#if WITH_EDITOR
bool UGeneralSystemConfiguration_PDA::RefreshRuntimeRequirements()
{
    const IGorgeousExtensionResourceGuardRuntimeInterface* Guard = FindGuardForSystem(GetPDAAssetName());
    if (!Guard) return false;
    TMap<FName, bool> CookedPlugins;
    for (const TPair<FName, bool>& Requirement : Guard->GetGuardRequiredPlugins())
    {
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(Requirement.Key.ToString());
        // Editor-only helpers such as DataValidation must not become packaged-game requirements.
        if (Plugin.IsValid())
        {
            const TArray<FModuleDescriptor>& Modules = Plugin->GetDescriptor().Modules;
            const bool bRuntimeCapable = Modules.IsEmpty() || Modules.ContainsByPredicate([](const FModuleDescriptor& Module)
            {
                return Module.Type == EHostType::Runtime || Module.Type == EHostType::RuntimeNoCommandlet ||
                    Module.Type == EHostType::RuntimeAndProgram || Module.Type == EHostType::CookedOnly ||
                    Module.Type == EHostType::ClientOnly || Module.Type == EHostType::ClientOnlyNoCommandlet || Module.Type == EHostType::ServerOnly;
            });
            if (!bRuntimeCapable) continue;
        }
        CookedPlugins.Add(Requirement.Key, Requirement.Value);
    }
    const TMap<FName, bool>& CookedSystems = Guard->GetGuardRequiredBlueprintPacks();
    if (RuntimeRequiredPlugins.OrderIndependentCompareEqual(CookedPlugins) && RuntimeRequiredSystems.OrderIndependentCompareEqual(CookedSystems)) return false;
    RuntimeRequiredPlugins = MoveTemp(CookedPlugins);
    RuntimeRequiredSystems = CookedSystems;
    return true;
}

void UGeneralSystemConfiguration_PDA::PreSave(FObjectPreSaveContext SaveContext)
{
    RefreshRuntimeRequirements();
    Super::PreSave(SaveContext);
}
#endif
