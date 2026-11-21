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
#include "LibraryWizard/GorgeousNetworkConsent.h"

#if WITH_DEV_AUTOMATION_TESTS
//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "LibraryWizard/GorgeousLibrarySettings.h"
#include "NudgeHelper/GorgeousNudgeDeveloperSettings.h"
#include "DetailExtensions/GorgeousDetailExtensionSettings.h"
#include "ModuleCore/GorgeousAutoReplicationSettings.h"
#include "ModuleCore/GorgeousUsefulSettings.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "HAL/FileManager.h"
#include "IDetailsView.h"
#include "PropertyPath.h"
#include "ISettingsCategory.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "Widgets/Text/STextBlock.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// Helpers
//=============================================================================

namespace
{
	int32 CountWidgetText(const TSharedRef<SWidget>& Widget, const FString& Text)
	{
		int32 Count = Widget->GetTypeAsString() == TEXT("STextBlock")
			&& StaticCastSharedRef<STextBlock>(Widget)->GetText().ToString() == Text ? 1 : 0;
		FChildren* Children = Widget->GetChildren();
		for (int32 Index = 0; Index < Children->Num(); ++Index) Count += CountWidgetText(Children->GetChildAt(Index), Text);
		return Count;
	}

	int32 CountSettingsObject(const TSharedRef<SWidget>& Widget, const UObject* Settings, FName Property = NAME_None)
	{
		int32 Count = 0;
		if (Widget->GetTypeAsString() == TEXT("SDetailsView"))
		{
			const auto Details = StaticCastSharedRef<IDetailsView>(Widget);
			for (const TWeakObjectPtr<UObject>& Object : Details->GetSelectedObjects())
			{
				if (Object.Get() != Settings) continue;
				if (Property.IsNone())
				{
					++Count;
					continue;
				}
				for (const FPropertyPath& Path : Details->GetPropertiesInOrderDisplayed())
				{
					if (Path.IsValid() && Path.GetLeafMostProperty().Property.Get() == Settings->GetClass()->FindPropertyByName(Property))
						++Count;
				}
			}
		}
		FChildren* Children = Widget->GetChildren();
		for (int32 Index = 0; Index < Children->Num(); ++Index) Count += CountSettingsObject(Children->GetChildAt(Index), Settings, Property);
		return Count;
	}
}

//=============================================================================
// FGorgeousNetworkConsentPolicyTest Implementation
//=============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousNetworkConsentPolicyTest,
	"GorgeousCore.Editor.NetworkConsent.Policy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousNetworkConsentPolicyTest::RunTest(const FString&)
{
	using namespace GorgeousNetworkConsent;
	TStrongObjectPtr<UGorgeousLibrarySettings> Settings(NewObject<UGorgeousLibrarySettings>());
	Settings->bAllowGorgeousNetwork = false;
	Settings->bNetworkConsentDecided = false;
	FEnvironment Environment;
	Environment.bSlateReady = true;
	TestTrue(TEXT("An undecided interactive first start requests consent"), ResolveAccess(*Settings, true, Environment) == EAccess::Prompt);
	TestTrue(TEXT("Background requests cannot open the dialog or use the network"), ResolveAccess(*Settings, false, Environment) == EAccess::Denied);
	Environment.bSlateReady = false;
	TestTrue(TEXT("Consent waits until Slate exists"), ResolveAccess(*Settings, true, Environment) == EAccess::Denied);
	Environment.bSlateReady = true;
	Settings->bNetworkConsentDecided = true;
	TestTrue(TEXT("A saved refusal prevents repeated startup prompts"), ResolveAccess(*Settings, true, Environment) == EAccess::Denied);
	Settings->bAllowGorgeousNetwork = true;
	TestTrue(TEXT("Saved permission permits normal requests"), ResolveAccess(*Settings, false, Environment) == EAccess::Allowed);
	Environment.bHeadless = true;
	TestTrue(TEXT("Commandlet and unattended runs cannot prompt or implicitly use saved permission"), ResolveAccess(*Settings, true, Environment) == EAccess::Denied);
	Environment.bHeadless = false;
	Environment.bAutomation = true;
	TestTrue(TEXT("Automation cannot open consent or start implicit API requests"), ResolveAccess(*Settings, true, Environment) == EAccess::Denied);
	Environment.bAllowOverride = true;
	TestTrue(TEXT("An explicit allow override supports deliberate headless API tests"), ResolveAccess(*Settings, true, Environment) == EAccess::Allowed);
	Environment.bOfflineOverride = true;
	TestTrue(TEXT("Offline overrides both a stored grant and the explicit allow override"), ResolveAccess(*Settings, true, Environment) == EAccess::Denied);
	return true;
}

