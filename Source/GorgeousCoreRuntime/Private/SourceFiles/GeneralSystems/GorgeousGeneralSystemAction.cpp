// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/GorgeousGeneralSystemAction.h"

#include "DataRegistrySubsystem.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/PackageName.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/StreamableManager.h"
#include "UObject/Class.h"
#if WITH_EDITOR
#include "UnrealEdMisc.h"
#endif

namespace
{
	FString NormalizeConfigSection(FString SectionName)
	{
		SectionName.TrimStartAndEndInline();
		if (SectionName.StartsWith(TEXT("[")) && SectionName.EndsWith(TEXT("]")))
		{
			SectionName = SectionName.Mid(1, SectionName.Len() - 2);
		}
		return SectionName;
	}

	FString NormalizeConfigValue(FString ConfigValue)
	{
		ConfigValue.TrimStartAndEndInline();
		const FString ClassPrefix = TEXT("/Script/CoreUObject.Class'");
		if (ConfigValue.StartsWith(ClassPrefix) && ConfigValue.EndsWith(TEXT("'")))
		{
			ConfigValue = ConfigValue.Mid(ClassPrefix.Len(), ConfigValue.Len() - ClassPrefix.Len() - 1);
		}
		return ConfigValue;
	}

	FString GetPersistentProjectConfigPath(const FString& ConfigFile)
	{
		if (ConfigFile.Equals(TEXT("Engine"), ESearchCase::IgnoreCase) ||
			ConfigFile.Equals(GEngineIni, ESearchCase::IgnoreCase))
		{
			return FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
		}
		if (ConfigFile.Equals(TEXT("Game"), ESearchCase::IgnoreCase) ||
			ConfigFile.Equals(GGameIni, ESearchCase::IgnoreCase))
		{
			return FPaths::ProjectConfigDir() / TEXT("DefaultGame.ini");
		}
		if (ConfigFile.Equals(TEXT("Editor"), ESearchCase::IgnoreCase) ||
			ConfigFile.Equals(GEditorIni, ESearchCase::IgnoreCase))
		{
			return FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini");
		}
		return ConfigFile;
	}

	void PromptForEditorRestart()
	{
#if WITH_EDITOR
		const EAppReturnType::Type Result = FMessageDialog::Open(
			EAppMsgType::YesNo,
			NSLOCTEXT(
				"GorgeousCore",
				"ConfigurationRestartRequired",
				"The project configuration was permanently updated. Restart the editor now to apply the change?"));
		if (Result == EAppReturnType::Yes)
		{
			FUnrealEdMisc::Get().RestartEditor(false);
		}
#endif
	}
}

#if WITH_EDITOR
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "BlueprintEditorLibrary.h"
#include "UnrealEdMisc.h"
#include "Engine/Blueprint.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#endif

TArray<FName> UGorgeousGeneralSystemAction::GetAvailableSystemNames()
{
    return UGeneralSystemConfiguration_PDA::GetAllSystemNames();
}

bool UGorgeousGeneralSystemAction::ShouldActivateForSystem_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const
{
    for (const FName& RequiredSystem : RequiredSystems)
    {
        if (!UGeneralSystemConfiguration_PDA::IsSystemPresent(RequiredSystem))
        {
            GT_W_LOG("GT.GeneralSystemAction",
                TEXT("Action '%s' requirement '%s' is unavailable; skipping the action for system '%s'."),
                *GetNameSafe(this), *RequiredSystem.ToString(), *GetNameSafe(SystemConfiguration));
            return false;
        }
    }
    return true;
}

bool UGorgeousGeneralSystemAction::PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const
{
    if ((Target == EGorgeousSystemActionTarget::Editor && !GIsEditor)
        || (Target == EGorgeousSystemActionTarget::Game && GIsEditor))
    {
        return false;
    }

    return ShouldActivateForSystem_Implementation(SystemConfiguration);
}

bool UGorgeousGeneralSystemAction::ExecuteAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration)
{
    if (!PredictAction(SystemConfiguration))
    {
        return false;
    }

    OnSystemActivated(SystemConfiguration);
    return true;
}

bool UGorgeousSystemCondition::Evaluate_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration,
    FGorgeousSystemActionEvaluation& Evaluation) const
{
    Evaluation.bConditionMet = false;
    Evaluation.Reason = NSLOCTEXT("GorgeousCore", "ConditionNotImplemented", "The condition did not provide an evaluation.");
    return false;
}

bool UGorgeousAddDataRegistrySourceAction::PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const
{
    return UGorgeousGeneralSystemAction::PredictAction_Implementation(SystemConfiguration)
        && RegistryType.IsValid()
        && SourceAsset.ToSoftObjectPath().IsValid()
        && UDataRegistrySubsystem::Get() != nullptr;
}

void UGorgeousAddDataRegistrySourceAction::OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration)
{
    const FSoftObjectPath SourcePath = SourceAsset.ToSoftObjectPath();
    if (!RegistryType.IsValid() || !SourcePath.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("General system '%s' has an invalid Data Registry extension action."), *GetNameSafe(SystemConfiguration));
        return;
    }

    if (UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get())
    {
        TMap<FDataRegistryType, TArray<FSoftObjectPath>> AssetsToRegister;
        AssetsToRegister.FindOrAdd(RegistryType).Add(SourcePath);
        DataRegistrySubsystem->PreregisterSpecificAssets(AssetsToRegister, Priority);
    }
}

