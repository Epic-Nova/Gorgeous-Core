// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Events - Events functionality provider              |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "Validators/GorgeousGeneralSystemValidator.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#include "ModuleCore/GorgeousSystemDeveloperSettings.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "EditorAssetLibrary.h"
#include "UnrealEdMisc.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "Engine/Blueprint.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/DataValidation.h"
#include "Async/Async.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
//<-------------------------------------------------------------------------->

//@todo: as this behaviour is relatively complex, we plan this to be a standalone toolit that can perform various validation and fixing tasks related to gorgeous systems. For now, we will keep it as a general validator to ensure the core system setup is always correct, but we may want to split it up later on if it grows too much or has distinct areas of responsibility.
//@todo: handle edge case handling outside GorgeousCore

TSet<FName> UGorgeousGeneralSystemValidator::ProcessedAssets = TSet<FName>();

TSet<FString> UGorgeousGeneralSystemValidator::CreatedSystems = TSet<FString>();

bool UGorgeousGeneralSystemValidator::bPluginSystemsDiscovered = false;
FTSTicker::FDelegateHandle UGorgeousGeneralSystemValidator::AsyncValidationTickerHandle;
TArray<FAssetData> UGorgeousGeneralSystemValidator::AsyncValidationQueue;
TSharedPtr<SNotificationItem> UGorgeousGeneralSystemValidator::AsyncProgressNotification;
int32 UGorgeousGeneralSystemValidator::TotalAsyncAssets = 0;

// Tracks the last time a PDA was validated to suppress duplicate/rapid re-validation
static TMap<FName, double> GLastPDAValidationTimes;

//=============================================================================
// UGorgeousGeneralSystemValidator Implementation
//=============================================================================

UGorgeousGeneralSystemValidator::UGorgeousGeneralSystemValidator()
{
	UGT_EditorLogging_FL::RegisterLogHyperlinkAction(
		StaticClass(),
		FName("GT.GeneralSystemValidator.RegisterAsset"),
		FName("HandleRegisterAssetRegistryEntry"));

	const FAssetRegistryModule& ARM =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	ARM.Get().OnAssetAdded().AddUObject(
		this,
		&UGorgeousGeneralSystemValidator::HandleAssetAdded
	);

	FCoreDelegates::OnPostEngineInit.AddLambda([this]()
	{
		if (!GEditor)
			return;

		// Discover all Gorgeous plugins and register their systems
		DiscoverAndRegisterGorgeousPluginSystems();

		// If no known gorgeous plugins yet, start a short ticker to wait for registrations
		UGorgeousPluginHelper* PluginHelper = GetMutableDefault<UGorgeousPluginHelper>();
		TSet<FName> Known = PluginHelper ? PluginHelper->GetKnownGorgeousPlugins() : TSet<FName>();
		if (Known.Num() == 0 && !SystemPathsTickerHandle.IsValid())
		{
			SystemPathsTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UGorgeousGeneralSystemValidator::TickUpdateSystemPaths),
				2.0f);
		}

		GEditor->GetTimerManager()->SetTimer(
			ScanTimerHandle,
			this,
			&UGorgeousGeneralSystemValidator::ScanAndRecreateMissingPDAs,
			1.0f,
			false
		);
	});
}

UGorgeousGeneralSystemValidator::~UGorgeousGeneralSystemValidator()
{
	UGT_EditorLogging_FL::UnregisterLogHyperlinkAction("GT.GeneralSystemValidator.RegisterAsset");
}

