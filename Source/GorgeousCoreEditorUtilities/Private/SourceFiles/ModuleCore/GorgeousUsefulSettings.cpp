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
#include "GorgeousUsefulSettings.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CookerSettings.h"
#include "DataValidationSettings.h"
#include "Settings/EditorExperimentalSettings.h"
//<-------------------------------------------------------------------------->

namespace
{
	const TCHAR* ScriptStackSection = TEXT("Kismet");
	const TCHAR* ScriptStackKey = TEXT("ScriptStackOnWarnings");
	const TCHAR* CoreLogSection = TEXT("Core.Log");
	const TCHAR* LogStreamingKey = TEXT("LogStreaming");
	const TCHAR* LogGarbageKey = TEXT("LogGarbage");

	bool IsVerboseLogSetting(const FString& Value)
	{
		return Value.Equals(TEXT("Verbose"), ESearchCase::IgnoreCase)
			|| Value.Equals(TEXT("VeryVerbose"), ESearchCase::IgnoreCase);
	}
}

//=============================================================================
// UGorgeousUsefulSettings Implementation
//=============================================================================

bool UGorgeousUsefulSettings::WriteIniSettingDirect(const FString& IniFilePath, const FString& Section, const FString& Key, const FString& Value, bool bReloadIntoGConfig)
{
	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *IniFilePath))
	{
		// File doesn't exist, create it with the section and key
		FileContents = FString::Printf(TEXT("[%s]\n%s=%s\n"), *Section, *Key, *Value);
		if (!FFileHelper::SaveStringToFile(FileContents, *IniFilePath))
		{
			GT_W_LOG("GT.Editor.Settings", TEXT("WriteIniSettingDirect: Failed to create ini file %s"), *IniFilePath);
			return false;
		}
	}
	else
	{
		const FString SectionHeader = FString::Printf(TEXT("[%s]"), *Section);
		const FString KeyPattern = Key + TEXT("=");
		const FString NewLine = KeyPattern + Value;

		if (const int32 SectionIndex = FileContents.Find(SectionHeader, ESearchCase::IgnoreCase); SectionIndex != INDEX_NONE)
		{
			// Find the end of this section (next section or end of file)
			int32 NextSectionIndex = FileContents.Find(TEXT("["), ESearchCase::IgnoreCase, ESearchDir::FromStart, SectionIndex + SectionHeader.Len());
			int32 SearchEndIndex = (NextSectionIndex != INDEX_NONE) ? NextSectionIndex : FileContents.Len();
			
			// Find the key within this section only
			int32 KeyIndex = INDEX_NONE;
			int32 SearchStart = SectionIndex + SectionHeader.Len();
			int32 TempIndex = FileContents.Find(KeyPattern, ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchStart);
			if (TempIndex != INDEX_NONE && TempIndex < SearchEndIndex)
			{
				KeyIndex = TempIndex;
			}
			
			if (KeyIndex != INDEX_NONE)
			{
				// Find the end of this line
				int32 LineEnd = FileContents.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, KeyIndex);
				if (LineEnd == INDEX_NONE)
				{
					LineEnd = FileContents.Len();
				}
				// Replace the line (handle \r\n)
				int32 LineEndActual = LineEnd;
				if (LineEnd > 0 && FileContents[LineEnd - 1] == '\r')
				{
					LineEndActual = LineEnd - 1;
				}
				FileContents = FileContents.Left(KeyIndex) + NewLine + FileContents.Mid(LineEndActual);
			}
			else
			{
				// Key not found in section, add it after the section header
				int32 InsertPos = SectionIndex + SectionHeader.Len();
				// Skip newlines after section header
				while (InsertPos < FileContents.Len() && (FileContents[InsertPos] == '\r' || FileContents[InsertPos] == '\n'))
				{
					InsertPos++;
				}
				FileContents = FileContents.Left(InsertPos) + NewLine + TEXT("\n") + FileContents.Mid(InsertPos);
			}
		}
		else
		{
			// Section doesn't exist, add it at the beginning
			FileContents = SectionHeader + TEXT("\n") + NewLine + TEXT("\n\n") + FileContents;
		}
		
		if (!FFileHelper::SaveStringToFile(FileContents, *IniFilePath))
		{
			GT_W_LOG("GT.Editor.Settings", TEXT("WriteIniSettingDirect: Failed to save ini file %s"), *IniFilePath);
			return false;
		}
	}
	
	// Reload the file into GConfig so in-memory state matches disk
	if (bReloadIntoGConfig && GConfig)
	{
		GConfig->UnloadFile(IniFilePath);
		GConfig->LoadFile(IniFilePath);
	}
	
	GT_I_LOG("GT.Editor.Settings", TEXT("WriteIniSettingDirect: Wrote [%s] %s=%s to %s"), *Section, *Key, *Value, *IniFilePath);
	return true;
}

