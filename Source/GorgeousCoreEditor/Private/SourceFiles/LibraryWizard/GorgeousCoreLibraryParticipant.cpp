// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "LibraryWizard/GorgeousCoreLibraryParticipant.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "LibraryWizard/GorgeousRecompilationHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ISettingsModule.h"
#include "LibraryWizard/GorgeousPersistentDataConfig_DA.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "PropertyEditorModule.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"

FGorgeousCoreLibraryParticipant::FGorgeousCoreLibraryParticipant()
{
	RefreshOfflineCache();
}

FGorgeousCoreLibraryParticipant::~FGorgeousCoreLibraryParticipant()
{
	TransientAssets.Empty();
}

FName FGorgeousCoreLibraryParticipant::GetParticipantName() const
{
	return TEXT("GorgeousCore");
}

FText FGorgeousCoreLibraryParticipant::GetParticipantDisplayName() const
{
	return NSLOCTEXT("GorgeousCore", "LibraryParticipantDisplayName", "Gorgeous Core");
}

FName FGorgeousCoreLibraryParticipant::GetParticipantIcon() const
{
	return NAME_None;
}

TArray<FGorgeousLibraryCategoryDescriptor> FGorgeousCoreLibraryParticipant::GetLibraryCategories() const
{
	TArray<FGorgeousLibraryCategoryDescriptor> Categories;

	FGorgeousLibraryCategoryDescriptor CoreSettings;
	CoreSettings.CategoryId = "CoreSettings";
	CoreSettings.DisplayName = NSLOCTEXT("GorgeousCore", "CoreSettingsCategory", "Configuration");
	CoreSettings.AssetClassPath = FTopLevelAssetPath(TEXT("/Script/GorgeousCoreEditor"), TEXT("GorgeousPersistentDataConfig_DA"));
	Categories.Add(CoreSettings);

	FGorgeousLibraryCategoryDescriptor CoreSystems;
	CoreSystems.CategoryId = "CoreSystems";
	CoreSystems.DisplayName = NSLOCTEXT("GorgeousCore", "CoreSystemsCategory", "Core Systems");
	CoreSystems.AssetClassPath = FTopLevelAssetPath(TEXT("/Script/GorgeousCoreEditor"), TEXT("GorgeousInstallableSystem_DA"));

	// Inject custom action buttons directly into the Category Header
	CoreSystems.CategoryActions.Add(FGorgeousLibraryCategoryAction(
		"ConfigureSettings",
		NSLOCTEXT("GorgeousCore", "ConfigureSettings", "Configure API"),
		NSLOCTEXT("GorgeousCore", "ConfigureSettingsTooltip", "Open Project Settings to configure the Gorgeous Core API endpoints and persistent data."),
		"Icons.Settings"
	));

	CoreSystems.CategoryActions.Add(FGorgeousLibraryCategoryAction(
		"Recompile",
		NSLOCTEXT("GorgeousCore", "Recompile", "Recompile Systems"),
		NSLOCTEXT("GorgeousCore", "RecompileTooltip", "Trigger a full recompilation of the Gorgeous Plugin ecosystem."),
		"Icons.Refresh"
	));

	Categories.Add(CoreSystems);

	return Categories;
}

void FGorgeousCoreLibraryParticipant::OnAssetDoubleClicked(const FAssetData& Asset, const FGorgeousLibraryCategoryDescriptor& Category)
{
	// Optional: open a details panel or popup
}

void FGorgeousCoreLibraryParticipant::OnBuildContextMenu(class FMenuBuilder& MenuBuilder, const TArray<FAssetData>& SelectedAssets, const FGorgeousLibraryCategoryDescriptor& Category)
{
	if (SelectedAssets.Num() == 1 && Category.CategoryId == "CoreSystems")
	{
		const FAssetData& Asset = SelectedAssets[0];
		
		MenuBuilder.BeginSection("GorgeousCoreActions", NSLOCTEXT("GorgeousCore", "SystemActionsSection", "System Actions"));
		{
			MenuBuilder.AddMenuEntry(
				NSLOCTEXT("GorgeousCore", "InstallSystem", "Install / Update System"),
				NSLOCTEXT("GorgeousCore", "InstallSystemTooltip", "Download and install this system via the Gorgeous Installer."),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Download"),
				FUIAction(FExecuteAction::CreateLambda([Asset]()
				{
					if (UGorgeousInstallableSystem_DA* System = Cast<UGorgeousInstallableSystem_DA>(Asset.GetAsset()))
					{
						// If the system is pre-shipped, it's already there. But maybe they want to force update it?
						if (System->DownloadUrl.IsEmpty())
						{
							// Ideally use a slate notification here
							return;
						}

						TSharedPtr<IPlugin> CorePlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
						if (CorePlugin.IsValid())
						{
							FString InstallerPath = FPaths::Combine(CorePlugin->GetBaseDir(), TEXT("Source"), TEXT("ThirdParty"), TEXT("GorgeousInstaller"), TEXT("build"));
#if PLATFORM_WINDOWS
							InstallerPath = FPaths::Combine(InstallerPath, TEXT("gorgeous-installer.exe"));
#elif PLATFORM_LINUX
							InstallerPath = FPaths::Combine(InstallerPath, TEXT("gorgeous-installer"));
#else
							return;
#endif
							if (FPaths::FileExists(InstallerPath))
							{
								FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
								uint32 PID = FPlatformProcess::GetCurrentProcessId();
								// Pass the zip URL to the installer!
								FString Args = FString::Printf(TEXT("-cli -project \"%s\" --install-zip \"%s\" --wait-for-pid %d"), *ProjectPath, *System->DownloadUrl, PID);

								FPlatformProcess::CreateProc(*InstallerPath, *Args, true, false, false, nullptr, 0, nullptr, nullptr);
								FGenericPlatformMisc::RequestExit(false);
							}
						}
					}
				}))
			);
		}
		MenuBuilder.EndSection();
	}
}