void UGorgeousGeneralSystemValidator::DiscoverAndRegisterGorgeousPluginSystems()
{
	if (bPluginSystemsDiscovered || !GEditor)
	{
		return;
	}

	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = ARM.Get();

	// Get Asset Manager settings
	UAssetManagerSettings* AssetManagerSettings = GetMutableDefault<UAssetManagerSettings>();
	if (!AssetManagerSettings)
	{
		return;
	}

	const FPrimaryAssetType SystemType = TEXT("GorgeousBlueprintSystems");

	// Create a set of already registered paths for quick lookup
	TSet<FString> RegisteredSearchPaths;
	for (const FPrimaryAssetTypeInfo& TypeInfo : AssetManagerSettings->PrimaryAssetTypesToScan)
	{
		if (TypeInfo.PrimaryAssetType == SystemType.GetName())
		{
			for (const FDirectoryPath& Path : TypeInfo.GetDirectories())
			{
				RegisteredSearchPaths.Add(Path.Path);
			}
		}
	}

	bool bSettingsModified = false;

	// Use IPluginManager to find all Gorgeous plugins (broad detection)
	IPluginManager& PluginManager = IPluginManager::Get();
	TArray<TSharedRef<IPlugin>> AllPlugins = PluginManager.GetDiscoveredPlugins();

	for (const TSharedRef<IPlugin>& Plugin : AllPlugins)
	{
		const FString PluginName = Plugin->GetName();
		
		// Check if this is a Gorgeous plugin
		if (!PluginName.StartsWith(TEXT("Gorgeous")) && !PluginName.Contains(TEXT("Gorgeous")))
		{
			continue;
		}

		// Convert plugin name to content path format
		const FString PluginContentPath = FString::Printf(TEXT("/%s"), *PluginName);
		const FString SystemsPath = PluginContentPath / TEXT("Systems");

		if (RegisteredSearchPaths.Contains(SystemsPath))
		{
			continue;
		}

		// Check if the directory exists in the Asset Registry / scan results
		// We use a broad filter to include even unknown assets in that path
		TArray<FAssetData> SystemAssets;
		FARFilter Filter;
		Filter.PackagePaths.Add(*SystemsPath);
		Filter.bRecursivePaths = true;
		AssetRegistry.GetAssets(Filter, SystemAssets);

		// If discovery is successful, we register the path
		// We register even if 0 assets are currently found to ensure future-proofing
		// especially since AssetRegistry might be still scanning.
		
		// Find or create the GorgeousBlueprintSystems entry
		int32 TypeIndex = INDEX_NONE;
		for (int32 i = 0; i < AssetManagerSettings->PrimaryAssetTypesToScan.Num(); ++i)
		{
			if (AssetManagerSettings->PrimaryAssetTypesToScan[i].PrimaryAssetType == SystemType.GetName())
			{
				TypeIndex = i;
				break;
			}
		}

		if (TypeIndex == INDEX_NONE)
		{
			// Create new entry if it doesn't exist
			FPrimaryAssetTypeInfo NewInfo;
			NewInfo.PrimaryAssetType = SystemType;
			NewInfo.SetAssetBaseClass(UGeneralSystemConfiguration_PDA::StaticClass());
			NewInfo.bHasBlueprintClasses = false; // Primary Data Assets are not blueprint classes

			FDirectoryPath DirPath;
			DirPath.Path = SystemsPath;
			NewInfo.GetDirectories().Add(DirPath);

			AssetManagerSettings->Modify();
			AssetManagerSettings->PrimaryAssetTypesToScan.Add(NewInfo);
			RegisteredSearchPaths.Add(SystemsPath);
			bSettingsModified = true;

			GT_I_LOG("GT.GeneralSystemValidator",
				TEXT("✓ Created GorgeousBlueprintSystems entry with path for plugin '%s': %s"),
				*PluginName,
				*SystemsPath);
		}
		else
		{
			// Add to existing entry
			FPrimaryAssetTypeInfo& TypeInfo = AssetManagerSettings->PrimaryAssetTypesToScan[TypeIndex];
			
			FDirectoryPath DirPath;
			DirPath.Path = SystemsPath;
			TypeInfo.GetDirectories().Add(DirPath);

			AssetManagerSettings->Modify();
			RegisteredSearchPaths.Add(SystemsPath);
			bSettingsModified = true;

			GT_I_LOG("GT.GeneralSystemValidator",
				TEXT("✓ Added path to GorgeousBlueprintSystems for plugin '%s': %s"),
				*PluginName,
				*SystemsPath);
		}
	}

	bPluginSystemsDiscovered = true;

	// Save settings if modified
	if (bSettingsModified)
	{
		AssetManagerSettings->TryUpdateDefaultConfigFile();
		AssetManagerSettings->SaveConfig(CPF_Config, *AssetManagerSettings->GetDefaultConfigFilename());

		// Reinitialize Asset Manager so changes take effect in-memory immediately
		if (UAssetManager::IsInitialized())
		{
			UAssetManager::Get().ReinitializeFromConfig();
		}

		GT_I_LOG("GT.GeneralSystemValidator",
			TEXT("Asset Manager settings updated with plugin system paths. Validation scope expanded."));

		// Prompt user to restart if we have a lot of changes or if it's the first time
		// (optional, since ReinitializeFromConfig handles the immediate in-memory state)
	}
	else
	{
		GT_I_LOG("GT.GeneralSystemValidator",
			TEXT("No new system paths to register. Found %d plugins."), AllPlugins.Num());
	}
}

bool UGorgeousGeneralSystemValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (!InObject)
	{
		return false;
	}

	// Only validate assets within directories registered for Gorgeous systems
	const FString PackagePath = InAssetData.PackageName.ToString();
	const TArray<FString> ScannedDirs = GetGorgeousSystemDirectories();
	
	bool bIsWithinScannedDir = false;
	for (const FString& Dir : ScannedDirs)
	{
		// Ensure directory path has a trailing slash for accurate prefix matching
		FString ScaledDir = Dir;
		if (!ScaledDir.EndsWith(TEXT("/")))
		{
			ScaledDir += TEXT("/");
		}

		if (PackagePath.StartsWith(ScaledDir))
		{
			bIsWithinScannedDir = true;
			break;
		}
	}

	if (!bIsWithinScannedDir)
	{
		return false;
	}
	
	if (InObject->IsA<UGeneralSystemConfiguration_PDA>())
	{
		return true;
	}

	// validate relevant system components (so we can detect missing PDA) - runtime objects
	if (InObject->GetClass()->IsChildOf(UGeneralSystem_AC::StaticClass()))
	{
		if (const FString Name = InObject->GetName(); 
			IsSystemComponent(Name, 0)) // check all system component types
		{
			return true;
		}
	}

	return false;
}