void UGorgeousAddDataRegistrySourceAction::OnSystemDeactivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration)
{
    if (UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get())
    {
        DataRegistrySubsystem->UnregisterSpecificAsset(RegistryType, SourceAsset.ToSoftObjectPath());
    }
}

TArray<FString> UGorgeousManageConfigurationAction::GetConfigurationTargets() const
{
    return { TEXT("Game"), TEXT("Engine"), TEXT("Editor"), TEXT("GameplayTags") };
}

TArray<FString> UGorgeousManageConfigurationAction::ResolveConfigFiles() const
{
    TArray<FString> Result;
    const FString& TargetName = ConfigurationTarget;
    {
        if (TargetName.Equals(TEXT("Game"), ESearchCase::IgnoreCase) || TargetName.Equals(TEXT("GameplayTags"), ESearchCase::IgnoreCase))
        {
            Result.AddUnique(GGameIni);
        }
        else if (TargetName.Equals(TEXT("Engine"), ESearchCase::IgnoreCase))
        {
            Result.AddUnique(GEngineIni);
        }
        else if (TargetName.Equals(TEXT("Editor"), ESearchCase::IgnoreCase))
        {
            Result.AddUnique(GEditorIni);
        }
    }
    return Result;
}

bool UGorgeousConfigValueCondition::Evaluate_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration,
    FGorgeousSystemActionEvaluation& Evaluation) const
{
    const FString ConfigFile = ConfigurationTarget.Equals(TEXT("Engine"), ESearchCase::IgnoreCase) ? GEngineIni :
        ConfigurationTarget.Equals(TEXT("Editor"), ESearchCase::IgnoreCase) ? GEditorIni :
        ConfigurationTarget.Equals(TEXT("Game"), ESearchCase::IgnoreCase) || ConfigurationTarget.Equals(TEXT("GameplayTags"), ESearchCase::IgnoreCase)
            ? GGameIni : ConfigurationTarget;
    FString CurrentValue;
    const FString NormalizedSection = NormalizeConfigSection(Section);
    const bool bFound = GConfig->GetString(*NormalizedSection, *Key, CurrentValue, *ConfigFile);
    Evaluation.Value = CurrentValue;
    Evaluation.bConditionMet = bFound && (!bUseExpectedValue || CurrentValue == ExpectedValue);
    Evaluation.Reason = Evaluation.bConditionMet
        ? NSLOCTEXT("GorgeousCore", "ConditionMatched", "The configuration condition matched.")
        : NSLOCTEXT("GorgeousCore", "ConditionNotMatched", "The configuration condition did not match.");
    return Evaluation.bConditionMet;
}

TArray<FString> UGorgeousConfigValueCondition::GetConfigurationTargets() const
{
	return { TEXT("Game"), TEXT("Engine"), TEXT("Editor"), TEXT("GameplayTags") };
}

bool UGorgeousClassInheritanceCondition::Evaluate_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration,
    FGorgeousSystemActionEvaluation& Evaluation) const
{
    if (ConfigurationTarget.IsEmpty() || Section.IsEmpty() || Key.IsEmpty() || !ExpectedClass.ToSoftObjectPath().IsValid())
    {
        Evaluation.bConditionMet = false;
        Evaluation.Value.Reset();
        Evaluation.Reason = NSLOCTEXT(
            "GorgeousCore",
            "InvalidClassCondition",
            "The class condition is incomplete and was not evaluated.");
        GT_E_LOG("GT.GeneralSystemAction.Condition",
            TEXT("Class condition '%s' is incomplete: target='%s', section='%s', key='%s', expected='%s'."),
            *GetNameSafe(this),
            *ConfigurationTarget,
            *Section,
            *Key,
            *GetNameSafe(ExpectedClass.Get()));
        return false;
    }

    FString ConfigValue;
    const FString ConfigFile = ConfigurationTarget;
    const FString NormalizedSection = NormalizeConfigSection(Section);
    const FString PersistentConfigFile = GetPersistentProjectConfigPath(ConfigFile);
    FConfigFile PersistentConfig;
    PersistentConfig.Read(PersistentConfigFile);
    const bool bFound = PersistentConfig.GetString(*NormalizedSection, *Key, ConfigValue);
    UClass* ConfiguredClass = bFound ? StaticLoadClass(UObject::StaticClass(), nullptr, *ConfigValue) : nullptr;
    UClass* RequiredClass = ExpectedClass.LoadSynchronous();
    const FString CanonicalConfiguredPath = NormalizeConfigValue(ConfigValue);
    const FString CanonicalRequiredPath = RequiredClass ? RequiredClass->GetPathName() : FString();
    const bool bCanonicalPathMatches = bFound && CanonicalConfiguredPath.Equals(CanonicalRequiredPath, ESearchCase::IgnoreCase);
    Evaluation.Value = ConfigValue;
    Evaluation.bConditionMet = RequiredClass &&
        (bCanonicalPathMatches ||
            (ConfiguredClass &&
                (Comparison == EGorgeousClassComparison::ExactClass
                    ? ConfiguredClass == RequiredClass
                    : ConfiguredClass->IsChildOf(RequiredClass))));
    GT_I_LOG("GT.GeneralSystemAction.Condition",
        TEXT("Class condition '%s': persistentFile='%s', target='%s', section='%s', key='%s', found=%s, rawValue='%s', normalizedValue='%s', configured='%s', expected='%s', expectedPath='%s', canonicalMatch=%s, matched=%s."),
        *GetNameSafe(this),
        *PersistentConfigFile,
        *ConfigFile,
        *NormalizedSection,
        *Key,
        bFound ? TEXT("true") : TEXT("false"),
        *ConfigValue,
        *CanonicalConfiguredPath,
        *GetNameSafe(ConfiguredClass),
        *GetNameSafe(RequiredClass),
        *CanonicalRequiredPath,
        bCanonicalPathMatches ? TEXT("true") : TEXT("false"),
        Evaluation.bConditionMet ? TEXT("true") : TEXT("false"));
    Evaluation.Reason = Evaluation.bConditionMet
        ? NSLOCTEXT("GorgeousCore", "ClassConditionMatched", "The configured class is compatible.")
        : NSLOCTEXT("GorgeousCore", "ClassConditionNotMatched", "The configured class is missing or incompatible.");
    return Evaluation.bConditionMet;
}

