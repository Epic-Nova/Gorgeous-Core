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

#include "Engine/DeveloperSettings.h"
#include "GorgeousUsefulSettings.generated.h"

enum class ECookProgressDisplayMode : int32;

//@TODO: Document this properly

/**
 * This class surfaces hidden editor settings and CVars for convenience.
 * It modifies the corresponding editor settings/CVars when toggled.
 */
UCLASS(Config = Editor, DefaultConfig, DisplayName = "Core ↪ Useful Settings")
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousUsefulSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousUsefulSettings();

	/** Get the settings and ensure they are synced with upstream sources. */
	static UGorgeousUsefulSettings* Get();

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual FName GetSectionName() const override { return TEXT("30_Core|Useful Settings"); }
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousUsefulSettings", "SectionDescription", "Convenience settings that surface hidden editor options and CVars without overriding engine defaults."); }
#endif

	/** Toggle for Editor -> Data Validation -> Validate on Save (UDataValidationSettings::bValidateOnSave). */
	UPROPERTY(EditAnywhere, Transient, Category = "Data Validation")
	bool bValidateOnSave;

	/** Toggle for Editor -> Data Validation -> Load Assets when Validating Changelists (UDataValidationSettings::bLoadAssetsWhenValidatingChangelists). */
	UPROPERTY(EditAnywhere, Transient, Category = "Data Validation")
	bool bLoadAssetsWhenValidatingChangelists;

	/** When enabled, Gorgeous validators treat warnings as errors (local policy, not an engine setting). */
	UPROPERTY(EditAnywhere, Config, Category = "Data Validation")
	bool bTreatValidationWarningsAsErrors;

	/** Toggle for Editor -> Experimental -> Blueprint Break on Exceptions (UEditorExperimentalSettings::bBreakOnExceptions). */
	UPROPERTY(EditAnywhere, Transient, Category = "Blueprint Debugging")
	bool bBreakOnExceptions;

	/** Toggle ScriptStackOnWarnings (stored in DefaultEngine.ini). Requires editor restart to apply to Blueprint runtime diagnostics. */
	UPROPERTY(EditAnywhere, Transient, Category = "Blueprint Debugging", meta = (ConfigRestartRequired = true))
	bool bScriptStackOnWarnings;

	/** Set [Core.Log] LogStreaming=Verbose (equivalent to "Log LogStreaming Verbose"). */
	UPROPERTY(EditAnywhere, Transient, Category = "Logging")
	bool bVerboseLogStreaming;

	/** Set [Core.Log] LogGarbage=Verbose (equivalent to "Log LogGarbage Verbose"). */
	UPROPERTY(EditAnywhere, Transient, Category = "Logging")
	bool bVerboseLogGarbage;

	/** Toggle for Cooker progress display mode (UCookerSettings::CookProgressDisplayMode). */
	UPROPERTY(EditAnywhere, Transient, Category = "Cooking", DisplayName = "Cooker Progress Display Mode")
	ECookProgressDisplayMode CookProgressDisplayMode;
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
protected:
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void BeginDestroy() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void SyncFromEditorSettings();
	void ApplyToEditorSettings();
	void RegisterExternalSync();
	void UnregisterExternalSync();
#if WITH_EDITOR
	void HandleDataValidationSettingsChanged(UObject* SettingsObject, FPropertyChangedEvent& PropertyChangedEvent);
	void HandleExperimentalSettingsChanged(FName PropertyName);
	void HandleGlobalObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);
	void HandleCVarChanged(IConsoleVariable* ChangedCVar);
#endif
	bool bIsSyncing = false;
	bool bExternalSyncRegistered = false;
#if WITH_EDITOR
	FDelegateHandle DataValidationSettingsChangedHandle;
	FDelegateHandle ExperimentalSettingsChangedHandle;
	FDelegateHandle GlobalObjectPropertyChangedHandle;
#endif
};