EDataValidationResult UGorgeousGeneralSystemValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	// If this is a PDA asset, validate it (and ensure AssetManager registration)
	if (UGeneralSystemConfiguration_PDA* PDA = Cast<UGeneralSystemConfiguration_PDA>(InAsset))
	{
		ValidatePDA(PDA, Context);

		if (IsSystemAssetManagerRegistered())
		{
			return EDataValidationResult::Valid;
		}

		const UAssetManager& AssetManager = UAssetManager::Get();

		if (const FPrimaryAssetId ExistingId = AssetManager.GetPrimaryAssetIdForObject(PDA); 
			!ExistingId.IsValid())
		{
			UGT_EditorLogging_FL::LogMessageWithActionHyperlink(
				FString::Printf(
					TEXT("System '%s' is not registered in Asset Manager."),
					*PDA->GetPathName()
				),
				"GT.Systems.GeneralSystem.AssetRegistryEntry",
				Logging_Error,
				FName("GT.GeneralSystemValidator.RegisterAsset"),
				PDA->GetPathName(),
				"Fix Asset Manager Config"
			);

			return EDataValidationResult::Invalid;
		}

		return EDataValidationResult::Valid;
	}

	// If this is a system component (blueprint), ensure its PDA exists and validate it.
	if (UBlueprint* BP = Cast<UBlueprint>(InAsset); BP && BP->GeneratedClass && BP->GeneratedClass->IsChildOf(UGeneralSystem_AC::StaticClass()))
	{
		UClass* ComponentClass = BP->GeneratedClass;
		UGeneralSystemConfiguration_PDA* PDA = FindExistingPDA(ComponentClass);
		if (!PDA)
		{
			PDA = CreatePDA(ComponentClass);
			if (!PDA)
				return EDataValidationResult::Valid;
		}

		ValidatePDA(PDA, Context);
		return EDataValidationResult::Valid;
	}

	return EDataValidationResult::Valid;
}

void UGorgeousGeneralSystemValidator::BeginDestroy()
{
	if (GEditor)
	{
		if (UImportSubsystem* ImportSubsystem =
			GEditor->GetEditorSubsystem<UImportSubsystem>())
		{
			ImportSubsystem->OnAssetPostImport.RemoveAll(this);
		}
	}

	FAssetRegistryModule* ARM =
		FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry");

	if (ARM)
	{
		ARM->Get().OnAssetAdded().RemoveAll(this);
	}

	if (SystemPathsTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(SystemPathsTickerHandle);
		SystemPathsTickerHandle.Reset();
	}

	Super::BeginDestroy();
}

void UGorgeousGeneralSystemValidator::HandleRegisterAssetRegistryEntry(const FString& Payload)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	
	const FPrimaryAssetType SystemType = TEXT("GorgeousBlueprintSystems");

	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();

	if (!Settings)
	{
		GT_E_LOG("GT.GeneralSystemValidator", TEXT("Failed to access AssetManager settings."));
		return;
	}

	bool bAlreadyExists = false;

	for (const FPrimaryAssetTypeInfo& Info : Settings->PrimaryAssetTypesToScan)
	{
		if (Info.PrimaryAssetType == SystemType)
		{
			bAlreadyExists = true;
			break;
		}
	}

	if (!bAlreadyExists)
	{
		FPrimaryAssetTypeInfo NewInfo;
		NewInfo.PrimaryAssetType = SystemType;
		NewInfo.SetAssetBaseClass(UGeneralSystemConfiguration_PDA::StaticClass());
		NewInfo.bHasBlueprintClasses = true;

		// Add all known gorgeous plugin system directories so AssetManager scans all plugins
		UGorgeousPluginHelper* PluginHelper = GetMutableDefault<UGorgeousPluginHelper>();
		TSet<FName> Known = PluginHelper ? PluginHelper->GetKnownGorgeousPlugins() : TSet<FName>();

		if (Known.Num() == 0)
		{
			FDirectoryPath Dir;
			Dir.Path = TEXT("/GorgeousCore/Systems");
			NewInfo.GetDirectories().Add(Dir);
		}
		else
		{
			for (const FName& Plugin : Known)
			{
				FDirectoryPath Dir;
				Dir.Path = FString::Printf(TEXT("/%s/Systems"), *Plugin.ToString());
				NewInfo.GetDirectories().Add(Dir);
			}
		}

		Settings->PrimaryAssetTypesToScan.Add(NewInfo);

		Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());

		GT_I_LOG("GT.GeneralSystemValidator",
			TEXT("Added PrimaryAssetType '%s' to config."),
			*SystemType.ToString());
	}

	AssetManager.ReinitializeFromConfig();

	GT_S_LOG("GT.GeneralSystemValidator",
		TEXT("Asset Manager reinitialized."));

	FMessageDialog::Open(
		EAppMsgType::Ok,
		FText::FromString(TEXT("Asset Manager updated and refreshed."))
	);
}


