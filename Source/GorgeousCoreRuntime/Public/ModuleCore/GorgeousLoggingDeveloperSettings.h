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
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/DeveloperSettings.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousLoggingDeveloperSettings.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UUserWidget;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Logging Developer Settings
| Functional Name: UGorgeousLoggingDeveloperSettings
| Parent Class: UDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Logging Developer Settings.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Config = Game, DefaultConfig, DisplayName = "Core ↪ Logging",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ModuleCore/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ModuleCore/GorgeousLoggingDeveloperSettings",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ModuleCore/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousLoggingDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousLoggingDeveloperSettings();

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual FName GetSectionName() const override { return TEXT("Core|Logging"); }
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousLogging", "SectionName", "Core ↪ Logging"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousLogging", "SectionDescription", "Configure logging outputs, message log routing, and in-game log widgets."); }
#endif

	// Enables the Message Log listing for Gorgeous Things.
	UPROPERTY(EditAnywhere, Config, Category = "Message Log")
	bool bEnableGorgeousMessageLog;

	// Listing name used in the Message Log window.
	UPROPERTY(EditAnywhere, Config, Category = "Message Log")
	FName MessageLogListingName;

	// Minimum verbosity required for entries to be pushed into the Message Log listing.
	UPROPERTY(EditAnywhere, Config, Category = "Message Log")
	TEnumAsByte<EGorgeousLoggingImportance> MinMessageLogVerbosity;

	// Mirrors logs to the output log.
	UPROPERTY(EditAnywhere, Config, Category = "Output")
	bool bMirrorToOutputLog;

	// Displays logs on-screen via AddOnScreenDebugMessage.
	UPROPERTY(EditAnywhere, Config, Category = "Output")
	bool bShowOnScreen;

	/** Shows an in-game logging widget. */ //@TODO: I like the idea of the in game wlog widget class, but maybe this property should be for if logs should show in game at all
	UPROPERTY(EditAnywhere, Config, Category = "In-Game", meta = (EditCondition = "bShowOnScreen", EditConditionHides))
	bool bShowInGame;

	// Widget class used for the in-game logging panel.
	UPROPERTY(EditAnywhere, Config, Category = "In-Game", meta = (EditCondition = "bShowInGame", EditConditionHides))
	TSoftClassPtr<class UUserWidget> InGameLogWidgetClass;
};