bool UGorgeousAssetExistsCondition::Evaluate_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration,
    FGorgeousSystemActionEvaluation& Evaluation) const
{
    const bool bExists = Asset.ToSoftObjectPath().IsValid() && (Asset.IsValid() || Asset.LoadSynchronous() != nullptr);
    Evaluation.bConditionMet = bExists;
    Evaluation.Value = Asset.ToSoftObjectPath().ToString();
    Evaluation.Reason = bExists
        ? NSLOCTEXT("GorgeousCore", "AssetExists", "The configured asset exists.")
        : NSLOCTEXT("GorgeousCore", "AssetMissing", "The configured asset does not exist.");
    return bExists;
}

bool UGorgeousPluginEnabledCondition::Evaluate_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration,
    FGorgeousSystemActionEvaluation& Evaluation) const
{
    const bool bEnabled = !PluginName.IsNone() && IPluginManager::Get().FindEnabledPlugin(PluginName.ToString()).IsValid();
    Evaluation.bConditionMet = bEnabled;
    Evaluation.Value = PluginName.ToString();
    Evaluation.Reason = bEnabled
        ? NSLOCTEXT("GorgeousCore", "PluginEnabled", "The plugin is enabled.")
        : NSLOCTEXT("GorgeousCore", "PluginDisabled", "The plugin is missing or disabled.");
    return bEnabled;
}

namespace GorgeousSystemConditionPrivate
{
	static bool FindSystemManifest(const UGeneralSystemConfiguration_PDA* System, FString& OutManifest)
	{
		if (!System)
		{
			return false;
		}

		const FString PackageFilename = FPackageName::LongPackageNameToFilename(System->GetOutermost()->GetName(), TEXT(".uasset"));
		if (!System->bIsCodeSystem &&
			FFileHelper::LoadFileToString(OutManifest, *(FPaths::GetPath(PackageFilename) / TEXT("SystemManifest.json"))))
		{
			return true;
		}

		for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
		{
			TArray<FString> Matches;
			IFileManager::Get().FindFilesRecursive(Matches, *(Plugin->GetBaseDir() / TEXT("Source")), TEXT("SystemManifest.json"), true, false);
			for (const FString& Match : Matches)
			{
				FString Contents;
				if (FFileHelper::LoadFileToString(Contents, *Match))
				{
					TSharedPtr<FJsonObject> Json;
					const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
					FString ManifestSystemId;
					if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid() &&
						Json->TryGetStringField(TEXT("SystemId"), ManifestSystemId) &&
						ManifestSystemId.Equals(System->GetPDAAssetName().ToString().Replace(TEXT("PDA_"), TEXT("")), ESearchCase::IgnoreCase))
					{
						OutManifest = MoveTemp(Contents);
						return true;
					}
				}
			}
		}
		return false;
	}
}

bool UGorgeousSystemPresentCondition::Evaluate_Implementation(
	UGeneralSystemConfiguration_PDA* SystemConfiguration,
	FGorgeousSystemActionEvaluation& Evaluation) const
{
	UGeneralSystemConfiguration_PDA* TargetSystem = System.LoadSynchronous();
	if (!TargetSystem)
	{
		TargetSystem = SystemConfiguration;
	}

	FString Manifest;
	const bool bPresent = TargetSystem != nullptr;
	const bool bHasManifest = GorgeousSystemConditionPrivate::FindSystemManifest(TargetSystem, Manifest);
	Evaluation.Value = Manifest;
	Evaluation.bConditionMet = bPresent && (!bRequireManifest || bHasManifest);
	Evaluation.Reason = Evaluation.bConditionMet
		? NSLOCTEXT("GorgeousCore", "SystemPresent", "The general system is present.")
		: NSLOCTEXT("GorgeousCore", "SystemMissing", "The configured general system or its manifest is missing.");
	return Evaluation.bConditionMet;
}

bool UGorgeousActionPredictionCondition::Evaluate_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration,
    FGorgeousSystemActionEvaluation& Evaluation) const
{
    const bool bPrediction = Action && Action->PredictAction(SystemConfiguration);
    Evaluation.bConditionMet = bPrediction == bExpectedResult;
    Evaluation.Reason = Evaluation.bConditionMet
        ? NSLOCTEXT("GorgeousCore", "ActionPredictionMatched", "The action prediction matched.")
        : NSLOCTEXT("GorgeousCore", "ActionPredictionDidNotMatch", "The action prediction did not match.");
    return Evaluation.bConditionMet;
}