// Ticker callback: wait until other gorgeous plugins register, then run discovery/registration.
bool UGorgeousGeneralSystemValidator::TickUpdateSystemPaths(float)
{
	UGorgeousPluginHelper* PluginHelper = GetMutableDefault<UGorgeousPluginHelper>();
	if (!PluginHelper)
		return true; // keep ticking

	const TSet<FName> Known = PluginHelper->GetKnownGorgeousPlugins();
	if (Known.Num() == 0)
	{
		return true; // keep ticking
	}

	// Known plugins discovered - run discovery/registration and stop ticking
	DiscoverAndRegisterGorgeousPluginSystems();

	if (SystemPathsTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(SystemPathsTickerHandle);
		SystemPathsTickerHandle.Reset();
	}

	return false; // stop ticker
}

void UGorgeousGeneralSystemValidator::ScanAndRecreateMissingPDAs()
{
	// Prevent multiple concurrent scans
	if (AsyncValidationTickerHandle.IsValid() || AsyncValidationQueue.Num() > 0)
	{
		return;
	}

	// Ensure discovery has run first so we have the directory list
	DiscoverAndRegisterGorgeousPluginSystems();

	TArray<FString> ScannedDirs = GetGorgeousSystemDirectories();
	if (ScannedDirs.Num() == 0)
	{
		return;
	}

	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> FoundAssets;
	
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	for (const FString& Dir : ScannedDirs)
	{
		Filter.PackagePaths.Add(FName(*Dir));
	}
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;

	// Must be on Game Thread
	ARM.Get().GetAssets(Filter, FoundAssets);

	// Immediately filter to actual Gorgeous Systems to get an accurate count
	AsyncValidationQueue.Empty();
	for (const FAssetData& Asset : FoundAssets)
	{
		// Only queue if it's potentially a system component
		// We can't do IsChildOf here easily without loading, but we can check naming 
		// or tags if they exist. For now, let's keep it simple but accurate.
		const FString Name = Asset.AssetName.ToString();
		if (IsSystemComponent(Name, 0))
		{
			AsyncValidationQueue.Add(Asset);
		}
	}

	TotalAsyncAssets = AsyncValidationQueue.Num();

	if (TotalAsyncAssets > 0)
	{
		// Show notification in bottom right
		//@TODO: only validate systems every 10 startups or so & fix the lag
		FNotificationInfo Info(NSLOCTEXT("GT.GeneralSystemValidator", "AsyncScanProgress", "Verifying Gorgeous Systems..."));
		Info.bFireAndForget = false;
		Info.FadeOutDuration = 2.0f;
		Info.ExpireDuration = 2.0f;
		
		AsyncProgressNotification = FSlateNotificationManager::Get().AddNotification(Info);
		if (AsyncProgressNotification.IsValid())
		{
			AsyncProgressNotification->SetCompletionState(SNotificationItem::CS_Pending);
		}

		AsyncValidationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([this](float DeltaTime) { return TickAsyncValidation(DeltaTime); })
		);
	}
}

bool UGorgeousGeneralSystemValidator::TickAsyncValidation(float)
{
	if (AsyncValidationQueue.Num() == 0)
	{
		if (AsyncProgressNotification.IsValid())
		{
			AsyncProgressNotification->SetText(NSLOCTEXT("GT.GeneralSystemValidator", "AsyncScanDone", "Gorgeous Systems Verified"));
			AsyncProgressNotification->SetCompletionState(SNotificationItem::CS_Success);
			AsyncProgressNotification->ExpireAndFadeout();
		}
		return false;
	}

	// Process batch of assets per frame to avoid stalling UI
	constexpr int32 BatchSize = 5;
	int32 ProcessedCount = 0;

	while (AsyncValidationQueue.Num() > 0 && ProcessedCount < BatchSize)
	{
		const FAssetData& Asset = AsyncValidationQueue.Pop();
		ProcessedCount++;

		UObject* Obj = Asset.GetAsset();
		if (!Obj)
			continue;

		UBlueprint* BP = Cast<UBlueprint>(Obj);
		if (!BP || !BP->GeneratedClass)
			continue;

		UClass* Class = BP->GeneratedClass;
		if (!Class->IsChildOf(UGeneralSystem_AC::StaticClass()))
			continue;

		const FString Name = Class->GetName();
		const bool bIsManager = IsSystemComponent(Name, 1);
		const bool bIsInteractor = IsSystemComponent(Name, 2);

		if (!bIsManager && !bIsInteractor)
			continue;

		if (!FindExistingPDA(Class))
		{
			CreatePDA(Class);
		}
	}

	// Update notification progress
	if (AsyncProgressNotification.IsValid() && TotalAsyncAssets > 0)
	{
		const float Progress = 1.0f - ((float)AsyncValidationQueue.Num() / (float)TotalAsyncAssets);
		// Notification items usually don't have built-in progress bars if not customized,
		// but we can update the text periodically or just keep it pending until done.
		FFormatNamedArguments Args;
		Args.Add("Current", FText::AsNumber(TotalAsyncAssets - AsyncValidationQueue.Num()));
		Args.Add("Total", FText::AsNumber(TotalAsyncAssets));
		AsyncProgressNotification->SetText(FText::Format(NSLOCTEXT("GT.GeneralSystemValidator", "AsyncScanProgressCount", "Verifying Gorgeous Systems ({Current}/{Total})..."), Args));
	}

	return true; // keep ticking
}

