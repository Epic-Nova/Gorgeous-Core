// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousUsefulSettings.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
enum class ECookProgressDisplayMode : int32;
//<-------------------------------------------------->

/**
 * This class surfaces hidden editor settings and CVars for convenience.
 * It modifies the corresponding editor settings/CVars when toggled.
 * 
 * @note Special Thanks to Ari Arnbjörnsson for the inspirational work, check out https://ari.games to see more gorgeous work from him
 */
UCLASS(Config = Editor, DefaultConfig, DisplayName = "Core ↪ Useful Settings")
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousUsefulSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	// Default constructor that initializes default values for the settings properties.
	UGorgeousUsefulSettings();

	// Get the settings and ensure they are synced with upstream sources. */
	static UGorgeousUsefulSettings* Get();
	
protected:
	
	//<============================--- Overrides ---============================>
	
	// Syncs with editor settings on load and when properties are changed in the editor UI
	virtual void PostInitProperties() override;
	
#if WITH_EDITOR
	// Unregister from editor settings delegates to avoid dangling references
	virtual void BeginDestroy() override;
	
	// Override to sync settings when changed in the editor UI
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITOR
	
	// Returns the name of the category in the Project Settings where these settings should appear. This is used to group these settings under a common category in the editor UI.
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	
	// Returns the display name of the section in the Project Settings where these settings should appear. This is used as the section header in the editor UI.
	virtual FText GetSectionText() const override;
	
	// Returns the description of the section in the Project Settings where these settings should appear. This is used as the section description in the editor UI.
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousUsefulSettings", "SectionDescription", "Convenience settings that surface hidden editor options and CVars without overriding engine defaults."); }
#endif
	//<------------------------------------------------------------------------->

public:
	
	// Toggle for Editor -> Data Validation -> Validate on Save (UDataValidationSettings::bValidateOnSave).
	UPROPERTY(EditAnywhere, Transient, Category = "Data Validation")
	bool bValidateOnSave;

	// Toggle for Editor -> Data Validation -> Load Assets when Validating Changelists (UDataValidationSettings::bLoadAssetsWhenValidatingChangelists).
	UPROPERTY(EditAnywhere, Transient, Category = "Data Validation")
	bool bLoadAssetsWhenValidatingChangelists;

	// When enabled, Gorgeous validators treat warnings as errors (local policy, not an engine setting).
	UPROPERTY(EditAnywhere, Config, Category = "Data Validation")
	bool bTreatValidationWarningsAsErrors;

	// Toggle for Editor -> Experimental -> Blueprint Break on Exceptions (UEditorExperimentalSettings::bBreakOnExceptions).
	UPROPERTY(EditAnywhere, Transient, Category = "Blueprint Debugging")
	bool bBreakOnExceptions;

	// Toggle ScriptStackOnWarnings (stored in DefaultEngine.ini). Requires editor restart to apply to Blueprint runtime diagnostics.
	UPROPERTY(EditAnywhere, Transient, Category = "Blueprint Debugging", meta = (ConfigRestartRequired = true))
	bool bScriptStackOnWarnings;

	// Set [Core.Log] LogStreaming=Verbose (equivalent to "Log LogStreaming Verbose").
	UPROPERTY(EditAnywhere, Transient, Category = "Logging")
	bool bVerboseLogStreaming;

	// Set [Core.Log] LogGarbage=Verbose (equivalent to "Log LogGarbage Verbose").
	UPROPERTY(EditAnywhere, Transient, Category = "Logging")
	bool bVerboseLogGarbage;

	// Toggle for Cooker progress display mode (UCookerSettings::CookProgressDisplayMode).
	UPROPERTY(EditAnywhere, Transient, Category = "Cooking", DisplayName = "Cooker Progress Display Mode")
	ECookProgressDisplayMode CookProgressDisplayMode;
 	
	//<============================--- C++ Only ---=============================>
	
	/**
	 * Directly writes a key-value pair to an ini file section.
	 * Use this for settings where GConfig doesn't properly flush (e.g., sections defined in base engine configs).
	 * @param IniFilePath Full path to the ini file
	 * @param Section Section name (e.g., "Kismet")
	 * @param Key Key name (e.g., "ScriptStackOnWarnings")
	 * @param Value Value to write
	 * @param bReloadIntoGConfig If true, reloads the file into GConfig cache after writing
	 * @return True if successfully written
	 */
	static bool WriteIniSettingDirect(const FString& IniFilePath, const FString& Section, const FString& Key, const FString& Value, bool bReloadIntoGConfig = true);