bool UGorgeousManageConfigurationAction::IsTriggerSatisfied() const
{
    const TArray<FString> ConfigFiles = ResolveConfigFiles();
    if (ConfigFiles.IsEmpty() || Section.IsEmpty() || Key.IsEmpty())
    {
        return false;
    }

    if (bVersionDependent)
    {
        const FEngineVersion Current = FEngineVersion::Current();
        FEngineVersion Minimum;
        FEngineVersion Maximum;
        const bool bHasMinimum = !MinEngineVersion.IsEmpty() && FEngineVersion::Parse(MinEngineVersion, Minimum);
        const bool bHasMaximum = !MaxEngineVersion.IsEmpty() && FEngineVersion::Parse(MaxEngineVersion, Maximum);
        if ((!MinEngineVersion.IsEmpty() && !bHasMinimum) || (!MaxEngineVersion.IsEmpty() && !bHasMaximum))
        {
            GT_W_LOG("GT.GeneralSystemAction",
                TEXT("Configuration action '%s' has an invalid engine-version range."), *GetNameSafe(this));
            return false;
        }

        const bool bBelowMinimum = bHasMinimum &&
            FEngineVersionBase::GetNewest(Current, Minimum, nullptr) == EVersionComparison::Second;
        const bool bAboveMaximum = bHasMaximum &&
            FEngineVersionBase::GetNewest(Current, Maximum, nullptr) == EVersionComparison::First;
        if (bBelowMinimum || bAboveMaximum)
        {
            return false;
        }
    }

    if (Trigger == EGorgeousConfigTrigger::OnAssetOpened && !bHandlingAssetOpened)
    {
#if WITH_EDITOR
        if (!GEditor)
        {
            return false;
        }
        UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
        for (const TSoftObjectPtr<UObject>& TriggerAsset : TriggerAssets)
        {
            UObject* Asset = TriggerAsset.Get();
            if (Asset && AssetEditorSubsystem->FindEditorForAsset(Asset, false))
            {
                return true;
            }
        }
#endif
        return false;
    }

    for (const FString& ConfigFile : ConfigFiles)
    {
        FString CurrentValue;
        const FString NormalizedSection = NormalizeConfigSection(Section);
        bool bPresent = false;
        const FString PersistentConfigFile = GetPersistentProjectConfigPath(ConfigFile);
        FConfigFile PersistentConfig;
        PersistentConfig.Read(PersistentConfigFile);
        bPresent = PersistentConfig.GetString(*NormalizedSection, *Key, CurrentValue);
        ReadValue = bPresent ? CurrentValue : FString();
        if (Operation == EGorgeousConfigOperation::Read)
        {
            return bPresent;
        }
        if (Trigger == EGorgeousConfigTrigger::IfPresent && bPresent)
        {
            return true;
        }
        if (Trigger == EGorgeousConfigTrigger::IfNotSetToValue &&
            (!bPresent || NormalizeConfigValue(CurrentValue) != NormalizeConfigValue(Value)))
        {
            return true;
        }
        if (Trigger == EGorgeousConfigTrigger::IfNotPresent && !bPresent)
        {
            return true;
        }
    }
    return false;
}

bool UGorgeousManageConfigurationAction::PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const
{
    const bool bBasePrediction = UGorgeousGeneralSystemAction::PredictAction_Implementation(SystemConfiguration);
    const bool bTriggerSatisfied = bBasePrediction && IsTriggerSatisfied();
    GT_I_LOG("GT.GeneralSystemAction.Configuration",
        TEXT("Configuration action '%s' prediction: base=%s, trigger=%s, target='%s', section='%s', key='%s', operation=%d, configuredValue='%s'."),
        *GetNameSafe(this),
        bBasePrediction ? TEXT("true") : TEXT("false"),
        bTriggerSatisfied ? TEXT("true") : TEXT("false"),
        *ConfigurationTarget,
        *Section,
        *Key,
        static_cast<int32>(Operation),
        *Value);
    return bTriggerSatisfied;
}

bool UGorgeousReparentBlueprintAction::PredictAction_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration) const
{
    if (!UGorgeousGeneralSystemAction::PredictAction_Implementation(SystemConfiguration)
        || !NewParentClass.ToSoftObjectPath().IsValid())
    {
        return false;
    }

    if (!bDrawBlueprintFromConfig)
    {
        return Blueprint.LoadSynchronous() != nullptr;
    }

    if (!BlueprintConfig)
    {
        return false;
    }

    if (BlueprintConfig->Operation != EGorgeousConfigOperation::Read ||
        !BlueprintConfig->PredictAction(SystemConfiguration))
    {
        return false;
    }

#if WITH_EDITOR
    UObject* ConfiguredObject = FSoftObjectPath(NormalizeConfigValue(BlueprintConfig->ReadValue)).TryLoad();
    if (Cast<UBlueprint>(ConfiguredObject))
    {
        return true;
    }

    if (const UClass* ConfiguredClass = Cast<UClass>(ConfiguredObject))
    {
        return ConfiguredClass->ClassGeneratedBy != nullptr;
    }

    return false;
#else
    return FSoftObjectPath(NormalizeConfigValue(BlueprintConfig->ReadValue)).IsValid();
#endif
}