bool UGorgeousGeneralSystemValidator::IsSystemAssetManagerRegistered() const
{
	const TArray<FString> ScannedDirs = GetGorgeousSystemDirectories();
	return ScannedDirs.Num() > 0;
}

TArray<FString> UGorgeousGeneralSystemValidator::GetGorgeousSystemDirectories() const
{
	TArray<FString> Results;
	const FPrimaryAssetType Type(TEXT("GorgeousBlueprintSystems"));
	
	const UAssetManagerSettings* Settings = GetDefault<UAssetManagerSettings>();
	if (!Settings)
		return Results;

	for (const FPrimaryAssetTypeInfo& Info : Settings->PrimaryAssetTypesToScan)
	{
		if (Info.PrimaryAssetType == Type.GetName())
		{
			for (const FDirectoryPath& Dir : Info.GetDirectories())
			{
				Results.AddUnique(Dir.Path);
			}
		}
	}

	// Fallback to GorgeousCore if everything else is missing
	if (Results.Num() == 0)
	{
		Results.Add(TEXT("/GorgeousCore/Systems"));
	}

	return Results;
}

bool UGorgeousGeneralSystemValidator::IsSystemComponent(const FString Name, const uint8 CheckMode) const
{
	TArray<FString> AuthorativeComponentNames = {
		TEXT("Manager"),
		TEXT("Handler"),
		TEXT("Handling")
	};
	
	TArray<FString> InteractorComponentNames = {
		TEXT("Interactor")
	};
	
	for (const FString& Keyword : AuthorativeComponentNames)
	{
		if (Name.Contains(Keyword))
		{
			if (CheckMode == 0 || CheckMode == 1)
			{
				return true;
			}
			break; // if it contains an authoritative keyword, we can skip checking interactor keywords
		}
	}
	
	for (const FString& Keyword : InteractorComponentNames)
	{
		if (Name.Contains(Keyword))
		{
			if (CheckMode == 0 || CheckMode == 2)
			{
				return true;
			}
			break;
		}
	}
	
	return false;
}

UClass* UGorgeousGeneralSystemValidator::FindSystemDefaultComponent(const FString& PDA_SystemRoot, const bool bIsManager)
{
	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Stage 1: collect classes that live inside this system folder
	TArray<FAssetData> LocalAssets;
	AssetRegistry.Get().GetAssetsByPath(FName(*PDA_SystemRoot), LocalAssets, true);

	TArray<UClass*> SystemClasses;
	TArray<UClass*> LocalRoleMatches;

	for (const FAssetData& Asset : LocalAssets)
	{
		if (Asset.AssetClassPath != UBlueprint::StaticClass()->GetClassPathName())
			continue;

		const FString GeneratedClassPath = Asset.GetTagValueRef<FString>("GeneratedClass");
		UClass* GeneratedClass = nullptr;

		if (!GeneratedClassPath.IsEmpty())
		{
			GeneratedClass = LoadObject<UClass>(nullptr, *GeneratedClassPath);
		}
		else
		{
			UBlueprint* BP = Cast<UBlueprint>(Asset.GetAsset());
			if (BP && BP->GeneratedClass)
				GeneratedClass = BP->GeneratedClass;
		}

		if (!GeneratedClass)
			continue;

		if (!GeneratedClass->IsChildOf(UGeneralSystem_AC::StaticClass()))
			continue;

		SystemClasses.AddUnique(GeneratedClass);

		const FString FullClassName = GeneratedClass->GetName();
		FString ClassName = FullClassName;
		// Remove _C suffix from blueprint class names for keyword matching
		ClassName.RemoveFromEnd(TEXT("_C"));

		const bool bMatchesRole =
			(bIsManager && IsSystemComponent(ClassName, 1)) ||
			(!bIsManager && IsSystemComponent(ClassName, 2));

		if (bMatchesRole)
		{
			LocalRoleMatches.AddUnique(GeneratedClass);
		}
	}

	// Prefer direct matches inside the same system folder that have the system name in their name
	if (LocalRoleMatches.Num() > 0)
	{
		FString SystemDirName;
		if (PDA_SystemRoot.Split(TEXT("/"), nullptr, &SystemDirName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			// SystemDirName is now the last part of the path (e.g., "Test")
		}

		for (UClass* Candidate : LocalRoleMatches)
		{
			FString ClassName = Candidate->GetName();
			ClassName.RemoveFromEnd(TEXT("_C"));

			if (SystemDirName.Len() > 0 && ClassName.Contains(SystemDirName))
			{
				return Candidate;
			}
		}

		return LocalRoleMatches[0];
	}

	// Stage 2: allow derived classes located outside the folder, but only if they
	// derive from one of the system classes discovered above.
	if (SystemClasses.Num() > 0)
	{
		TArray<FAssetData> AllBlueprints;
		AssetRegistry.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AllBlueprints, true);

		TArray<UClass*> DerivedMatches;

		for (const FAssetData& Asset : AllBlueprints)
		{
			if (Asset.AssetClassPath != UBlueprint::StaticClass()->GetClassPathName())
				continue;

			const FString GeneratedClassPath = Asset.GetTagValueRef<FString>("GeneratedClass");
			UClass* CandidateClass = nullptr;

			if (!GeneratedClassPath.IsEmpty())
			{
				CandidateClass = LoadObject<UClass>(nullptr, *GeneratedClassPath);
			}
			else
			{
				UBlueprint* BP = Cast<UBlueprint>(Asset.GetAsset());
				if (BP && BP->GeneratedClass)
					CandidateClass = BP->GeneratedClass;
			}

			if (!CandidateClass)
				continue;

			if (!CandidateClass->IsChildOf(UGeneralSystem_AC::StaticClass()))
				continue;

			bool bChildOfSystem = false;
			for (UClass* SysCls : SystemClasses)
			{
				if (CandidateClass->IsChildOf(SysCls))
				{
					bChildOfSystem = true;
					break;
				}
			}

			if (!bChildOfSystem)
				continue;

			const FString Name = CandidateClass->GetName();
			const bool bMatchesRole =
				(bIsManager && IsSystemComponent(Name, 1)) ||
				(!bIsManager && IsSystemComponent(Name, 2));

			if (bMatchesRole)
			{
				DerivedMatches.AddUnique(CandidateClass);
			}
		}

		if (DerivedMatches.Num() > 0)
		{
			FString SystemName;
			if (PDA_SystemRoot.Split(TEXT("/Systems/"), nullptr, &SystemName))
			{
				SystemName.Split(TEXT("/"), &SystemName, nullptr);
			}

			for (UClass* Candidate : DerivedMatches)
			{
				if (SystemName.Len() > 0 && Candidate->GetName().Contains(SystemName))
				{
					return Candidate;
				}
			}

			return DerivedMatches[0];
		}
	}

	return nullptr;
}

