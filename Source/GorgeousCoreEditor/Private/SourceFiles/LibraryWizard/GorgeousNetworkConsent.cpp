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

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "LibraryWizard/GorgeousLibrarySettings.h"
#include "LibraryWizard/GorgeousUpdateManager.h"
#include "NudgeHelper/GorgeousNudgeDeveloperSettings.h"
#include "NudgeHelper/GorgeousNudgeSubsystem.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Containers/Ticker.h"
#include "CoreGlobals.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Parse.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousNetworkConsent"

//=============================================================================
// Helpers
//=============================================================================

namespace
{
	FTSTicker::FDelegateHandle StartupPromptTicker;
	FSimpleDelegate StartupAllowedCallback;
	bool bConsentDialogOpen = false;

	GorgeousNetworkConsent::FEnvironment ReadEnvironment()
	{
		GorgeousNetworkConsent::FEnvironment Environment;
		Environment.bOfflineOverride = FPlatformMisc::GetEnvironmentVariable(TEXT("GORGEOUS_OFFLINE")) == TEXT("1")
			|| FParse::Param(FCommandLine::Get(), TEXT("GorgeousOffline"));
		Environment.bAllowOverride = FPlatformMisc::GetEnvironmentVariable(TEXT("GORGEOUS_ALLOW_NETWORK")) == TEXT("1")
			|| FParse::Param(FCommandLine::Get(), TEXT("AllowGorgeousNetwork"));
		Environment.bHeadless = IsRunningCommandlet() || FApp::IsUnattended() || GIsRunningUnattendedScript;
		FString StartupCommands;
		FParse::Value(FCommandLine::Get(), TEXT("ExecCmds="), StartupCommands);
		Environment.bAutomation = GIsAutomationTesting || StartupCommands.Contains(TEXT("Automation"), ESearchCase::IgnoreCase)
			|| FParse::Param(FCommandLine::Get(), TEXT("Automation"))
			|| FParse::Param(FCommandLine::Get(), TEXT("RunAutomationTests"));
		Environment.bSlateReady = FSlateApplication::IsInitialized();
		return Environment;
	}
}

//=============================================================================
// GorgeousNetworkConsent Implementation
//=============================================================================

GorgeousNetworkConsent::EAccess GorgeousNetworkConsent::ResolveAccess(const UGorgeousLibrarySettings& Settings,
	bool bPrompt, const FEnvironment& Environment)
{
	if (Environment.bOfflineOverride) return EAccess::Denied;
	if (Environment.bAllowOverride) return EAccess::Allowed;
	if (Environment.bHeadless || Environment.bAutomation) return EAccess::Denied;
	if (Settings.bAllowGorgeousNetwork) return EAccess::Allowed;
	return bPrompt && !Settings.bNetworkConsentDecided && Environment.bSlateReady ? EAccess::Prompt : EAccess::Denied;
}

bool GorgeousNetworkConsent::CanOpenDialog(const UGorgeousLibrarySettings& Settings,
	EDialogMode Mode, const FEnvironment& Environment)
{
	if (Environment.bOfflineOverride || Environment.bAllowOverride || Environment.bHeadless
		|| Environment.bAutomation || !Environment.bSlateReady) return false;
	return Mode == EDialogMode::Preferences || ResolveAccess(Settings, true, Environment) == EAccess::Prompt;
}

GorgeousNetworkConsent::FSelection GorgeousNetworkConsent::ReadSelection(const UGorgeousNudgeDeveloperSettings& NudgeSettings)
{
	FSelection Selection;
	Selection.bEnableAutomaticNudges = NudgeSettings.bEnableAutomaticNudges;
	Selection.bShowPluginUpdates = NudgeSettings.bShowPluginUpdates;
	Selection.bShowGeneralSystemUpdates = NudgeSettings.bShowGeneralSystemUpdates;
	Selection.bShowEcosystemNews = NudgeSettings.bShowEcosystemNews;
	Selection.bShowTutorialForNewPlugins = NudgeSettings.bShowTutorialForNewPlugins;
	Selection.bShowNewTutorialsWhenAvailable = NudgeSettings.bShowNewTutorialsWhenAvailable;
	Selection.bNotifyNewSystems = NudgeSettings.bNotifyNewSystems;
	Selection.bShowCommonPluginTips = NudgeSettings.bShowCommonPluginTips;
	Selection.bContextAwareTips = NudgeSettings.bContextAwareTips;
	return Selection;
}