TSharedPtr<FSlateBrush> FGorgeousCoreLibraryParticipant::GetAssetIcon(const FAssetData& Asset)
{
	return IGorgeousLibraryParticipant::GetAssetIcon(Asset);
}

FText FGorgeousCoreLibraryParticipant::GetAssetDescription(const FAssetData& Asset)
{
	if (UGorgeousInstallableSystem_DA* System = Cast<UGorgeousInstallableSystem_DA>(Asset.GetAsset()))
	{
		return System->SystemDescription;
	}
	return FText::GetEmpty();
}

void FGorgeousCoreLibraryParticipant::OnCategoryActionExecuted(FName CategoryId, FName ActionId)
{
	if (CategoryId == "CoreSystems")
	{
		if (ActionId == "ConfigureSettings")
		{
			UPackage* MemoryPackage = FindPackage(nullptr, TEXT("/Temp/GorgeousCore/OfflineSystems"));
			if (MemoryPackage)
			{
				if (UGorgeousPersistentDataConfig_DA* SettingsAsset = FindObject<UGorgeousPersistentDataConfig_DA>(MemoryPackage, TEXT("GorgeousPersistentData")))
				{
					FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

					FDetailsViewArgs DetailsViewArgs;
					DetailsViewArgs.bAllowSearch = true;
					DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
					DetailsViewArgs.bHideSelectionTip = true;

					TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
					DetailsView->SetObject(SettingsAsset);

					TSharedRef<SWindow> SettingsWindow = SNew(SWindow)
						.Title(NSLOCTEXT("GorgeousCore", "APIConfiguration", "Gorgeous API Configuration"))
						.ClientSize(FVector2D(600, 400))
						.SupportsMaximize(false)
						.SupportsMinimize(false)
						[
							DetailsView
						];

					FSlateApplication::Get().AddWindow(SettingsWindow);
				}
			}
		}
		else if (ActionId == "Recompile")
		{
			if (UGorgeousRecompilationHelper* RecompHelper = UGorgeousRecompilationHelper::GetSingleton())
			{
				RecompHelper->TriggerGorgeousRecompilation();
			}
		}
	}
}

void FGorgeousCoreLibraryParticipant::RefreshOfflineCache()
{
	TransientAssets.Empty();

	if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
	{
		TArray<FGorgeousOfflineSystemCacheEntry> CacheEntries = Helper->GetOfflineSystemCache();

		// We register a virtual mount point in /Memory so the Asset Registry indexes the items
		// without throwing "Save Package" prompts when the editor closes!
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().AddPath(TEXT("/Temp/GorgeousCore/OfflineSystems"));

		UPackage* MemoryPackage = CreatePackage(TEXT("/Temp/GorgeousCore/OfflineSystems"));

		for (const FGorgeousOfflineSystemCacheEntry& Entry : CacheEntries)
		{
			FString AssetName = FString::Printf(TEXT("SYS_%s"), *Entry.SystemId);
			// Dropping RF_Transient so the Asset Registry doesn't instantly reject it
			UGorgeousInstallableSystem_DA* NewSystem = NewObject<UGorgeousInstallableSystem_DA>(MemoryPackage, FName(*AssetName), RF_Public | RF_Standalone);

			NewSystem->SystemId = Entry.SystemId;
			NewSystem->TargetPluginName = Entry.TargetPluginName;
			NewSystem->SystemName = FText::FromString(Entry.DisplayName);
			NewSystem->SystemDescription = FText::FromString(Entry.Description);
			NewSystem->Version = Entry.Version;
			NewSystem->DownloadUrl = Entry.DownloadUrl;
			NewSystem->SourcePaths = Entry.SourcePaths;
			NewSystem->ContentPaths = Entry.ContentPaths;
			NewSystem->bIsPreShipped = Entry.bIsCoreSystem;
			NewSystem->bIsInstalled = NewSystem->CheckIsInstalled();

			TransientAssets.Add(NewSystem);
			FAssetRegistryModule::AssetCreated(NewSystem);
		}

		// Spawn a memory instance of the persistent data config so it appears in the Library UI
		UGorgeousPersistentDataConfig_DA* SettingsAsset = NewObject<UGorgeousPersistentDataConfig_DA>(MemoryPackage, TEXT("GorgeousPersistentData"), RF_Public | RF_Standalone);
		
		// Map the properties from the live Helper JSON data
		SettingsAsset->ValidationCount = Helper->GetSystemValidationCount();
		SettingsAsset->ValidationInterval = Helper->GetSystemValidationInterval();
		SettingsAsset->bHasRunInitialValidation = Helper->HasRunInitialValidation();
		SettingsAsset->bForceDevMode = Helper->GetForceDevMode();
		SettingsAsset->OfflineSystemCache = CacheEntries;
		SettingsAsset->PluginUpdateCache = Helper->GetPluginUpdateCache();
		
		TransientAssets.Add(SettingsAsset);
		FAssetRegistryModule::AssetCreated(SettingsAsset);
	}
}