void UGorgeousGeneralSystemValidator::HandleAssetAdded(const FAssetData& AssetData)
{
	if (!AssetData.IsValid() || ProcessedAssets.Contains(AssetData.PackageName))
		return;

	ProcessedAssets.Add(AssetData.PackageName);

	if (AssetData.AssetClassPath != UBlueprint::StaticClass()->GetClassPathName())
		return;

	FTSTicker::GetCoreTicker().AddTicker(
	FTickerDelegate::CreateLambda([WeakThis = TWeakObjectPtr<UGorgeousGeneralSystemValidator>(this), AssetData](float)
	{
		if (!WeakThis.IsValid())
			return false;

		WeakThis->OnSafeProcessAsset(nullptr, AssetData);
		return false;
	}),0.5f);
}

void UGorgeousGeneralSystemValidator::OnSafeProcessAsset(UWorld* World, FAssetData AssetData)
{
	UObject* Asset = AssetData.GetAsset();

	if (!IsValid(Asset))
		return;

	UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
	if (!Blueprint || !Blueprint->GeneratedClass)
		return;

	UClass* ComponentClass = Blueprint->GeneratedClass;

	if (!ComponentClass->IsChildOf(UGeneralSystem_AC::StaticClass()))
		return;
	
	const FString ClassName = ComponentClass->GetName();

	const bool bIsManager    = IsSystemComponent(ClassName, 1);
	const bool bIsInteractor = IsSystemComponent(ClassName, 2);

	if (!bIsManager && !bIsInteractor)
		return;

	UGeneralSystemConfiguration_PDA* PDA = FindExistingPDA(ComponentClass);
	if (!PDA)
	{
		PDA = CreatePDA(ComponentClass);
		
		if (!PDA)
			return;
	}

	FDataValidationContext Context;
	ValidatePDA(PDA, Context);
}