void GorgeousNetworkConsent::ApplyDecision(UGorgeousLibrarySettings& Settings, UGorgeousNudgeDeveloperSettings& NudgeSettings,
	const FSelection& Selection, bool bAllowSelected, const FString& ConfigFilename)
{
	Settings.bAllowGorgeousNetwork = bAllowSelected;
	Settings.bNetworkConsentDecided = true;
	NudgeSettings.bShowNewTutorialsWhenAvailable = Selection.bShowNewTutorialsWhenAvailable;
	NudgeSettings.bNotifyNewSystems = Selection.bNotifyNewSystems;
	NudgeSettings.bShowCommonPluginTips = Selection.bShowCommonPluginTips;
	NudgeSettings.bContextAwareTips = Selection.bContextAwareTips;
	NudgeSettings.bEnableAutomaticNudges = Selection.bEnableAutomaticNudges;
	NudgeSettings.bShowPluginUpdates = Selection.bShowPluginUpdates;
	NudgeSettings.bShowGeneralSystemUpdates = Selection.bShowGeneralSystemUpdates;
	NudgeSettings.bShowEcosystemNews = Selection.bShowEcosystemNews;
	NudgeSettings.bShowTutorialForNewPlugins = Selection.bShowTutorialForNewPlugins;
	Settings.SaveConfig(CPF_Config, *ConfigFilename);
	NudgeSettings.SaveConfig(CPF_Config, *ConfigFilename);
}

bool GorgeousNetworkConsent::ApplyDialogClose(UGorgeousLibrarySettings& Settings, UGorgeousNudgeDeveloperSettings& NudgeSettings,
	const FSelection& Selection, EDialogMode Mode, const FString& ConfigFilename)
{
	if (Mode == EDialogMode::Preferences) return false;
	ApplyDecision(Settings, NudgeSettings, Selection, false, ConfigFilename);
	return true;
}

//=============================================================================
// Helpers
//=============================================================================

