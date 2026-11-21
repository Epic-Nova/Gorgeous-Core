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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousLibrarySettings;
class UGorgeousNudgeDeveloperSettings;
//<------------------------------------------------------------->

// Per-project, per-user permission shared by all Gorgeous editor HTTP requests.
namespace GorgeousNetworkConsent
{
	// Describes the context needed to decide permission without displaying a window.
	struct FEnvironment
	{
		// An explicit offline override takes precedence over every other setting.
		bool bOfflineOverride = false;
		// Allows deliberate noninteractive API use without opening a consent window.
		bool bAllowOverride = false;
		// Commandlets and unattended sessions cannot display a consent window.
		bool bHeadless = false;
		// Automation never displays a consent window or implicitly enables network access.
		bool bAutomation = false;
		// Indicates that Slate is available to display a requested consent window.
		bool bSlateReady = false;
	};

	// Describes the result of checking the current permission and execution context.
	enum class EAccess : uint8
	{
		Denied,
		Allowed,
		Prompt
	};

	// Distinguishes an initial permission request from editing saved preferences.
	enum class EDialogMode : uint8
	{
		InitialConsent,
		Preferences
	};

	// Holds the choices edited by the dialog until the user commits a decision.
	struct FSelection
	{
		// Enables automatic nudge checks and presentation.
		bool bEnableAutomaticNudges = true;
		// Includes plugin update notices in the carousel.
		bool bShowPluginUpdates = true;
		// Includes installed general-system update notices in the carousel.
		bool bShowGeneralSystemUpdates = true;
		// Includes Gorgeous ecosystem news in the carousel.
		bool bShowEcosystemNews = true;
		// Includes tutorials for newly installed plugins in the existing carousel.
		bool bShowTutorialForNewPlugins = true;
		// Displays a notification when the API announces a newly available tutorial.
		bool bShowNewTutorialsWhenAvailable = true;
		// Displays notifications for newly available general systems.
		bool bNotifyNewSystems = true;
		// Displays relevant tips for installed Gorgeous plugins.
		bool bShowCommonPluginTips = true;
		bool bContextAwareTips = false;
	};

	/**
	 * Resolves permission without creating a window or modifying preferences.
	 * @param Settings The saved project-user network preferences.
	 * @param bPrompt Whether the caller explicitly allows interactive consent.
	 * @param Environment The current execution and override context.
	 * @return The permitted action for this request.
	 */
	GORGEOUSCOREEDITOR_API EAccess ResolveAccess(const UGorgeousLibrarySettings& Settings, bool bPrompt, const FEnvironment& Environment);

	/**
	 * Checks whether an explicit preferences window or initial prompt can open.
	 * @param Settings The saved project-user network preferences.
	 * @param Mode Whether this is initial consent or an explicit preferences edit.
	 * @param Environment The current execution and override context.
	 * @return Whether a dialog may open without changing the saved permission.
	 */
	GORGEOUSCOREEDITOR_API bool CanOpenDialog(const UGorgeousLibrarySettings& Settings, EDialogMode Mode, const FEnvironment& Environment);

	/**
	 * Reads the preferences used to initialize the consent checkboxes.
	 * @param NudgeSettings The current notification and carousel preferences.
	 * @return The current checkbox values.
	 */
	GORGEOUSCOREEDITOR_API FSelection ReadSelection(const UGorgeousNudgeDeveloperSettings& NudgeSettings);

	/**
	 * Stores a consent decision and the selected features in one user config file.
	 * @param Settings The network permission to update.
	 * @param NudgeSettings The notification and carousel preferences to update.
	 * @param Selection The checkbox values selected in the dialog.
	 * @param bAllowSelected Whether the decision authorizes API requests.
	 * @param ConfigFilename The destination config file, normally GEditorPerProjectIni.
	 */
	GORGEOUSCOREEDITOR_API void ApplyDecision(UGorgeousLibrarySettings& Settings, UGorgeousNudgeDeveloperSettings& NudgeSettings,
		const FSelection& Selection, bool bAllowSelected, const FString& ConfigFilename);

	/**
	 * Saves an initial dismissal as offline or cancels an explicit preferences edit.
	 * @param Settings The network permission to preserve or update.
	 * @param NudgeSettings The notification preferences to preserve or update.
	 * @param Selection The uncommitted checkbox values currently in the dialog.
	 * @param Mode Whether this is initial consent or an explicit preferences edit.
	 * @param ConfigFilename The destination used when initial consent is dismissed.
	 * @return Whether an offline decision was saved; false leaves settings untouched.
	 */
	GORGEOUSCOREEDITOR_API bool ApplyDialogClose(UGorgeousLibrarySettings& Settings, UGorgeousNudgeDeveloperSettings& NudgeSettings,
		const FSelection& Selection, EDialogMode Mode, const FString& ConfigFilename);

	// Opens the online-services preferences using the saved choices, without resetting consent.
	GORGEOUSCOREEDITOR_API void ShowPreferences();

	/**
	 * Checks network permission and optionally displays the consent window.
	 * @param bPrompt Whether the caller explicitly allows interactive consent.
	 * @return Whether Gorgeous editor API requests are permitted.
	 */
	GORGEOUSCOREEDITOR_API bool IsAllowed(bool bPrompt = false);

	/**
	 * Defers the first-start prompt until the interactive editor window is ready.
	 * @param OnAllowed The startup work to retry after the user grants permission.
	 */
	GORGEOUSCOREEDITOR_API void ScheduleStartupPrompt(const FSimpleDelegate& OnAllowed);

	// Cancels a deferred prompt before the editor module is unloaded.
	GORGEOUSCOREEDITOR_API void CancelStartupPrompt();
}