void UGorgeousGeneralSystemValidator::ValidatePDA(UGeneralSystemConfiguration_PDA* PDA, FDataValidationContext& Context)
{
	if (!IsValid(PDA) || PDA->HasAnyFlags(RF_ClassDefaultObject))
		return;

	// Suppress duplicate validations for the same PDA within a short time window
	const FName PDAKey(*PDA->GetPathName());
	const double Now = FPlatformTime::Seconds();
	if (double* Last = GLastPDAValidationTimes.Find(PDAKey))
	{
		constexpr double ValidationSuppressSeconds = 5.0; // suppress repeats within 5s
		if ((Now - *Last) < ValidationSuppressSeconds)
		{
			GT_I_LOG("GT.GeneralSystemValidator",
				TEXT("Skipping duplicate validation for '%s' (last validated %.2fs ago)."),
				*PDA->GetPathName(), Now - *Last);
			return;
		}
	}

	GLastPDAValidationTimes.Add(PDAKey, Now);

	const FString PDA_SystemRoot = GetSystemRootPath(PDA->GetPathName());
	
	bool bChanged = false;

	auto FixComponent = [&](TSubclassOf<UGeneralSystem_AC>& Ref, const bool bExpectManager)
	{
		const TSubclassOf<UGeneralSystem_AC> Default = FindSystemDefaultComponent(PDA_SystemRoot, bExpectManager);
		
		// missing or CDO - attempt to assign default
		if (!Ref || Ref->HasAnyFlags(RF_ClassDefaultObject))
		{
			if (Default)
			{
				Ref = Default;
				bChanged = true;
				return;
			}
			else
			{
				// No default found and currently null - this is a validation error
				const FText ErrorMsg = FText::Format(
					NSLOCTEXT("GT.GeneralSystemValidator", "MissingComponentError", "General System PDA '{0}' is missing its {1} component and no default could be discovered in '{2}'."),
					FText::FromString(PDA->GetName()),
					FText::FromString(bExpectManager ? TEXT("Authorative (Manager)") : TEXT("Interactor")),
					FText::FromString(PDA_SystemRoot)
				);
				Context.AddError(ErrorMsg);
				return;
			}
		}

		// If we actually found a default, enforce inheritance
		if (Default)
		{
			// Is it already the default or a derivative of it?
			if (Ref == Default || Ref->IsChildOf(Default))
			{
				return;
			}

			// secondary safety: even without a perfect default match, strictly enforce the role keywords
			const FString Name = Ref->GetName();
			const bool bIsCorrectRole = bExpectManager ? IsSystemComponent(Name, 1) : IsSystemComponent(Name, 2);

			if (bIsCorrectRole)
			{
				// It has the correct keywords, and either discovery failed or it's a valid custom match
			}
			else
			{
				// Not related and wrong role - resetting to default
				GT_W_LOG("GT.GeneralSystemValidator",
					TEXT("PDA '%s' component '%s' is not related to the system default '%s' - resetting to default."),
					*PDA->GetName(), *Ref->GetName(), *Default->GetName());
				Ref = Default;
				bChanged = true;
				return;
			}

			// If we got here, it's a role match but NOT an inheritance match - we still prefer the system default
			GT_W_LOG("GT.GeneralSystemValidator",
				TEXT("PDA '%s' component '%s' is not a child of the system default '%s' - resetting to default to ensure system integrity."),
				*PDA->GetName(), *Ref->GetName(), *Default->GetName());
			Ref = Default;
			bChanged = true;
		}
		else
		{
			// No default found - strictly enforce role keywords on the manual assignment
			const FString Name = Ref->GetName();
			const bool bIsCorrectRole = bExpectManager ? IsSystemComponent(Name, 1) : IsSystemComponent(Name, 2);

			if (!bIsCorrectRole)
			{
				const FText ErrorMsg = FText::Format(
					NSLOCTEXT("GT.GeneralSystemValidator", "InvalidRoleError", "PDA '{0}' component '{1}' does not match the required {2} role keywords and no default was found."),
					FText::FromString(PDA->GetName()),
					FText::FromString(Ref->GetName()),
					FText::FromString(bExpectManager ? TEXT("Manager") : TEXT("Interactor"))
				);
				Context.AddError(ErrorMsg);
				
				Ref = nullptr;
				bChanged = true;
			}
		}
	};

	// Validate both components
	FixComponent(PDA->AuthorativeComponent, true);
	FixComponent(PDA->InteractorComponent, false);

	// Save only if something actually changed
	if (bChanged)
	{
		if (PDA && PDA->IsValidLowLevelFast() && PDA->GetOutermost())
		{
			PDA->MarkPackageDirty();
			UEditorAssetLibrary::SaveLoadedAsset(PDA);
		}
	}
}

FString UGorgeousGeneralSystemValidator::GetSystemRootPath(const FString& Path)
{
	FString AfterSystems;
	if (!Path.Split(TEXT("/Systems/"), nullptr, &AfterSystems))
	{
		return TEXT("");
	}

	FString SystemName;
	if (!AfterSystems.Split(TEXT("/"), &SystemName, nullptr))
	{
		SystemName = AfterSystems;
	}

	FString Prefix;
	if (!Path.Split(TEXT("/Systems/"), &Prefix, nullptr))
	{
		return TEXT("");
	}

	return Prefix + TEXT("/Systems/") + SystemName;
}

FString UGorgeousGeneralSystemValidator::ConstructPDASystemName(const UClass* InComponentClass, FString& OutSystemName)
{
	const FString PackageName = InComponentClass->GetOutermost()->GetName();

	FString AfterSystems;
	if (!PackageName.Split(TEXT("/Systems/"), nullptr, &AfterSystems))
	{
		return "";
	}

	FString SystemName;
	if (!AfterSystems.Split(TEXT("/"), &SystemName, nullptr))
	{
		SystemName = AfterSystems; // fallback if already root
	}
	
	OutSystemName = SystemName;
	return FString::Printf(TEXT("PDA_%s"), *SystemName);
}

// Helper: Find an Extension Resource Guard asset that matches the given system name.
static UGorgeousExtensionResourceGuard* FindGuardForSystemName(const FString& SystemName)
{
	if (SystemName.IsEmpty())
	{
		return nullptr;
	}

	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> GuardAssets;
	ARM.Get().GetAssetsByClass(UGorgeousExtensionResourceGuard::StaticClass()->GetClassPathName(), GuardAssets, true);

	for (const FAssetData& Asset : GuardAssets)
	{
		if (!Asset.IsValid())
			continue;

		UObject* Obj = Asset.GetAsset();
		if (!Obj)
			continue;

		UGorgeousExtensionResourceGuard* Guard = Cast<UGorgeousExtensionResourceGuard>(Obj);
		if (!Guard)
			continue;

		if (!Guard->SystemIdentifier.IsNone() && Guard->SystemIdentifier == FName(*SystemName))
		{
			return Guard;
		}
	}

	return nullptr;
}

