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
#include "Engine/DeveloperSettings.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousNudgeDeveloperSettings.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Nudges
| Functional Name: UGorgeousNudgeDeveloperSettings
| Parent Class: UDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Stores the project-user preferences that control automatic Gorgeous nudge
| checks and tutorial recommendations.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig,
	meta = (
		DisplayName = "Nudges",
		DocumentationOverview = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/GorgeousNudgeDeveloperSettings",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/Examples"
		))
class GORGEOUSCOREEDITOR_API UGorgeousNudgeDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Returns the settings category displayed by the editor.
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }

	// Returns the settings section displayed by the editor.
	virtual FText GetSectionText() const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:

	/**
	 * Returns the project-user nudge settings.
	 *
	 * @return The project-user nudge settings.
	 */
	static const UGorgeousNudgeDeveloperSettings* Get();
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Enables periodic checks and automatic carousel popups.
	UPROPERTY(EditAnywhere, Config, Category = "Nudges")
	bool bEnableAutomaticNudges = true;

	// Defines the interval between automatic nudge checks.
	UPROPERTY(EditAnywhere, Config, Category = "Nudges", meta = (ClampMin = "60.0"))
	float CheckIntervalSeconds = 3600.0f;

	// Enables tutorial recommendations for newly installed plugins.
	UPROPERTY(EditAnywhere, Config, Category = "Nudges")
	bool bShowTutorialForNewPlugins = true;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};
