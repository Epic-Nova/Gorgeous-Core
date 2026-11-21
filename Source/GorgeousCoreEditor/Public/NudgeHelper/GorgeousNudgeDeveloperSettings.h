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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousNudgeDeveloperSettings.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Nudges
| Functional Name: UGorgeousNudgeDeveloperSettings
| Parent Class: UGorgeousCoreDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Stores the project-user preferences that control automatic Gorgeous nudge
| checks and tutorial recommendations.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Config = EditorPerProjectUserSettings,
	meta = (GorgeousSettingsPage = "General",
		DisplayName = "Nudges",
		DocumentationOverview = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#updates-and-news",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Reference/ProjectSettings/#nudges",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#updates-and-news"
		))
class GORGEOUSCOREEDITOR_API UGorgeousNudgeDeveloperSettings : public UGorgeousCoreDeveloperSettings
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Shows these project-user preferences in Project Settings.
	virtual FName GetContainerName() const override { return TEXT("Project"); }

	// Returns the settings section displayed by the editor.
	virtual FText GetSectionText() const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only

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

	// Enables periodic checks and deferred update, news, and tutorial presentation.
	UPROPERTY(Config)
	bool bEnableAutomaticNudges = true;

	// Defines the interval between automatic nudge checks.
	UPROPERTY(EditAnywhere, Config, Category = "Nudges", meta = (ClampMin = "60.0"))
	float CheckIntervalSeconds = 3600.0f;

	// Enables tutorial recommendations for newly installed plugins.
	UPROPERTY(Config)
	bool bShowTutorialForNewPlugins = true;

	// Includes update announcements for enabled Gorgeous plugins in the carousel.
	UPROPERTY(Config)
	bool bShowPluginUpdates = true;

	// Includes update announcements for installed general systems in the carousel.
	UPROPERTY(Config)
	bool bShowGeneralSystemUpdates = true;

	// Includes general ecosystem news in the carousel.
	UPROPERTY(Config)
	bool bShowEcosystemNews = true;

	// Announces newly published tutorials in a toast with a separate article window.
	UPROPERTY(Config)
	bool bShowNewTutorialsWhenAvailable = true;

	// Announces downloadable general systems and offers to select them in the Library.
	UPROPERTY(Config)
	bool bNotifyNewSystems = true;

	// Shows common tips only for Gorgeous plugins enabled in this project.
	UPROPERTY(Config)
	bool bShowCommonPluginTips = true;

	// Lets sampled asset-editor visits send a coarse asset category to request a matching tip.
	UPROPERTY(Config)
	bool bContextAwareTips = false;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};