bool UGorgeousReparentBlueprintAction::ExecuteAction_Implementation(
    UGeneralSystemConfiguration_PDA* SystemConfiguration)
{
    GT_I_LOG("GT.GeneralSystemAction.ReparentBlueprint",
        TEXT("Executing Blueprint reparent action '%s' for system '%s'."),
        *GetNameSafe(this), *GetNameSafe(SystemConfiguration));

    if (!PredictAction(SystemConfiguration))
    {
        GT_W_LOG("GT.GeneralSystemAction.ReparentBlueprint",
            TEXT("Blueprint reparent action '%s' prediction failed."),
            *GetNameSafe(this));
        return false;
    }
#if WITH_EDITOR
    UBlueprint* BlueprintAsset = nullptr;
    FString ConfiguredObjectPath;
    if (bDrawBlueprintFromConfig)
    {
        ConfiguredObjectPath = BlueprintConfig ? BlueprintConfig->ReadValue : FString();
        GT_I_LOG("GT.GeneralSystemAction.ReparentBlueprint",
            TEXT("Configuration action '%s' returned Blueprint path '%s'."),
            *GetNameSafe(BlueprintConfig), *ConfiguredObjectPath);
        UObject* ConfiguredObject = FSoftObjectPath(ConfiguredObjectPath).TryLoad();
        BlueprintAsset = Cast<UBlueprint>(ConfiguredObject);
        if (!BlueprintAsset)
        {
            if (UClass* ConfiguredClass = Cast<UClass>(ConfiguredObject))
            {
                BlueprintAsset = Cast<UBlueprint>(ConfiguredClass->ClassGeneratedBy);
            }
        }
    }
    else
    {
        BlueprintAsset = Blueprint.LoadSynchronous();
    }
    UClass* ParentClass = NewParentClass.LoadSynchronous();
    if (!BlueprintAsset || !ParentClass)
    {
        GT_E_LOG("GT.GeneralSystemAction.ReparentBlueprint",
            TEXT("Unable to resolve Blueprint asset or target parent class for action '%s' (configured object '%s', Blueprint '%s', parent '%s')."),
            *GetNameSafe(this),
            *ConfiguredObjectPath,
            *GetNameSafe(BlueprintAsset),
            *GetNameSafe(ParentClass));
        return false;
    }

    GT_I_LOG("GT.GeneralSystemAction.ReparentBlueprint",
        TEXT("Resolved Blueprint asset '%s' from configured object path '%s'."),
        *BlueprintAsset->GetPathName(), *ConfiguredObjectPath);
    UBlueprintEditorLibrary::ReparentBlueprint(BlueprintAsset, ParentClass);
    const bool bReparented = BlueprintAsset->ParentClass == ParentClass;
    BlueprintAsset->MarkPackageDirty();
    GT_I_LOG("GT.GeneralSystemAction.ReparentBlueprint",
        TEXT("Blueprint '%s' reparent result: %s, current parent '%s', target parent '%s'."),
        *GetNameSafe(BlueprintAsset),
        bReparented ? TEXT("success") : TEXT("failed"),
        *GetNameSafe(BlueprintAsset->ParentClass),
        *GetNameSafe(ParentClass));
    return bReparented;
#else
    return false;
#endif
}

void UGorgeousManageConfigurationAction::OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration)
{
#if WITH_EDITOR
	if (Trigger == EGorgeousConfigTrigger::OnAssetOpened)
	{
		BindAssetOpenedEvent();
		return;
	}
#endif

    const TArray<FString> ConfigFiles = ResolveConfigFiles();
    if (ConfigFiles.IsEmpty() || Section.IsEmpty() || Key.IsEmpty() || !IsTriggerSatisfied())
    {
        return;
    }

#if WITH_EDITOR
    if (bPrompt)
    {
        const FText PromptText = PromptMessage.IsEmpty()
            ? NSLOCTEXT("GorgeousCore", "AddConfigurationPrompt", "Apply the requested Gorgeous configuration?")
            : PromptMessage;
        GT_W_LOG_FULL_EX("GT.GeneralSystemAction.ConfigurationPrompt",
            TEXT("%s"), GT_DURATION, false, true, true, true, nullptr, nullptr, *PromptText.ToString());
        if (!bRequired)
        {
            if (FMessageDialog::Open(EAppMsgType::YesNo, PromptText) != EAppReturnType::Yes)
            {
                return;
            }
        }
        else
        {
            FMessageDialog::Open(EAppMsgType::Ok, PromptText);
        }
    }
#endif

    ApplyConfiguration(SystemConfiguration);
}

#if WITH_EDITOR
void UGorgeousManageConfigurationAction::BindAssetOpenedEvent()
{
	if (AssetOpenedHandle.IsValid() || !GEditor)
	{
		return;
	}

	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetOpenedHandle = AssetEditorSubsystem->OnAssetOpenedInEditor().AddUObject(this, &UGorgeousManageConfigurationAction::HandleAssetOpened);
	}
}

void UGorgeousManageConfigurationAction::UnbindAssetOpenedEvent()
{
	if (!AssetOpenedHandle.IsValid() || !GEditor)
	{
		return;
	}

	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->OnAssetOpenedInEditor().Remove(AssetOpenedHandle);
	}
	AssetOpenedHandle.Reset();
}

void UGorgeousManageConfigurationAction::OnSystemDeactivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration)
{
	UnbindAssetOpenedEvent();
}

