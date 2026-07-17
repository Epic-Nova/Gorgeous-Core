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
#include "GorgeousStatFoundationStructures.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousStatFoundationSettings.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Stat Foundation Settings
| Functional Name: UGorgeousStatFoundationSettings
| Parent Class: UDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Developer Settings for the Gorgeous Stat System.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Config = GorgeousStats, DefaultConfig,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/GorgeousStatFoundationSettings",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Examples/"
		)
)

class GORGEOUSCORERUNTIME_API UGorgeousStatFoundationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	// Constructs the Stat Foundation developer settings.
public:
	UGorgeousStatFoundationSettings();

	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Global registry of all available stats in the project.
	UPROPERTY(Config, EditAnywhere, Category = "Stat Registry", meta = (ForceInlineRow))
	TMap<FGameplayTag, FGorgeousStat_S> StatRegistry;

	// If true, the system will automatically dispatch a Signal Bridge signal when a stat changes.
	UPROPERTY(Config, EditAnywhere, Category = "System Configuration")
	bool bAutoDispatchChangeSignals = true;

	// The root tag used for auto-dispatched change signals (e.g., 'Signal.StatChanged').
	UPROPERTY(Config, EditAnywhere, Category = "System Configuration", meta = (EditCondition = "bAutoDispatchChangeSignals"))
	FGameplayTag ChangeSignalRoot;

	// The signal tag dispatched when stats are restored from the network root.
	UPROPERTY(Config, EditAnywhere, Category = "System Configuration")
	FGameplayTag RestoredSignal;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:

	/**
	 * Finds the metadata registered for a statistic tag.
	 *
	 * @param Tag The statistic tag to find.
	 * @return The registered metadata, or nullptr when the tag is not registered.
	 */
	const FGorgeousStat_S* FindStatMetadata(const FGameplayTag& Tag) const;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Overrides ---============================>
	#pragma region Overrides
#if WITH_EDITOR
	public:

	// Returns the project settings category containing these settings.
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	// Returns the project settings section identifier.
	virtual FName GetSectionName() const override { return TEXT("Core|Stats"); }
	// Returns the display text for the project settings section.
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousStatSettings", "SectionName", "Core ↪ Stats"); }
	// Returns the description shown for the project settings section.
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousStatSettings", "SectionDescription", "Configure global stat registry, signal routing, and default values."); }
#endif
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
};