UGorgeousUsefulSettings::UGorgeousUsefulSettings()
{
	bValidateOnSave = false;
	bLoadAssetsWhenValidatingChangelists = true;
	bTreatValidationWarningsAsErrors = false;
	bBreakOnExceptions = false;
	bScriptStackOnWarnings = false;
	bVerboseLogStreaming = false;
	bVerboseLogGarbage = false;
	CookProgressDisplayMode = ECookProgressDisplayMode::Nothing;
}

UGorgeousUsefulSettings* UGorgeousUsefulSettings::Get()
{
	UGorgeousUsefulSettings* Settings = GetMutableDefault<UGorgeousUsefulSettings>();
	if (Settings)
	{
		Settings->SyncFromEditorSettings();
	}
	return Settings;
}

#if WITH_EDITOR
FText UGorgeousUsefulSettings::GetSectionText() const
{
	// Sync settings when the section is accessed (e.g., when opening project settings)
	const_cast<UGorgeousUsefulSettings*>(this)->SyncFromEditorSettings();
	return NSLOCTEXT("GorgeousUsefulSettings", "SectionName", "Core ↪ Useful Settings");
}
#endif

void UGorgeousUsefulSettings::PostInitProperties()
{
	Super::PostInitProperties();
	SyncFromEditorSettings();
	RegisterExternalSync();
}

#if WITH_EDITOR
void UGorgeousUsefulSettings::BeginDestroy()
{
	UnregisterExternalSync();
	Super::BeginDestroy();
}
#endif

void UGorgeousUsefulSettings::SyncFromEditorSettings()
{
	if (bIsSyncing)
	{
		return;
	}

	bIsSyncing = true;

	if (const UDataValidationSettings* DataSettings = GetDefault<UDataValidationSettings>())
	{
		bValidateOnSave = DataSettings->bValidateOnSave;
		bLoadAssetsWhenValidatingChangelists = DataSettings->bLoadAssetsWhenValidatingChangelists;
	}

	if (const UEditorExperimentalSettings* ExperimentalSettings = GetDefault<UEditorExperimentalSettings>())
	{
		bBreakOnExceptions = ExperimentalSettings->bBreakOnExceptions;
	}

	if (const UCookerSettings* CookerSettings = GetDefault<UCookerSettings>())
	{
		CookProgressDisplayMode = CookerSettings->CookProgressDisplayMode;
	}

	if (GConfig)
	{
		// Read from the combined engine config (GEngineIni includes project settings layered on top)
		GConfig->GetBool(ScriptStackSection, ScriptStackKey, bScriptStackOnWarnings, GEngineIni);

		if (FString LogStreamingValue; GConfig->GetString(CoreLogSection, LogStreamingKey, LogStreamingValue, GEngineIni))
		{
			bVerboseLogStreaming = IsVerboseLogSetting(LogStreamingValue);
		}

		if (FString LogGarbageValue; GConfig->GetString(CoreLogSection, LogGarbageKey, LogGarbageValue, GEngineIni))
		{
			bVerboseLogGarbage = IsVerboseLogSetting(LogGarbageValue);
		}
	}

	bIsSyncing = false;
}

void UGorgeousUsefulSettings::ApplyToEditorSettings() const
{
	if (UDataValidationSettings* DataSettings = GetMutableDefault<UDataValidationSettings>())
	{
		DataSettings->bValidateOnSave = bValidateOnSave;
		DataSettings->bLoadAssetsWhenValidatingChangelists = bLoadAssetsWhenValidatingChangelists;
		DataSettings->SaveConfig();
	}

	if (UEditorExperimentalSettings* ExperimentalSettings = GetMutableDefault<UEditorExperimentalSettings>())
	{
		ExperimentalSettings->bBreakOnExceptions = bBreakOnExceptions;
		ExperimentalSettings->SaveConfig();
	}

	if (UCookerSettings* CookerSettings = GetMutableDefault<UCookerSettings>())
	{
		CookerSettings->CookProgressDisplayMode = CookProgressDisplayMode;
		CookerSettings->SaveConfig();
	}

	if (GConfig)
	{
		// Write to the project's DefaultEngine.ini
		const FString ProjectEngineIni = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
		
		// For Kismet section, we need to directly update the source config file
		// GConfig may not flush properly for sections that exist in engine base configs
		WriteIniSettingDirect(ProjectEngineIni, ScriptStackSection, ScriptStackKey, bScriptStackOnWarnings ? TEXT("True") : TEXT("False"));
		
		// For Core.Log section, GConfig works fine
		GConfig->SetString(CoreLogSection, LogStreamingKey, bVerboseLogStreaming ? TEXT("Verbose") : TEXT("Log"), ProjectEngineIni);
		GConfig->SetString(CoreLogSection, LogGarbageKey, bVerboseLogGarbage ? TEXT("Verbose") : TEXT("Log"), ProjectEngineIni);
		GConfig->Flush(true, ProjectEngineIni);
	}

	if (GEngine)
	{
		const TCHAR* StreamingVerbosity = bVerboseLogStreaming ? TEXT("Verbose") : TEXT("Log");
		const TCHAR* GarbageVerbosity = bVerboseLogGarbage ? TEXT("Verbose") : TEXT("Log");
		GEngine->Exec(nullptr, *FString::Printf(TEXT("Log LogStreaming %s"), StreamingVerbosity));
		GEngine->Exec(nullptr, *FString::Printf(TEXT("Log LogGarbage %s"), GarbageVerbosity));
	}
}