UGeneralSystemConfiguration_PDA* UGorgeousGeneralSystemValidator::CreatePDA(const UClass* ComponentClass)
{
	if (!ComponentClass)
		return nullptr;

	FString SystemName;
	const FString DesiredAssetName = ConstructPDASystemName(ComponentClass, SystemName);

	// Prefer explicit developer settings override first
	if (SystemName.Len() > 0)
	{
		if (UGorgeousSystemDeveloperSettings* Settings = UGorgeousSystemDeveloperSettings::Get())
		{
			if (const TSoftObjectPtr<UGeneralSystemConfiguration_PDA>* Entry = Settings->PDAOverrides.Find(FName(*SystemName)))
			{
				if (Entry->IsValid())
				{
					return Entry->LoadSynchronous();
				}
			}
		}
	}

	// Next prefer the Extension Resource Guard mapping
	FString SystemRootPath;
	if (SystemName.Len() > 0)
	{
		if (UGorgeousExtensionResourceGuard* Guard = FindGuardForSystemName(SystemName))
		{
			if (Guard->bIsContentPackGuard && Guard->IsContentPresent())
			{
				SystemRootPath = FString::Printf(TEXT("/%s/%s"), *Guard->OwningPluginName.ToString(), *Guard->ContentSubPath);
			}
		}
	}

	// Finally, fallback to the path where the component itself is located
	if (SystemRootPath.IsEmpty())
	{
		SystemRootPath = GetSystemRootPath(ComponentClass->GetOutermost()->GetName());
	}

	const FString DesiredPackagePath = SystemRootPath / DesiredAssetName;

	// Safety check: is it already there?
	if (UObject* Existing = StaticFindObject(UGeneralSystemConfiguration_PDA::StaticClass(), nullptr, *DesiredPackagePath))
	{
		return Cast<UGeneralSystemConfiguration_PDA>(Existing);
	}

	if (CreatedSystems.Contains(SystemRootPath))
		return nullptr;

	CreatedSystems.Add(SystemRootPath);

	UPackage* Package = CreatePackage(*DesiredPackagePath);

	UGeneralSystemConfiguration_PDA* NewPDA =
		NewObject<UGeneralSystemConfiguration_PDA>(
			Package,
			*DesiredAssetName,
			RF_Public | RF_Standalone
		);
	
	NewPDA->MarkPackageDirty();

	FAssetRegistryModule::AssetCreated(NewPDA);
	UEditorAssetLibrary::SaveLoadedAsset(NewPDA);

	GT_I_LOG("GT.SystemValidator",
		TEXT("Created PDA '%s'"), *DesiredPackagePath);

	return NewPDA;
}

UGeneralSystemConfiguration_PDA* UGorgeousGeneralSystemValidator::FindExistingPDA(const UClass* ComponentClass)
{
	if (!ComponentClass)
		return nullptr;

	FString SystemName;
	const FString ConstructedName = ConstructPDASystemName(ComponentClass, SystemName);

	const FString DesiredAssetName = FString::Printf(TEXT("PDA_%s"), *SystemName);

	// 1) Developer settings override (exact SystemIdentifier match)
	if (SystemName.Len() > 0)
	{
		if (UGorgeousSystemDeveloperSettings* Settings = UGorgeousSystemDeveloperSettings::Get())
		{
			if (const TSoftObjectPtr<UGeneralSystemConfiguration_PDA>* Entry = Settings->PDAOverrides.Find(FName(*SystemName)))
			{
				if (Entry->IsValid())
				{
					return Entry->LoadSynchronous();
				}
			}
		}
	}

	// 2) Extension Resource Guard path (preferred)
	if (SystemName.Len() > 0)
	{
		if (UGorgeousExtensionResourceGuard* Guard = FindGuardForSystemName(SystemName))
		{
			if (Guard->bIsContentPackGuard && Guard->IsContentPresent())
			{
				const FString CandidatePath = FString::Printf(TEXT("/%s/%s/%s.%s"), *Guard->OwningPluginName.ToString(), *Guard->ContentSubPath, *DesiredAssetName, *DesiredAssetName);
				if (UObject* Loaded = StaticLoadObject(UGeneralSystemConfiguration_PDA::StaticClass(), nullptr, *CandidatePath))
				{
					return Cast<UGeneralSystemConfiguration_PDA>(Loaded);
				}
			}
		}
	}

	// 3) Authoritative path based on component's own location
	const FString RootPath = GetSystemRootPath(ComponentClass->GetOutermost()->GetName());
	if (!RootPath.IsEmpty())
	{
		const FString CandidatePath = FString::Printf(TEXT("%s/%s.%s"), *RootPath, *DesiredAssetName, *DesiredAssetName);
		if (UObject* Loaded = StaticLoadObject(UGeneralSystemConfiguration_PDA::StaticClass(), nullptr, *CandidatePath))
		{
			return Cast<UGeneralSystemConfiguration_PDA>(Loaded);
		}
	}

	// Not found
	return nullptr;
}
