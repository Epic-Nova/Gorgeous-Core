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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
#include "Engine/DeveloperSettings.h"
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
#include "GorgeousLoggingDeveloperSettings.generated.h"
//<-------------------------------------------------------------------------->

class UUserWidget;

UCLASS(Config = Game, DefaultConfig, DisplayName = "Core ↪ Logging")
class GORGEOUSCORERUNTIME_API UGorgeousLoggingDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousLoggingDeveloperSettings();

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual FName GetSectionName() const override { return TEXT("00_Core|Logging"); }
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousLogging", "SectionName", "Core ↪ Logging"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousLogging", "SectionDescription", "Configure logging outputs, message log routing, and in-game log widgets."); }
#endif

	/** Enables the Message Log listing for Gorgeous Things. */
	UPROPERTY(EditAnywhere, Config, Category = "Message Log")
	bool bEnableGorgeousMessageLog;

	/** Listing name used in the Message Log window. */
	UPROPERTY(EditAnywhere, Config, Category = "Message Log")
	FName MessageLogListingName;

	/** Minimum verbosity required for entries to be pushed into the Message Log listing. */
	UPROPERTY(EditAnywhere, Config, Category = "Message Log")
	TEnumAsByte<EGorgeousLoggingImportance> MinMessageLogVerbosity;

	/** Mirrors logs to the output log. */
	UPROPERTY(EditAnywhere, Config, Category = "Output")
	bool bMirrorToOutputLog;

	/** Displays logs on-screen via AddOnScreenDebugMessage. */
	UPROPERTY(EditAnywhere, Config, Category = "Output")
	bool bShowOnScreen;

	/** Shows an in-game logging widget. */
	UPROPERTY(EditAnywhere, Config, Category = "In-Game", meta = (EditCondition = "bShowOnScreen", EditConditionHides))
	bool bShowInGame;

	/** Widget class used for the in-game logging panel. */
	UPROPERTY(EditAnywhere, Config, Category = "In-Game", meta = (EditCondition = "bShowInGame", EditConditionHides))
	TSoftClassPtr<class UUserWidget> InGameLogWidgetClass;

	/** Emits sample Message Log entries on editor startup for verification. */
	UPROPERTY(EditAnywhere, Config, Category = "Editor")
	bool bEmitSampleLogsOnStartup;
};
