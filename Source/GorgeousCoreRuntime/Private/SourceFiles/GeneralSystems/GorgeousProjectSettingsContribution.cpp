// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/GorgeousProjectSettingsContribution.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

#if WITH_EDITOR
#include "Engine/AssetManagerSettings.h"
#include "GameFeatureData.h"
#include "Engine/Engine.h"
#include "Engine/CollisionProfile.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DataRegistry.h"
#include "CommonGameViewportClient.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#endif

bool UGorgeousProjectSettingsContribution::ApplyEntry(UGeneralSystemConfiguration_PDA* Configuration, const FString& TargetName,
	const FString& Section, const FString& Key, const FString& Value, EGorgeousConfigOperation Operation)
{
#if WITH_EDITOR
	UGorgeousManageConfigurationAction* Action = NewObject<UGorgeousManageConfigurationAction>(this);
	Action->ConfigurationTarget = TargetName;
	Action->Section = Section;
	Action->Key = Key;
	Action->Value = Value;
	Action->Operation = Operation;
	return Action->ApplyConfiguration(Configuration);
#else
	return false;
#endif
}

void UGorgeousProjectSettingsContribution::OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* Configuration)
{
#if WITH_EDITOR
	if (!Configuration || !CanExecuteInCurrentSession()) return;
	const FString SystemRoot = FPackageName::GetLongPackagePath(Configuration->GetOutermost()->GetName());
	if (Contribution == EGorgeousProjectSettingContribution::CommonUIViewport)
	{
		const UEngine* Engine = GetDefault<UEngine>();
		UClass* Current = Engine->GameViewportClientClassName.TryLoadClass<UGameViewportClient>();
		if (!Current || !Current->IsChildOf(UCommonGameViewportClient::StaticClass()))
			ApplyEntry(Configuration, TEXT("Engine"), TEXT("/Script/Engine.Engine"), TEXT("GameViewportClientClassName"), TEXT("/Script/CommonUI.CommonGameViewportClient"), EGorgeousConfigOperation::SetOrNewLine);
	}
	else if (Contribution == EGorgeousProjectSettingContribution::DataRegistryDirectories)
	{
		TArray<FAssetData> Registries;
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetAssetsByClass(UDataRegistry::StaticClass()->GetClassPathName(), Registries, true);
		TSet<FString> Directories;
		for (const FAssetData& Registry : Registries)
			if (Registry.PackageName.ToString().StartsWith(SystemRoot + TEXT("/"))) Directories.Add(Registry.PackagePath.ToString());
		for (const FString& Directory : Directories)
			ApplyEntry(Configuration, TEXT("Game"), TEXT("/Script/DataRegistry.DataRegistrySettings"), TEXT("DirectoriesToScan"), FString::Printf(TEXT("(Path=\"%s\")"), *Directory), EGorgeousConfigOperation::Add);
	}
	else if (Contribution == EGorgeousProjectSettingContribution::InteractionTraceChannel)
	{
		UCollisionProfile* Profile = UCollisionProfile::Get();
		int32 Selected = INDEX_NONE;
		for (int32 Channel = ECC_GameTraceChannel1; Channel <= ECC_GameTraceChannel18; ++Channel)
		{
			const FName Name = Profile->ReturnChannelNameFromContainerIndex(Channel);
			if (Name == TEXT("Interactable")) { Selected = Channel; break; }
			if (Selected == INDEX_NONE && (Name.IsNone() || Name.ToString().StartsWith(TEXT("GameTraceChannel")))) Selected = Channel;
		}
		if (Selected == INDEX_NONE)
		{
			GT_E_LOG("GT.ProjectSettings", TEXT("No free collision trace channel. Configure an Interactable trace channel manually; existing channels were preserved."));
			return;
		}
		const FString Section = TEXT("/Script/Engine.CollisionProfile");
		FConfigFile Existing;
		Existing.Read(FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini"));
		TArray<FString> Entries;
		Existing.GetArray(*Section, TEXT("DefaultChannelResponses"), Entries);
		const FString Desired = FString::Printf(TEXT("(Channel=ECC_GameTraceChannel%d,DefaultResponse=ECR_Block,bTraceType=True,bStaticObject=False,Name=\"Interactable\")"), Selected - ECC_GameTraceChannel1 + 1);
		for (const FString& Entry : Entries)
		{
			if (!Entry.Contains(TEXT("Name=\"Interactable\"")) && !Entry.Contains(TEXT("Name=Interactable"))) continue;
			if (Entry == Desired) return;
			ApplyEntry(Configuration, TEXT("Engine"), Section, TEXT("+DefaultChannelResponses"), Entry, EGorgeousConfigOperation::RemoveLine);
			ApplyEntry(Configuration, TEXT("Engine"), Section, TEXT("DefaultChannelResponses"), Entry, EGorgeousConfigOperation::RemoveLine);
		}
		ApplyEntry(Configuration, TEXT("Engine"), Section, TEXT("DefaultChannelResponses"), Desired, EGorgeousConfigOperation::Add);
	}
	else if (Contribution == EGorgeousProjectSettingContribution::AssetManagerSystems)
	{
		FString MountRoot;
		if (!SystemRoot.Split(TEXT("/Systems"), &MountRoot, nullptr)) return;
		const FString ScanPath = MountRoot / TEXT("Systems");
        const FString Section = TEXT("/Script/Engine.AssetManagerSettings");
        FConfigFile Source;
        Source.Read(FPaths::ProjectConfigDir() / TEXT("DefaultGame.ini"));
        TArray<FString> Entries;
        Source.GetArray(*Section, TEXT("PrimaryAssetTypesToScan"), Entries);
        auto EnsureType = [&](FName TypeName, UClass* BaseClass, const FString& Directory, bool bGameFeature)
        {
            FPrimaryAssetTypeInfo Info;
            bool bExistingType = false;
            for (const auto& Existing : GetDefault<UAssetManagerSettings>()->PrimaryAssetTypesToScan)
                if (Existing.PrimaryAssetType == TypeName) { Info = Existing; bExistingType = true; break; }
            TArray<FString> MatchingEntries;
            const FString QuotedKey = FString::Printf(TEXT("PrimaryAssetType=\"%s\""), *TypeName.ToString());
            const FString PlainKey = FString::Printf(TEXT("PrimaryAssetType=%s,"), *TypeName.ToString());
            for (const FString& Entry : Entries)
            {
                if (!Entry.Contains(QuotedKey) && !Entry.Contains(PlainKey)) continue;
                MatchingEntries.Add(Entry);
                FPrimaryAssetTypeInfo::StaticStruct()->ImportText(*Entry, &Info, nullptr, PPF_None, GWarn, TEXT("GorgeousProjectSettings"));
                bExistingType = true;
            }
            Info.PrimaryAssetType = TypeName;
            if (!bGameFeature || !bExistingType)
            {
                Info.SetAssetBaseClass(BaseClass);
                Info.bHasBlueprintClasses = false;
                Info.bIsEditorOnly = false;
            }
            if (!Directory.IsEmpty() && !Info.GetDirectories().ContainsByPredicate([&](const FDirectoryPath& Path) { return Path.Path == Directory; }))
            {
                FDirectoryPath Path; Path.Path = Directory; Info.GetDirectories().Add(Path);
            }
            // Matches UE5.8 GameFeaturesEditor::AddDefaultGameDataRule. Existing nondefault project rules remain intact.
            if (bGameFeature && Info.Rules.IsDefault()) Info.Rules.CookRule = EPrimaryAssetCookRule::AlwaysCook;
            FString Serialized;
            FPrimaryAssetTypeInfo::StaticStruct()->ExportText(Serialized, &Info, nullptr, nullptr, PPF_None, nullptr);
            if (MatchingEntries.Num() == 1 && MatchingEntries[0] == Serialized) return;
            for (const FString& Existing : MatchingEntries)
            {
                ApplyEntry(Configuration, TEXT("Game"), Section, TEXT("+PrimaryAssetTypesToScan"), Existing, EGorgeousConfigOperation::RemoveLine);
                ApplyEntry(Configuration, TEXT("Game"), Section, TEXT("PrimaryAssetTypesToScan"), Existing, EGorgeousConfigOperation::RemoveLine);
            }
            ApplyEntry(Configuration, TEXT("Game"), Section, TEXT("PrimaryAssetTypesToScan"), Serialized, EGorgeousConfigOperation::Add);
        };
        EnsureType(TEXT("GorgeousBlueprintSystems"), UGeneralSystemConfiguration_PDA::StaticClass(), ScanPath, false);
        EnsureType(UGameFeatureData::StaticClass()->GetFName(), UGameFeatureData::StaticClass(), FString(), true);
	}
#endif
}