private:
	
	// Syncs the settings from the editor settings and CVars to ensure the UI reflects the current state of those settings.
	void SyncFromEditorSettings();
	
	/** 
	 * Applies the current settings to the corresponding editor settings and CVars, 
	 * ensuring that changes made in this settings class are propagated to the actual engine/editor configuration.
	 */
	void ApplyToEditorSettings() const;
	
	/** 
	 * Registers delegates to sync with editor settings when they are changed externally (e.g., through the editor UI or other code), 
	 * ensuring that this settings class stays up to date with the current state of those settings.
	 */
	void RegisterExternalSync();
	
	// Unregisters delegates to prevent dangling references when this settings object is destroyed.
	void UnregisterExternalSync();
	
#if WITH_EDITOR
	/**
	 * Handler for global object property changes, used to catch changes to settings that don't properly broadcast their own OnSettingChanged delegate (like UCookerSettings).
	 * This is used to sync the relevant properties in this settings class when the upstream settings are modified, ensuring that the UI reflects the current state of those settings.
	 * 
	 * @param Object The object that was modified. We filter for specific settings classes in the implementation.
	 * @param PropertyChangedEvent The details of the property change event.
	 */
	void HandleGlobalObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);
	
	/**
	 * Handler for when UDataValidationSettings are changed. 
	 * This is used to sync the relevant properties in this settings class when the upstream settings are modified, ensuring that the UI reflects the current state of those settings.
	 * 
	 * @param SettingsObject The settings object that was changed (expected to be of type UDataValidationSettings).
	 * @param PropertyChangedEvent The details of the property change event.
	 */
	void HandleDataValidationSettingsChanged(UObject* SettingsObject, FPropertyChangedEvent& PropertyChangedEvent);
	
	/**
	 * Handler for when UEditorExperimentalSettings are changed. 
	 * This is used to sync the relevant properties in this settings class when the upstream settings are modified, ensuring that the UI reflects the current state of those settings.
	 * 
	 * @param PropertyName The name of the property that was changed (expected to be a property of UEditorExperimentalSettings).
	 */
	void HandleExperimentalSettingsChanged(FName PropertyName);
	
	/**
	 * Handler for when relevant CVars are changed. 
	 * This is used to sync the relevant properties in this settings class when the upstream CVars are modified, ensuring that the UI reflects the current state of those settings.
	 * 
	 * @param ChangedCVar The console variable that was changed. We filter for specific CVars in the implementation.
	 */
	void HandleCVarChanged(IConsoleVariable* ChangedCVar);
#endif
	//<------------------------------------------------------------------------->

	
	//<============================--- Variables ---============================>

	// Internal flag to prevent recursive syncing when applying settings to editor settings that may trigger delegates
	bool bIsSyncing = false;

	// Flag to ensure we only register editor delegates once
	bool bExternalSyncRegistered = false;
	
#if WITH_EDITOR
	// Delegate handles for editor settings change notifications, used to unregister on destroy
	FDelegateHandle DataValidationSettingsChangedHandle;
	
	/** 
	 * Some settings (like UCookerSettings) don't properly broadcast their OnSettingChanged delegate, 
	 * so we use a global object property changed delegate and filter for those settings. 
	 * This handle is used to unregister that global delegate on destroy.
	 */
	FDelegateHandle ExperimentalSettingsChangedHandle;
	
	// Delegate handle for global object property changed delegate, used to unregister on destroy
	FDelegateHandle GlobalObjectPropertyChangedHandle;
#endif
	//<------------------------------------------------------------------------->
};