//=============================================================================
// FGorgeousNetworkConsentDecisionTest Implementation
//=============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousNetworkConsentDecisionTest,
	"GorgeousCore.Editor.NetworkConsent.SelectedPreferencesPersist",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousNetworkConsentDecisionTest::RunTest(const FString&)
{
	using namespace GorgeousNetworkConsent;
	const FString Directory = FPaths::Combine(FPaths::AutomationTransientDir(), TEXT("GorgeousNetworkConsent"));
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString ConfigFilename = FPaths::CreateTempFilename(*Directory, TEXT("Consent"), TEXT(".ini"));
	ON_SCOPE_EXIT
	{
		if (GConfig) GConfig->UnloadFile(ConfigFilename);
		IFileManager::Get().Delete(*ConfigFilename, false, true);
	};
	TStrongObjectPtr<UGorgeousLibrarySettings> Settings(NewObject<UGorgeousLibrarySettings>());
	TStrongObjectPtr<UGorgeousNudgeDeveloperSettings> Nudges(NewObject<UGorgeousNudgeDeveloperSettings>());
	FSelection Selection;
	Selection.bEnableAutomaticNudges = true;
	Selection.bShowPluginUpdates = false;
	Selection.bShowGeneralSystemUpdates = true;
	Selection.bShowEcosystemNews = false;
	Selection.bShowTutorialForNewPlugins = false;
	Selection.bShowNewTutorialsWhenAvailable = false;
	Selection.bNotifyNewSystems = true;
	Selection.bShowCommonPluginTips = false;
	TestFalse(TEXT("Contextual data sharing requires an explicit opt-in"), Selection.bContextAwareTips);
	Selection.bContextAwareTips = true;
	ApplyDecision(*Settings, *Nudges, Selection, true, ConfigFilename);
	TestTrue(TEXT("Allow Selected grants API permission and records the decision"), Settings->bAllowGorgeousNetwork && Settings->bNetworkConsentDecided);
	const FSelection Applied = ReadSelection(*Nudges);
	TestTrue(TEXT("Existing automatic-nudge selection is applied"), Applied.bEnableAutomaticNudges);
	TestFalse(TEXT("Plugin update notices can be independently disabled"), Applied.bShowPluginUpdates);
	TestTrue(TEXT("Installed system update notices can remain selected"), Applied.bShowGeneralSystemUpdates);
	TestFalse(TEXT("Ecosystem news can be independently disabled"), Applied.bShowEcosystemNews);
	TestFalse(TEXT("Existing carousel tutorial selection is independent"), Applied.bShowTutorialForNewPlugins);
	TestFalse(TEXT("New tutorial toast can be independently disabled"), Applied.bShowNewTutorialsWhenAvailable);
	TestTrue(TEXT("New-system notifications can remain selected"), Applied.bNotifyNewSystems);
	TestFalse(TEXT("Common plugin tips can be independently disabled"), Applied.bShowCommonPluginTips);

	TStrongObjectPtr<UGorgeousLibrarySettings> LoadedSettings(NewObject<UGorgeousLibrarySettings>());
	TStrongObjectPtr<UGorgeousNudgeDeveloperSettings> LoadedNudges(NewObject<UGorgeousNudgeDeveloperSettings>());
	LoadedSettings->LoadConfig(UGorgeousLibrarySettings::StaticClass(), *ConfigFilename);
	LoadedNudges->LoadConfig(UGorgeousNudgeDeveloperSettings::StaticClass(), *ConfigFilename);
	TestTrue(TEXT("A fresh settings object loads the persisted permission grant"), LoadedSettings->bAllowGorgeousNetwork && LoadedSettings->bNetworkConsentDecided);
	const FSelection Reloaded = ReadSelection(*LoadedNudges);
	TestTrue(TEXT("Reload preserves automatic nudges"), Reloaded.bEnableAutomaticNudges);
	TestFalse(TEXT("Reload preserves the plugin update opt-out"), Reloaded.bShowPluginUpdates);
	TestTrue(TEXT("Reload preserves installed system updates"), Reloaded.bShowGeneralSystemUpdates);
	TestFalse(TEXT("Reload preserves the ecosystem news opt-out"), Reloaded.bShowEcosystemNews);
	TestFalse(TEXT("Reload preserves the separate carousel preference"), Reloaded.bShowTutorialForNewPlugins);
	TestFalse(TEXT("Reload preserves the tutorial toast opt-out"), Reloaded.bShowNewTutorialsWhenAvailable);
	TestTrue(TEXT("Reload preserves system notifications"), Reloaded.bNotifyNewSystems);
	TestFalse(TEXT("Reload preserves the plugin tip opt-out"), Reloaded.bShowCommonPluginTips);
	TestTrue(TEXT("Reload preserves the separate contextual tip opt-in"), Reloaded.bContextAwareTips);

	// The title-bar close handler uses this same refusal decision as Stay offline.
	ApplyDecision(*Settings, *Nudges, Selection, false, ConfigFilename);
	LoadedSettings->LoadConfig(UGorgeousLibrarySettings::StaticClass(), *ConfigFilename);
	TestTrue(TEXT("Refusal is persisted as a decided offline state"), !LoadedSettings->bAllowGorgeousNetwork && LoadedSettings->bNetworkConsentDecided);
	FEnvironment Interactive;
	Interactive.bSlateReady = true;
	TestTrue(TEXT("A closed or refused first-start dialog is not requested again"), ResolveAccess(*LoadedSettings, true, Interactive) == EAccess::Denied);
	TestTrue(TEXT("Only an isolated temporary config was written"), ConfigFilename != GEditorPerProjectIni && IFileManager::Get().FileExists(*ConfigFilename));
	return true;
}
//=============================================================================
// FGorgeousNetworkPreferencesDialogTest Implementation
//=============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousNetworkPreferencesDialogTest,
	"GorgeousCore.Editor.NetworkConsent.PreferencesReopenAndClose",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousNetworkPreferencesDialogTest::RunTest(const FString&)
{
	using namespace GorgeousNetworkConsent;
	TStrongObjectPtr<UGorgeousLibrarySettings> Settings(NewObject<UGorgeousLibrarySettings>());
	TStrongObjectPtr<UGorgeousNudgeDeveloperSettings> Nudges(NewObject<UGorgeousNudgeDeveloperSettings>());
	Settings->bAllowGorgeousNetwork = true;
	Settings->bNetworkConsentDecided = true;
	FEnvironment Interactive;
	Interactive.bSlateReady = true;
	TestTrue(TEXT("Preferences can reopen a saved grant without clearing consent"), CanOpenDialog(*Settings, EDialogMode::Preferences, Interactive));
	TestFalse(TEXT("A saved grant does not repeat the first-start prompt"), CanOpenDialog(*Settings, EDialogMode::InitialConsent, Interactive));
	TestTrue(TEXT("Checking whether preferences can open preserves the saved grant"), Settings->bAllowGorgeousNetwork && Settings->bNetworkConsentDecided);
	Settings->bAllowGorgeousNetwork = false;
	TestTrue(TEXT("Preferences also reopen a saved refusal without clearing the decision"), CanOpenDialog(*Settings, EDialogMode::Preferences, Interactive));
	TestFalse(TEXT("A saved refusal does not repeat initial consent"), CanOpenDialog(*Settings, EDialogMode::InitialConsent, Interactive));
	FEnvironment Blocked = Interactive;
	Blocked.bOfflineOverride = true;
	TestFalse(TEXT("Explicit preferences cannot bypass offline mode"), CanOpenDialog(*Settings, EDialogMode::Preferences, Blocked));
	Blocked = Interactive; Blocked.bAllowOverride = true;
	TestFalse(TEXT("A command-line permission override does not open a misleading preferences modal"), CanOpenDialog(*Settings, EDialogMode::Preferences, Blocked));
	Blocked = Interactive; Blocked.bHeadless = true;
	TestFalse(TEXT("Preferences never create windows in commandlets or unattended sessions"), CanOpenDialog(*Settings, EDialogMode::Preferences, Blocked));
	Blocked = Interactive; Blocked.bAutomation = true;
	TestFalse(TEXT("Preferences never create windows during automation"), CanOpenDialog(*Settings, EDialogMode::Preferences, Blocked));
	Blocked = Interactive; Blocked.bSlateReady = false;
	TestFalse(TEXT("Preferences require Slate"), CanOpenDialog(*Settings, EDialogMode::Preferences, Blocked));

	const FProperty* Permission = FindFProperty<FProperty>(UGorgeousLibrarySettings::StaticClass(), GET_MEMBER_NAME_CHECKED(UGorgeousLibrarySettings, bAllowGorgeousNetwork));
	if (TestNotNull(TEXT("Network permission remains a reflected config property"), Permission))
	{
		TestTrue(TEXT("Network permission retains Config persistence"), Permission->HasAnyPropertyFlags(CPF_Config));
		TestFalse(TEXT("The former direct-edit permission checkbox is removed"), Permission->HasAnyPropertyFlags(CPF_Edit));
	}
	const UFunction* Configure = UGorgeousLibrarySettings::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGorgeousLibrarySettings, ConfigureOnlineServices));
	if (TestNotNull(TEXT("Project Settings exposes Configure Online Services"), Configure))
	{
		TestTrue(TEXT("Configure Online Services is an editor button"), Configure->HasMetaData(TEXT("CallInEditor")));
		TestEqual(TEXT("The button has the requested readable title"), Configure->GetMetaData(TEXT("DisplayName")), FString(TEXT("Configure Online Services")));
	}

	const FString Directory = FPaths::Combine(FPaths::AutomationTransientDir(), TEXT("GorgeousNetworkConsent"));
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString ConfigFilename = FPaths::CreateTempFilename(*Directory, TEXT("Preferences"), TEXT(".ini"));
	ON_SCOPE_EXIT
	{
		if (GConfig) GConfig->UnloadFile(ConfigFilename);
		IFileManager::Get().Delete(*ConfigFilename, false, true);
	};
	FSelection Original;
	Original.bShowPluginUpdates = false;
	Original.bShowTutorialForNewPlugins = false;
	ApplyDecision(*Settings, *Nudges, Original, true, ConfigFilename);
	FString BeforeCancel;
	if (!TestTrue(TEXT("Read isolated saved preferences before cancel"), FFileHelper::LoadFileToString(BeforeCancel, *ConfigFilename))) return false;
	FSelection Pending = Original;
	bool FSelection::* const Members[] = { &FSelection::bEnableAutomaticNudges, &FSelection::bShowPluginUpdates,
		&FSelection::bShowGeneralSystemUpdates, &FSelection::bShowEcosystemNews, &FSelection::bShowTutorialForNewPlugins,
		&FSelection::bShowNewTutorialsWhenAvailable, &FSelection::bNotifyNewSystems, &FSelection::bShowCommonPluginTips,
		&FSelection::bContextAwareTips };
	for (auto Member : Members) Pending.*Member = !(Original.*Member);
	TestFalse(TEXT("Closing explicit preferences does not save an offline decision"), ApplyDialogClose(*Settings, *Nudges, Pending, EDialogMode::Preferences, ConfigFilename));
	TestTrue(TEXT("Canceling preferences retains the earlier network grant"), Settings->bAllowGorgeousNetwork && Settings->bNetworkConsentDecided);
	const FSelection AfterCancel = ReadSelection(*Nudges);
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Members); ++Index)
	{
		TestEqual(*FString::Printf(TEXT("Cancel preserves notification preference %d"), Index), AfterCancel.*Members[Index], Original.*Members[Index]);
	}
	FString AfterCancelFile;
	TestTrue(TEXT("Read the same isolated config after cancel"), FFileHelper::LoadFileToString(AfterCancelFile, *ConfigFilename));
	TestEqual(TEXT("Canceling preferences leaves the saved file byte-for-byte unchanged"), AfterCancelFile, BeforeCancel);

	Settings->bAllowGorgeousNetwork = false;
	Settings->bNetworkConsentDecided = false;
	TestTrue(TEXT("An undecided interactive first start can show initial consent"), CanOpenDialog(*Settings, EDialogMode::InitialConsent, Interactive));
	TestTrue(TEXT("Closing initial consent persists refusal"), ApplyDialogClose(*Settings, *Nudges, Pending, EDialogMode::InitialConsent, ConfigFilename));
	TestTrue(TEXT("Initial dismissal is a decided offline state"), !Settings->bAllowGorgeousNetwork && Settings->bNetworkConsentDecided);
	TStrongObjectPtr<UGorgeousLibrarySettings> Reloaded(NewObject<UGorgeousLibrarySettings>());
	Reloaded->LoadConfig(UGorgeousLibrarySettings::StaticClass(), *ConfigFilename);
	TestTrue(TEXT("Initial dismissal survives an isolated config reload"), !Reloaded->bAllowGorgeousNetwork && Reloaded->bNetworkConsentDecided);
	TestFalse(TEXT("Initial dismissal prevents another automatic prompt"), CanOpenDialog(*Reloaded, EDialogMode::InitialConsent, Interactive));
	return true;
}

