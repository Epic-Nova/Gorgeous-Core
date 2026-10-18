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

    for (const FAssetData& Asset : Assets)
    {
        if (Asset.AssetClassPath.GetAssetName() != TEXT("GorgeousExtensionResourceGuard"))
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
}

bool AreGuardRequirementsAvailable(const IGorgeousExtensionResourceGuardRuntimeInterface& Guard)
{
    for (const TPair<FName, bool>& RequiredPlugin : Guard.GetGuardRequiredPlugins())
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

    for (const TPair<FName, bool>& RequiredSystem : Guard.GetGuardRequiredBlueprintPacks())
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
        const FName ExpectedSystemAssetName(*FString::Printf(TEXT("PDA_%s"), *SystemName.ToString()));
        const FName ExpectedComponentTag(*FString::Printf(TEXT("%s_System"), *SystemName.ToString()));
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
    if (Guard)
    {
        for (const TPair<FName, bool>& RequiredSystem : Guard->GetGuardRequiredBlueprintPacks())
        {
            if (RequiredSystem.Value)
            {
                continue;
            }

            const FName RequiredPDAName(*FString::Printf(TEXT("PDA_%s"), *RequiredSystem.Key.ToString()));
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

    if (Guard && !AreGuardRequirementsAvailable(*Guard))
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
            if (Action->PredictAction(this))
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