void UGorgeousManageConfigurationAction::HandleAssetOpened(UObject* Asset, IAssetEditorInstance* EditorInstance)
{
	if (!Asset || !EditorInstance)
	{
		return;
	}

	for (const TSoftObjectPtr<UObject>& TriggerAsset : TriggerAssets)
	{
		if (TriggerAsset.ToSoftObjectPath() == FSoftObjectPath(Asset))
		{
			bHandlingAssetOpened = true;
			OnSystemActivated(GetTypedOuter<UGeneralSystemConfiguration_PDA>());
			bHandlingAssetOpened = false;
			return;
		}
	}
}
#endif

void UGorgeousManageConfigurationAction::ApplyConfiguration(UGeneralSystemConfiguration_PDA* SystemConfiguration) const
{
    const TArray<FString> ConfigFiles = ResolveConfigFiles();
    const FString NormalizedSection = NormalizeConfigSection(Section);
    FString ConfigKey = Key;
    FString ConfigValue = NormalizeConfigValue(Value);
    switch (Operation)
    {
    case EGorgeousConfigOperation::Read:
        return;
    case EGorgeousConfigOperation::SetOrNewLine:
        for (const FString& ConfigFile : ConfigFiles)
        {
            const FString WritableConfigFile = GetPersistentProjectConfigPath(ConfigFile);
            FConfigFile PersistentConfig;
            PersistentConfig.Read(WritableConfigFile);
            PersistentConfig.SetString(*NormalizedSection, *Key, *ConfigValue);
            const bool bWriteSucceeded = PersistentConfig.Write(WritableConfigFile);

            FString PersistedValue;
            const bool bPersisted = PersistentConfig.GetString(*NormalizedSection, *Key, PersistedValue);
            GT_I_LOG("GT.GeneralSystemAction",
                TEXT("Permanently set configuration '%s.%s' to '%s' in '%s' (persisted=%s, read-back='%s')."),
                *NormalizedSection,
                *Key,
                *ConfigValue,
                *WritableConfigFile,
                bWriteSucceeded && bPersisted && PersistedValue == ConfigValue ? TEXT("true") : TEXT("false"),
                *PersistedValue);
        }
        PromptForEditorRestart();
        return;
    case EGorgeousConfigOperation::Add:
        ConfigKey = TEXT("+") + Key;
        break;
    case EGorgeousConfigOperation::Remove:
        ConfigKey = TEXT("-") + Key;
        break;
    case EGorgeousConfigOperation::Clear:
        ConfigKey = TEXT("!") + Key;
        ConfigValue.Reset();
        break;
    case EGorgeousConfigOperation::RemoveLine:
        for (const FString& ConfigFile : ConfigFiles)
        {
            const FString WritableConfigFile = GetPersistentProjectConfigPath(ConfigFile);
            FConfigFile PersistentConfig;
            PersistentConfig.Read(WritableConfigFile);
            PersistentConfig.RemoveKeyFromSection(*NormalizedSection, *Key);
            PersistentConfig.Write(WritableConfigFile);
        }
        PromptForEditorRestart();
        return;
    default:
        break;
    }

    for (const FString& ConfigFile : ConfigFiles)
    {
        const FString WritableConfigFile = GetPersistentProjectConfigPath(ConfigFile);
        FConfigFile PersistentConfig;
        PersistentConfig.Read(WritableConfigFile);
        PersistentConfig.SetString(*NormalizedSection, *ConfigKey, *ConfigValue);
        const bool bWriteSucceeded = PersistentConfig.Write(WritableConfigFile);
        GT_I_LOG_FULL_EX("GT.GeneralSystemAction",
            TEXT("Permanently applied configuration action for '%s' to %s (write=%s)."),
            GT_DURATION, false, true, true, true, nullptr, nullptr,
            *GetNameSafe(SystemConfiguration),
            *WritableConfigFile,
            bWriteSucceeded ? TEXT("success") : TEXT("failed"));
    }
    PromptForEditorRestart();
}

namespace GorgeousPromptPrivate
{
    static FString GetPromptKey(const UGorgeousTriggerPromptAction* Action, const UGeneralSystemConfiguration_PDA* SystemConfiguration)
    {
        FString ConditionSignature;
        if (const UGorgeousClassInheritanceCondition* ClassCondition = Cast<UGorgeousClassInheritanceCondition>(Action ? Action->ConditionObject : nullptr))
        {
            ConditionSignature = FString::Printf(
                TEXT("|Class|%s|%s|%s|%s|%d"),
                *ClassCondition->ConfigurationTarget,
                *ClassCondition->Section,
                *ClassCondition->Key,
                *ClassCondition->ExpectedClass.ToSoftObjectPath().ToString(),
                static_cast<int32>(ClassCondition->Comparison));
        }
        else if (const UGorgeousConfigValueCondition* ConfigCondition = Cast<UGorgeousConfigValueCondition>(Action ? Action->ConditionObject : nullptr))
        {
            ConditionSignature = FString::Printf(
                TEXT("|Config|%s|%s|%s|%s|%d"),
                *ConfigCondition->ConfigurationTarget,
                *ConfigCondition->Section,
                *ConfigCondition->Key,
                *ConfigCondition->ExpectedValue,
                ConfigCondition->bUseExpectedValue ? 1 : 0);
        }

        return FString::Printf(
            TEXT("%s|%s|%s|%d"),
            *GetPathNameSafe(SystemConfiguration),
            *GetNameSafe(Action),
            *ConditionSignature,
            Action && Action->bInvertCondition ? 1 : 0);
    }

#if WITH_EDITOR
    static bool IsReparentOptionUsable(const UGorgeousPromptOption* Option, UGeneralSystemConfiguration_PDA* SystemConfiguration)
    {
        const UGorgeousReparentBlueprintAction* ReparentAction = Option ? Cast<UGorgeousReparentBlueprintAction>(Option->Action) : nullptr;
        if (!ReparentAction)
        {
            return true;
        }

        return ReparentAction->PredictAction(SystemConfiguration);
    }
#endif