//=============================================================================
// FGorgeousCoreSettingsRouteTest Implementation
//=============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousCoreSettingsRouteTest,
	"GorgeousCore.Editor.Settings.CorePageRouting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousCoreSettingsRouteTest::RunTest(const FString&)
{
	const auto* AutoReplication = GetDefault<UGorgeousAutoReplicationSettings>();
	TestEqual(TEXT("Auto Replication selects its own settings destination"),
		AutoReplication->GetClass()->GetMetaData(TEXT("GorgeousSettingsPage")), FString(TEXT("AutoReplication")));
	TestEqual(TEXT("Detail Extensions has the requested visible name"),
		GetDefault<UGorgeousDetailExtensionSettings>()->GetSectionText().ToString(), FString(TEXT("Detail Extensions")));
	TestEqual(TEXT("Nudge choices belong to the Project settings container"),
		UGorgeousNudgeDeveloperSettings::Get()->GetContainerName(), FName(TEXT("Project")));
	ISettingsModule* Settings = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings"));
	const auto Project = Settings ? Settings->GetContainer(TEXT("Project")) : nullptr;
	const auto Category = Project ? Project->GetCategory(TEXT("Gorgeous Things")) : nullptr;
	if (!Category || !Category->GetSection(TEXT("CoreLibrary")))
	{
		AddInfo(TEXT("This commandlet does not register the settings UI; CDO routing and display names were checked without opening windows."));
		return true;
	}
	const auto General = Category->GetSection(TEXT("CoreLibrary"));
	const auto GeneralWidget = General->GetCustomWidget().Pin();
	if (TestTrue(TEXT("The General page retains its actual settings widget"), GeneralWidget.IsValid()))
	{
		TestEqual(TEXT("General renders the explicit Configure Online Services button for its class-default settings"),
			CountWidgetText(GeneralWidget.ToSharedRef(), TEXT("Configure Online Services")), 1);
	}
	const auto Dedicated = Category->GetSection(TEXT("CoreAutoReplication"));
	if (!TestTrue(TEXT("The real Project container registers a dedicated Auto Replication page"), Dedicated.IsValid())) return false;
	TestEqual(TEXT("The registered page has the requested arrow-style title"), Dedicated->GetDisplayName().ToString(), FString(TEXT("Core ↪ Auto Replication")));
	const auto DedicatedWidget = Dedicated->GetCustomWidget().Pin();
	if (TestTrue(TEXT("The dedicated page retains its real settings widget"), DedicatedWidget.IsValid()))
	{
		TestEqual(TEXT("The dedicated page edits the existing Auto Replication CDO exactly once"), CountSettingsObject(DedicatedWidget.ToSharedRef(), AutoReplication), 1);
	}
	const auto Debugging = Category->GetSection(TEXT("CoreDebugging"));
	const auto DebuggingWidget = Debugging ? Debugging->GetCustomWidget().Pin() : nullptr;
	if (TestTrue(TEXT("The Debugging page remains registered"), DebuggingWidget.IsValid()))
	{
		TestEqual(TEXT("Auto Replication is absent from the Debugging page"), CountSettingsObject(DebuggingWidget.ToSharedRef(), AutoReplication), 0);
		const auto GeneralProtectionPage = Category->GetSection(TEXT("CoreLibrary"))->GetCustomWidget().Pin();
		if (TestTrue(TEXT("The General page remains registered"), GeneralProtectionPage.IsValid()))
		{
			const auto* UsefulSettings = GetDefault<UGorgeousUsefulSettings>();
			for (const FName Property : {GET_MEMBER_NAME_CHECKED(UGorgeousUsefulSettings, bDisableExtensionResourceProtection),
				GET_MEMBER_NAME_CHECKED(UGorgeousUsefulSettings, bAllowAutomaticPluginDescriptorChanges)})
			{
				TestEqual(Property.ToString() + TEXT(" occurs once in Debugging"),
					CountSettingsObject(DebuggingWidget.ToSharedRef(), UsefulSettings, Property), 1);
				TestEqual(Property.ToString() + TEXT(" is absent from General"),
					CountSettingsObject(GeneralProtectionPage.ToSharedRef(), UsefulSettings, Property), 0);
			}
		}
	}
	return true;
}
#endif