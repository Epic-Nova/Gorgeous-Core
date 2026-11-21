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
//<--------------------------=== Module Includes ===------------------------->
#include "ModuleCore/GorgeousCoreDeveloperSettings.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousLibrarySettings.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Library and Privacy
| Functional Name: UGorgeousLibrarySettings
| Parent Class: UGorgeousCoreDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Stores project-user API permission and endpoint settings alongside the
| library update and validation controls.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Config = EditorPerProjectUserSettings, meta = (GorgeousSettingsPage = "General", DisplayName = "Library and Privacy"))
class GORGEOUSCOREEDITOR_API UGorgeousLibrarySettings : public UGorgeousCoreDeveloperSettings
{
	GENERATED_BODY()

public:
	// Initializes the displayed validation interval and count.
	UGorgeousLibrarySettings();

#if WITH_EDITOR
	//<============================--- Overrides ---============================>
	#pragma region Overrides

	// Persists edits to the validation interval and development mode.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	// Restores validation state and development mode from Gorgeous persistent data.
	virtual void PostInitProperties() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
#endif


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions

	// Opens online-services permission and notification preferences with their saved values.
	UFUNCTION(CallInEditor, Category = "Network and Privacy", meta = (DisplayName = "Configure Online Services"))
	void ConfigureOnlineServices();

	// Starts the installer recompilation workflow after prompting to save the editor.
	UFUNCTION(CallInEditor, Category = "Recompilation")
	void TriggerRecompilation();

	// Checks the API for plugin updates, requesting consent if no choice is saved.
	UFUNCTION(CallInEditor, Category = "Updates")
	void CheckForUpdates();
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// Stores the API permission controlled by the online-services dialog.
	UPROPERTY(Config)
	bool bAllowGorgeousNetwork = false;

	// Records a saved permission choice, including dismissing the initial dialog.
	UPROPERTY(Config)
	bool bNetworkConsentDecided = false;

	// Uses HTTP for calls to the configured API origin during development.
	UPROPERTY(EditAnywhere, Category = "API Connection", meta = (DisplayName = "Force Dev Mode"))
	bool bForceDevMode = false;

	// Sets the API hostname and optional port. Force Dev Mode selects the protocol.
	UPROPERTY(EditAnywhere, Config, Category = "API Connection", meta = (DisplayName = "API Origin"))
	FString ApiOrigin;

	// Controls how many editor starts pass between automatic system validation runs.
	UPROPERTY(EditAnywhere, Category = "Validation", meta = (ClampMin = "1"))
	int32 ValidationInterval;

	// Displays how many times system validation has run.
	UPROPERTY(VisibleAnywhere, Category = "Validation")
	int32 ValidationCount;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};