    static bool IsDismissed(const FString& Key)
    {
        FString Contents;
        TSharedPtr<FJsonObject> Root;
        const FString Path = FPaths::ProjectSavedDir() / TEXT("GorgeousThings/GorgeousPersistentData.json");
        if (!FFileHelper::LoadFileToString(Contents, *Path))
        {
            return false;
        }
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        {
            return false;
        }
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (!Root->TryGetArrayField(TEXT("DismissedSystemActionPrompts"), Values) || !Values)
        {
            return false;
        }
        for (const TSharedPtr<FJsonValue>& Value : *Values)
        {
            if (Value.IsValid() && Value->AsString() == Key)
            {
                return true;
            }
        }
        return false;
    }

    static void Dismiss(const FString& Key)
    {
        const FString Path = FPaths::ProjectSavedDir() / TEXT("GorgeousThings/GorgeousPersistentData.json");
        FString Contents;
        TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
        if (FFileHelper::LoadFileToString(Contents, *Path))
        {
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
            TSharedPtr<FJsonObject> Parsed;
            if (FJsonSerializer::Deserialize(Reader, Parsed) && Parsed.IsValid())
            {
                Root = Parsed;
            }
        }

        TArray<TSharedPtr<FJsonValue>> Values;
        const TArray<TSharedPtr<FJsonValue>>* Existing = nullptr;
        if (Root->TryGetArrayField(TEXT("DismissedSystemActionPrompts"), Existing) && Existing)
        {
            Values = *Existing;
        }
        Values.Add(MakeShared<FJsonValueString>(Key));
        Root->SetArrayField(TEXT("DismissedSystemActionPrompts"), Values);
        FString Output;
        const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
        if (FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
        {
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
            FFileHelper::SaveStringToFile(Output, *Path);
        }
    }
}

bool UGorgeousTriggerPromptAction::PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const
{
    if (!UGorgeousGeneralSystemAction::PredictAction_Implementation(SystemConfiguration))
    {
        GT_W_LOG("GT.GeneralSystemAction.TriggerPrompt", TEXT("Trigger prompt '%s' was rejected by its base action gates for system '%s'."), *GetNameSafe(this), *GetNameSafe(SystemConfiguration));
        return false;
    }
    if (GorgeousPromptPrivate::IsDismissed(GorgeousPromptPrivate::GetPromptKey(this, SystemConfiguration)))
    {
        GT_I_LOG("GT.GeneralSystemAction.TriggerPrompt", TEXT("Trigger prompt '%s' was dismissed for system '%s'."), *GetNameSafe(this), *GetNameSafe(SystemConfiguration));
        return false;
    }
    if (ConditionObject)
    {
        if (const UGorgeousConfigValueCondition* ConfigCondition = Cast<UGorgeousConfigValueCondition>(ConditionObject))
        {
            if (ConfigCondition->ConfigurationTarget.IsEmpty() ||
                ConfigCondition->Section.IsEmpty() ||
                ConfigCondition->Key.IsEmpty())
            {
                GT_E_LOG("GT.GeneralSystemAction.TriggerPrompt",
                    TEXT("Trigger prompt '%s' has an incomplete configuration condition '%s'; suppressing the prompt."),
                    *GetNameSafe(this),
                    *GetNameSafe(ConditionObject));
                return false;
            }
        }

        const bool bConditionMet = ConditionObject->Evaluate(SystemConfiguration, LastEvaluation);
        const bool bShouldPrompt = bInvertCondition ? !bConditionMet : bConditionMet;
        GT_I_LOG("GT.GeneralSystemAction.TriggerPrompt",
            TEXT("Trigger prompt '%s' decision: condition='%s', value='%s', matched=%s, inverted=%s, shouldPrompt=%s, system='%s'."),
            *GetNameSafe(this),
            *GetNameSafe(ConditionObject),
            *LastEvaluation.Value,
            bConditionMet ? TEXT("true") : TEXT("false"),
            bInvertCondition ? TEXT("true") : TEXT("false"),
            bShouldPrompt ? TEXT("true") : TEXT("false"),
            *GetNameSafe(SystemConfiguration));
        return bShouldPrompt;
    }
    if (Condition == EGorgeousPromptCondition::OnFirstStart)
    {
        return true;
    }
    const UGorgeousGeneralSystemAction* Action = ActionToPredict.Get();
    if (!Action)
    {
        return false;
    }
    const bool bPrediction = Action->PredictAction(SystemConfiguration);
    const bool bConditionMet = bPrediction == (Condition == EGorgeousPromptCondition::IfActionPredictionSucceeds);
    return bInvertCondition ? !bConditionMet : bConditionMet;
}

void UGorgeousTriggerPromptAction::OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration)
{
#if WITH_EDITOR
    if (!GEditor || GorgeousPromptPrivate::IsDismissed(GorgeousPromptPrivate::GetPromptKey(this, SystemConfiguration)))
    {
        GT_W_LOG("GT.GeneralSystemAction.TriggerPrompt", TEXT("Trigger prompt '%s' activation skipped because the editor is unavailable or the prompt is dismissed."), *GetNameSafe(this));
        return;
    }

    const FText PromptText = Message.IsEmpty()
        ? NSLOCTEXT("GorgeousCore", "TriggerPromptDefault", "A Gorgeous system action requires your attention.")
        : Message;
    GT_W_LOG_FULL_EX("GT.GeneralSystemAction.TriggerPrompt",
        TEXT("%s"), GT_DURATION, false, true, true, true, nullptr, nullptr, *PromptText.ToString());
    if (Options.IsEmpty())
    {
        const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, PromptText);
        if (Result == EAppReturnType::No)
        {
            GorgeousPromptPrivate::Dismiss(GorgeousPromptPrivate::GetPromptKey(this, SystemConfiguration));
        }
        return;
    }

    TSharedPtr<SWindow> PromptWindow = SNew(SWindow)
        .Title(NSLOCTEXT("GorgeousCore", "TriggerPromptTitle", "Gorgeous System Action"))
        .SizingRule(ESizingRule::Autosized)
        .SupportsMinimize(false)
        .SupportsMaximize(false);
    TArray<UGorgeousPromptOption*> VisibleOptions;
    VisibleOptions.Reserve(Options.Num());
    for (UGorgeousPromptOption* Option : Options)
    {
        if (GorgeousPromptPrivate::IsReparentOptionUsable(Option, SystemConfiguration))
        {
            VisibleOptions.Add(Option);
        }
        else
        {
            GT_I_LOG("GT.GeneralSystemAction.TriggerPrompt",
                TEXT("Prompt '%s' omitted reparent option '%s' because the configured value is a native class without a Blueprint asset."),
                *GetNameSafe(this),
                *GetNameSafe(Option));
        }
    }

    if (VisibleOptions.IsEmpty())
    {
        GT_W_LOG("GT.GeneralSystemAction.TriggerPrompt",
            TEXT("Prompt '%s' has no usable options after filtering."),
            *GetNameSafe(this));
        return;
    }

    TSharedPtr<int32> SelectedOption = MakeShared<int32>(INDEX_NONE);
    TSharedRef<SVerticalBox> Content = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(18.0f)
        [
            SNew(STextBlock).Text(PromptText).AutoWrapText(true)
        ];
    for (int32 Index = 0; Index < VisibleOptions.Num(); ++Index)
    {
        const UGorgeousPromptOption* Option = VisibleOptions[Index];
        if (!Option) continue;
        const FLinearColor ButtonColor = Option->Style == EGorgeousPromptButtonStyle::Primary
            ? FLinearColor(0.05f, 0.35f, 0.85f)
            : Option->Style == EGorgeousPromptButtonStyle::Destructive
                ? FLinearColor(0.75f, 0.12f, 0.08f)
                : Option->Style == EGorgeousPromptButtonStyle::Warning
                    ? FLinearColor(0.75f, 0.48f, 0.05f)
                    : FLinearColor(0.25f, 0.25f, 0.25f);
        Content->AddSlot().AutoHeight().Padding(18.0f, 0.0f, 18.0f, 8.0f)
        [
            SNew(SButton)
            .ButtonColorAndOpacity(ButtonColor)
            .OnClicked_Lambda([PromptWindow, SelectedOption, Index]()
            {
                *SelectedOption = Index;
                PromptWindow->RequestDestroyWindow();
                return FReply::Handled();
            })
            [
                SNew(STextBlock).Text(Option->Label)
            ]
        ];
    }
    PromptWindow->SetContent(Content);
    FSlateApplication::Get().AddModalWindow(PromptWindow.ToSharedRef(), nullptr);
    if (*SelectedOption != INDEX_NONE && VisibleOptions.IsValidIndex(*SelectedOption))
    {
        UGorgeousPromptOption* Selected = VisibleOptions[*SelectedOption];
        if (Selected && Selected->Action)
        {
            const bool bActionSucceeded = Selected->Action->ExecuteAction(SystemConfiguration);
            GT_I_LOG("GT.GeneralSystemAction.TriggerPrompt",
                TEXT("Prompt '%s' selected option '%s'; nested action '%s' result: %s."),
                *GetNameSafe(this),
                *Selected->Label.ToString(),
                *GetNameSafe(Selected->Action),
                bActionSucceeded ? TEXT("success") : TEXT("failed"));
        }
        else
        {
            GT_W_LOG("GT.GeneralSystemAction.TriggerPrompt",
                TEXT("Prompt '%s' selected option index %d, but the option has no action."),
                *GetNameSafe(this), *SelectedOption);
        }
        if (Selected && Selected->bDismissPrompt)
        {
            GorgeousPromptPrivate::Dismiss(GorgeousPromptPrivate::GetPromptKey(this, SystemConfiguration));
        }
    }
#else
    UE_LOG(LogTemp, Warning, TEXT("Editor-only trigger prompt '%s' was skipped outside the editor."), *GetNameSafe(this));
#endif
}