namespace
{
	/**
	 * Displays either initial consent or an explicit edit of the saved choices.
	 * @param Mode Whether closing refuses initial consent or cancels preferences.
	 * @return The stored network permission after the window closes.
	 */
	bool ShowConsentDialog(GorgeousNetworkConsent::EDialogMode Mode)
	{
		using namespace GorgeousNetworkConsent;
		auto* Settings = GetMutableDefault<UGorgeousLibrarySettings>();
		if (!CanOpenDialog(*Settings, Mode, ReadEnvironment()) || bConsentDialogOpen) return Settings->bAllowGorgeousNetwork;
		TGuardValue<bool> DialogGuard(bConsentDialogOpen, true);

		auto* NudgeSettings = GetMutableDefault<UGorgeousNudgeDeveloperSettings>();
		const auto Selection = MakeShared<FSelection>(ReadSelection(*NudgeSettings));
		const auto bCommitted = MakeShared<bool>(false);
		const TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("Title", "Gorgeous API access"))
			.ClientSize(FVector2D(650, 440)).SizingRule(ESizingRule::Autosized)
			.SupportsMinimize(false).SupportsMaximize(false);
		const TWeakPtr<SWindow> WeakWindow = Window;
		const auto Commit = [Settings, NudgeSettings, Selection, bCommitted, WeakWindow, Mode](bool bAllowSelected)
		{
			ApplyDecision(*Settings, *NudgeSettings, *Selection, bAllowSelected, GEditorPerProjectIni);
			*bCommitted = true;
			if (Mode == EDialogMode::Preferences && bAllowSelected && GEditor)
			{
				if (auto* Updates = GEditor->GetEditorSubsystem<UGorgeousUpdateManager>())
				{
					Updates->FetchSystemsCatalog();
				}
				if (Selection->bEnableAutomaticNudges)
				{
					if (auto* Nudges = GEditor->GetEditorSubsystem<UGorgeousNudgeSubsystem>()) Nudges->CheckForNudgesNow();
				}
			}
			if (const auto PinnedWindow = WeakWindow.Pin()) PinnedWindow->RequestDestroyWindow();
			return FReply::Handled();
		};
		Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([Settings, NudgeSettings, Selection, bCommitted, Mode](const TSharedRef<SWindow>&)
		{
			// Initial dismissal stays offline; canceling preferences leaves saved choices untouched.
			if (!*bCommitted) ApplyDialogClose(*Settings, *NudgeSettings, *Selection, Mode, GEditorPerProjectIni);
		}));

		const TSharedRef<SVerticalBox> Preferences = SNew(SVerticalBox);
		const auto AddPreference = [Preferences, Selection](const FText& Label, const FText& Tooltip, bool FSelection::* Member)
		{
			Preferences->AddSlot().AutoHeight().Padding(0, 2)
			[
				SNew(SCheckBox)
				.ToolTipText(Tooltip)
				.IsChecked_Lambda([Selection, Member] { return Selection.Get().*Member ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([Selection, Member](ECheckBoxState State) { Selection.Get().*Member = State == ECheckBoxState::Checked; })
				[SNew(STextBlock).Text(Label)]
			];
		};
		AddPreference(LOCTEXT("AutomaticNudges", "Automatic news, update and tutorial checks"),
			LOCTEXT("AutomaticNudgesTooltip", "Periodically check the Gorgeous API and allow automatic nudge presentation."), &FSelection::bEnableAutomaticNudges);
		AddPreference(LOCTEXT("PluginUpdates", "Show plugin update notices"),
			LOCTEXT("PluginUpdatesTooltip", "Include available Gorgeous plugin updates in the nudge carousel."), &FSelection::bShowPluginUpdates);
		AddPreference(LOCTEXT("GeneralSystemUpdates", "Show installed system update notices"),
			LOCTEXT("GeneralSystemUpdatesTooltip", "Include update notices for installed Gorgeous general systems in the carousel."), &FSelection::bShowGeneralSystemUpdates);
		AddPreference(LOCTEXT("EcosystemNews", "Show Gorgeous ecosystem news"),
			LOCTEXT("EcosystemNewsTooltip", "Include ecosystem announcements and news in the nudge carousel."), &FSelection::bShowEcosystemNews);
		AddPreference(LOCTEXT("PluginTutorials", "Recommend tutorials for newly installed plugins"),
			LOCTEXT("PluginTutorialsTooltip", "Include getting-started recommendations in the existing nudge carousel."), &FSelection::bShowTutorialForNewPlugins);
		AddPreference(LOCTEXT("NewTutorials", "Show new tutorials when available"),
			LOCTEXT("NewTutorialsTooltip", "Show a toast when a new relevant tutorial becomes available, with an option to open it in a new window."), &FSelection::bShowNewTutorialsWhenAvailable);
		AddPreference(LOCTEXT("NewSystems", "Notify me about newly available systems"),
			LOCTEXT("NewSystemsTooltip", "Show a toast that opens the Gorgeous Library and highlights the available system."), &FSelection::bNotifyNewSystems);
		AddPreference(LOCTEXT("PluginTips", "Show tips for installed Gorgeous plugins"),
			LOCTEXT("PluginTipsTooltip", "Show relevant common plugin tips, including a documentation link when one is available."), &FSelection::bShowCommonPluginTips);
		AddPreference(LOCTEXT("ContextAwareTips", "Context Aware Tips"),
			LOCTEXT("ContextAwareTipsTooltip", "Occasionally request a tip for the kind of asset open in the editor. Requires automatic checks."),
			&FSelection::bContextAwareTips);

		const TSharedRef<SExpandableArea> DataDetails = SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()
			[SNew(STextBlock).Text(LOCTEXT("DataUseTitle", "What data is sent and why?"))]
			.BodyContent()
			[
				SNew(SBox).MaxDesiredHeight(170.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
						[SNew(STextBlock).WrapTextAt(580.f).Text(LOCTEXT("ApiAccessData",
							"• API access → The server receives your IP address and standard request headers to answer requests."))]
						+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
						[SNew(STextBlock).WrapTextAt(580.f).Text(LOCTEXT("UpdateCheckData",
							"• Update checks → Plugin names, versions, salted license proofs and source checksums help verify eligibility and find compatible releases."))]
						+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
						[SNew(STextBlock).WrapTextAt(580.f).Visibility_Lambda([Selection]
							{ return Selection->bEnableAutomaticNudges ? EVisibility::Visible : EVisibility::Collapsed; })
							.Text(LOCTEXT("AutomaticCheckData",
								"• Automatic checks → The editor periodically requests announcements and can fetch linked articles for enabled categories."))]
						+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
						[SNew(STextBlock).WrapTextAt(580.f).Visibility_Lambda([Selection]
							{ return Selection->bContextAwareTips ? EVisibility::Visible : EVisibility::Collapsed; })
							.Text(LOCTEXT("ContextAwareTipData",
								"• Context Aware Tips → When automatic checks are on, a sampled request sends one asset category, such as Blueprint or Material, to select a relevant tip. Asset names, paths and contents stay local."))]
					]
				]
			];

		Window->SetContent(SNew(SBox).WidthOverride(650.f)
			[SNew(SBorder).BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder"))).Padding(16)
				[SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
					[SNew(STextBlock).WrapTextAt(618.f).Text(LOCTEXT("Explanation",
						"Allow Gorgeous Core to contact the Gorgeous API for catalog, updates and downloads? Update checks include plugin names, versions and source checksums."))]
					+ SVerticalBox::Slot().AutoHeight()
					[SNew(SBox).MinDesiredHeight(120.f).MaxDesiredHeight(260.f)
						[SNew(SScrollBox) + SScrollBox::Slot()[Preferences]]]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
					[DataDetails]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 8)
					[SNew(STextBlock).WrapTextAt(618.f).Text(Mode == EDialogMode::Preferences
						? LOCTEXT("PreferencesCancel", "Changes apply to this project and user. Closing this window leaves your saved choices unchanged.")
						: LOCTEXT("InitialDismissal", "Change these choices later in Core General. Closing this window keeps API access disabled."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
					[SNew(STextBlock).WrapTextAt(618.f).Text(LOCTEXT("LegalNotice",
						"Review the EULA and Privacy Policy for Gorgeous Things."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
					[SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[SNew(SHyperlink).Text(LOCTEXT("Eula", "EULA"))
							.OnNavigate_Lambda([] { FPlatformProcess::LaunchURL(TEXT("https://epicnova.net/legal/eula/long"), nullptr, nullptr); })]
						+ SHorizontalBox::Slot().AutoWidth().Padding(16, 0, 0, 0)
						[SNew(SHyperlink).Text(LOCTEXT("Privacy", "Privacy Policy"))
							.OnNavigate_Lambda([] { FPlatformProcess::LaunchURL(TEXT("https://epicnova.net/legal/privacy-policy/long"), nullptr, nullptr); })]]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
					[SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(LOCTEXT("Deny", "Stay offline"))
							.OnClicked_Lambda([Commit] { return Commit(false); })]
						+ SHorizontalBox::Slot().AutoWidth().Padding(12, 0, 0, 0)[SNew(SButton).Text(LOCTEXT("AllowSelected", "Allow Selected"))
							.OnClicked_Lambda([Commit] { return Commit(true); })]]]]);
		FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().GetActiveTopLevelWindow());
		return Settings->bAllowGorgeousNetwork;
	}
}

//=============================================================================
// GorgeousNetworkConsent Implementation
//=============================================================================

bool GorgeousNetworkConsent::IsAllowed(bool bPrompt)
{
	const EAccess Access = ResolveAccess(*GetDefault<UGorgeousLibrarySettings>(), bPrompt, ReadEnvironment());
	return Access == EAccess::Prompt ? ShowConsentDialog(EDialogMode::InitialConsent) : Access == EAccess::Allowed;
}

void GorgeousNetworkConsent::ShowPreferences()
{
	ShowConsentDialog(EDialogMode::Preferences);
}

void GorgeousNetworkConsent::ScheduleStartupPrompt(const FSimpleDelegate& OnAllowed)
{
	FEnvironment Environment = ReadEnvironment();
	// Window readiness is checked by the ticker; all other policy gates apply now.
	Environment.bSlateReady = true;
	if (StartupPromptTicker.IsValid()
		|| ResolveAccess(*GetDefault<UGorgeousLibrarySettings>(), true, Environment) != EAccess::Prompt) return;
	StartupAllowedCallback = OnAllowed;
	StartupPromptTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float)
	{
		if (IsEngineExitRequested()) { CancelStartupPrompt(); return false; }
		FEnvironment CurrentEnvironment = ReadEnvironment();
		CurrentEnvironment.bSlateReady = true;
		if (ResolveAccess(*GetDefault<UGorgeousLibrarySettings>(), true, CurrentEnvironment) != EAccess::Prompt)
		{
			StartupPromptTicker.Reset();
			StartupAllowedCallback.Unbind();
			return false;
		}
		if (!GIsRunning || !FSlateApplication::IsInitialized()) return true;
		const auto ParentWindow = FSlateApplication::Get().GetActiveTopLevelRegularWindow();
		if (!ParentWindow || !ParentWindow->IsVisible()) return true;
		StartupPromptTicker.Reset();
		const FSimpleDelegate Callback = StartupAllowedCallback;
		StartupAllowedCallback.Unbind();
		if (IsAllowed(true)) Callback.ExecuteIfBound();
		return false;
	}));
}

void GorgeousNetworkConsent::CancelStartupPrompt()
{
	if (StartupPromptTicker.IsValid()) FTSTicker::RemoveTicker(StartupPromptTicker);
	StartupPromptTicker.Reset();
	StartupAllowedCallback.Unbind();
}

#undef LOCTEXT_NAMESPACE