void UGorgeousUsefulSettings::RegisterExternalSync()
{
#if WITH_EDITOR
	if (bExternalSyncRegistered)
	{
		return;
	}

	bExternalSyncRegistered = true;

	if (UDataValidationSettings* DataSettings = GetMutableDefault<UDataValidationSettings>())
	{
		DataValidationSettingsChangedHandle = DataSettings->OnSettingChanged().AddUObject(
			this, &UGorgeousUsefulSettings::HandleDataValidationSettingsChanged);
	}

	if (UEditorExperimentalSettings* ExperimentalSettings = GetMutableDefault<UEditorExperimentalSettings>())
	{
		ExperimentalSettingsChangedHandle = ExperimentalSettings->OnSettingChanged().AddUObject(
			this, &UGorgeousUsefulSettings::HandleExperimentalSettingsChanged);
	}

	// Use global object property changed delegate for settings that don't call Super::PostEditChangeProperty
	// (like UCookerSettings), which means their OnSettingChanged delegate doesn't fire
	GlobalObjectPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(
		this, &UGorgeousUsefulSettings::HandleGlobalObjectPropertyChanged);
#endif
}

void UGorgeousUsefulSettings::UnregisterExternalSync()
{
#if WITH_EDITOR
	if (!bExternalSyncRegistered)
	{
		return;
	}

	bExternalSyncRegistered = false;

	if (UDataValidationSettings* DataSettings = GetMutableDefault<UDataValidationSettings>())
	{
		if (DataValidationSettingsChangedHandle.IsValid())
		{
			DataSettings->OnSettingChanged().Remove(DataValidationSettingsChangedHandle);
			DataValidationSettingsChangedHandle.Reset();
		}
	}

	if (UEditorExperimentalSettings* ExperimentalSettings = GetMutableDefault<UEditorExperimentalSettings>())
	{
		if (ExperimentalSettingsChangedHandle.IsValid())
		{
			ExperimentalSettings->OnSettingChanged().Remove(ExperimentalSettingsChangedHandle);
			ExperimentalSettingsChangedHandle.Reset();
		}
	}

	if (GlobalObjectPropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(GlobalObjectPropertyChangedHandle);
		GlobalObjectPropertyChangedHandle.Reset();
	}
#endif
}

#if WITH_EDITOR
void UGorgeousUsefulSettings::HandleGlobalObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	// Filter for settings classes that don't properly broadcast their OnSettingChanged delegate
	// Check both the class and ensure it's the CDO (default object) that's being modified
	if (Object && (Object->IsA<UCookerSettings>() || Object->GetClass() == UCookerSettings::StaticClass()))
	{
		SyncFromEditorSettings();
	}
}

void UGorgeousUsefulSettings::HandleDataValidationSettingsChanged(UObject* SettingsObject, FPropertyChangedEvent& PropertyChangedEvent)
{
	SyncFromEditorSettings();
}

void UGorgeousUsefulSettings::HandleExperimentalSettingsChanged(FName PropertyName)
{
	SyncFromEditorSettings();
}


void UGorgeousUsefulSettings::HandleCVarChanged(IConsoleVariable* ChangedCVar)
{
	SyncFromEditorSettings();
}
#endif

#if WITH_EDITOR
void UGorgeousUsefulSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bIsSyncing)
	{
		return;
	}

	bIsSyncing = true;
	ApplyToEditorSettings();
	bIsSyncing = false;
}
#endif
